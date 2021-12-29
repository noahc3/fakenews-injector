#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "fakenews_bin.h"
#include "realnews_bin.h"

#define SECONDS(x) (x * 1000000000)

int main(int argc, char **argv) {
    Result rc = 0;

    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    puts("Fake News Injector\n\n");

    rc = newsInitialize(NewsServiceType_Administrator);

    if (R_FAILED(rc)) {
        puts("Failed to initialize news!\nExiting in 5 seconds\n");
        svcSleepThread(SECONDS(5));
        consoleExit(NULL);
        return 0;
    }

    puts("Press + to install Fake News, press - to restore real News, B to exit\n");

    while (appletMainLoop()) {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) {
            rc = newsPostLocalNews(fakenews_bin, fakenews_bin_size);
            if (R_FAILED(rc)) {
                printf("    Failed to install fake news: 0x%08x\n", rc);
            } else {
                printf("    Successfully installed fake news!\n");
            }
        } else if (kDown & HidNpadButton_Minus) {
            rc = newsPostLocalNews(realnews_bin, realnews_bin_size);
            if (R_FAILED(rc)) {
                printf("    Failed to restore real news: 0x%08x\n", rc);
            } else {
                printf("    Successfully restored real news!\n");
            }
        } else if (kDown & HidNpadButton_B) {
            break;
        }

        consoleUpdate(NULL);
    }

    newsExit();

    consoleExit(NULL);
    return 0;
}
