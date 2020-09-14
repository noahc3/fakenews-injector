#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>

#include "fakenews_bin.h"
#include "realnews_bin.h"

int main(int argc, char **argv) {
    Result rc = 0;

    consoleInit(NULL);

    puts("Fake News Injector\n\n");

    rc = newsInitialize(NewsServiceType_Administrator);

    puts("Press + to install Fake News, press - to restore real News, B to exit\n");

    while (appletMainLoop()) {
        hidScanInput();

        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            rc = newsPostLocalNews(fakenews_bin, fakenews_bin_size);
            if (R_FAILED(rc)) {
                printf("    Failed to install fake news: 0x%08x\n", rc);
            } else {
                printf("    Successfully installed fake news!\n");
            }
        } else if (kDown & KEY_MINUS) {
            rc = newsPostLocalNews(realnews_bin, realnews_bin_size);
            if (R_FAILED(rc)) {
                printf("    Failed to restore real news: 0x%08x\n", rc);
            } else {
                printf("    Successfully restored real news!\n");
            }
        } else if (kDown & KEY_B) {
            break;
        }

        consoleUpdate(NULL);
    }

    newsExit();

    consoleExit(NULL);
    return 0;
}
