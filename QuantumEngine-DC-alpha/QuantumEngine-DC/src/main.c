/*
 * src/main.c — entry point KOS/Dreamcast do Quantum Engine
 *
 * Este arquivo NAO existe no Java original (nao tem equivalente em
 * code/utils/Main.java, que foi portado em src/utils/main.c como
 * Main_startApp()/Main_init()/etc). Aqui fica so o bootstrap especifico
 * de plataforma: montar o romdisk, inicializar video/controle do KOS
 * e chamar a logica portada.
 *
 * IMPORTANTE: a camada de ligacao Graphics/Image <-> PVR (rendering
 * real na tela) e GameKeyboard <-> maple (controle) ainda nao foi
 * implementada nesta fase da conversao — os arquivos portados ate agora
 * cobrem a LOGICA do engine (fiel ao J2ME), nao o backend grafico do
 * Dreamcast. Por isso este loop soh inicializa o video e chama
 * Main_startApp(); o link pode falhar por falta de implementacao de
 * funcoes extern (Image_*, Graphics_*, etc.) que ainda serao criadas.
 */
#include <kos.h>
#include "utils/main.h"
#include "utils/qfps.h"

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    vid_set_mode(DM_640_480, PM_RGB565);

    Main_startApp();

    int quit = 0;
    while (!quit) {
        MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER, cont_state_t, st)
            if (st->buttons & CONT_START) quit = 1;
        MAPLE_FOREACH_END()

        QFPS_frame();

        vid_waitvbl();
    }

    Main_destroyApp(1);
    return 0;
}
