
#include <mujoco/mujoco.h>  
#include <iostream>        

// argc = quanti argomenti sono stati passati al programma
// argv = gli argomenti come testo. argv[0] è il nome del programma,
//        argv[1] è il primo argomento vero (per noi: il percorso di scene.xml)
int main(int argc, char** argv) {

    // --- 1. Controllo di avere il percorso del modello ---------------------
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " percorso/del/modello/scene.xml\n";
        return 1;  // un valore diverso da 0 significa "il programma è fallito"
    }

    // --- 2. Caricamento del modello ----------------------------------------
    char error[1000];
    mjModel* m = mj_loadXML(argv[1], nullptr, error, sizeof(error));

    // Se il caricamento fallisce
    if (!m) {
        std::cerr << "Errore nel caricamento: " << error << "\n";
        return 1;
    }

    // --- 3. Creazione dello stato della simulazione ------------------------
    mjData* d = mj_makeData(m);

    // --- 4. Elenco degli attuatori del modello -----------------------------
    // m->nu è il numero di attuatori. Per ognuno stampiamo:
    //   - l'indice (0, 1, 2...)
    //   - il nome (così sai come si chiama)
    //   - il range: i valori minimo e massimo che si possono dare come comando
    // actuator_ctrlrange è un array piatto: per l'attuatore i, il minimo sta
    // in [2*i] e il massimo in [2*i+1].
    for (int i = 0; i < m->nu; i++) {
        std::cout << i << ": " << mj_id2name(m, mjOBJ_ACTUATOR, i)
                  << "  range: " << m->actuator_ctrlrange[2 * i] << " .. "
                  << m->actuator_ctrlrange[2 * i + 1] << "\n";
    }

    // --- 5. Cerco l'attuatore del gripper per nome -------------------------
    const char* actuator_name = mj_id2name(m, mjOBJ_ACTUATOR, 0);
    int id = mj_name2id(m, mjOBJ_ACTUATOR, actuator_name);

    // Se il nome non esiste, mj_name2id restituisce -1: controlliamo sempre.
    if (id < 0) {
        std::cerr << "Attuatore non trovato: controlla il nome nell'elenco sopra\n";
        mj_deleteData(d);
        mj_deleteModel(m);
        return 1;
    }

    // --- 6. Ciclo di simulazione -------------------------------------------
    int t_sim = 8; // [s]
    int steps = t_sim / m->opt.timestep;        // 4000 passi = 8 secondi simulati.
    
    for (int step = 0; step < steps; step++) {

        // d->ctrl è un array con un valore per ogni attuatore; il nostro sta nella posizione "id".
        // (step < 2000) ? 0.0 : 255.0 significa:
        //    nei primi 2000 passi -> 0   (gripper aperto)
        //    dopo                 -> 255 (gripper chiuso)
        d->ctrl[id] = (step < steps/2) ? 0.0 : 255.0;

        // Fa avanzare la fisica di un passo
        mj_step(m, d);

        // Ogni 500 passi stampiamo:
        //   d->time = tempo simulato in secondi
        //   d->ncon = numero di contatti attivi in questo momento
        if (step % 500 == 0) {
            std::cout << "t=" << d->time << "  ncon=" << d->ncon << "\n";
        }
    }

    // --- 7. Pulizia ---------------------------------------------------------
    mj_deleteData(d);
    mj_deleteModel(m);

    return 0;  // 0 = programma terminato correttamente
}