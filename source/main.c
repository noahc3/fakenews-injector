#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

#include <switch.h>

Result copyFile(const char *srcPath, const char *dstPath)
{
    FILE *src;
    FILE *dst;

    if (!(src = fopen(srcPath, "rb")))
    {
        printf("    Failed to open file %s\n", srcPath);
        return 1;
    }

    if (!(dst = fopen(dstPath, "wb")))
    {
        printf("    Failed to open file %s\n", dstPath);
        fclose(src);
        return 1;
    }

    size_t size;
    u8 *buf = malloc(sizeof(u8) * 0x50000);
    u64 offset = 0;
    
    while ((size = fread(buf, 1, 0x50000, src)) > 0)
    {
        fwrite(buf, 1, size, dst);
        offset += size;
    }

    free(buf);
    fclose(src);
    fclose(dst);

    // Check if the dest path starts with save:/
    if (strncmp(dstPath, "save:/", strlen("save:/")) == 0)
    {
        fsdevCommitDevice("save");
    }

    return 0;
}

int main(int argc, char **argv)
{
    Result rc=0;

    FsFileSystem tmpfs;
    u64 processID=0x010000000000000C; //process which has the save open (BCAT)
    u64 saveID=0x8000000000000090; //system save id (news)
    const char *srcFile = "romfs:/D00000000000000000000_LS00000000000000010000.msgpack";
    const char *destFile = "save:/data/D00000000000000000000_LS00000000000000010000.msgpack";

    consoleInit(NULL);

    printf("Fake News Injector\n\n\n");

    printf("Trying for save ID=0x%016lx\n\n", saveID);
    
    printf("Initializing pm:shell\n");
    rc = pmshellInitialize();
    if (R_FAILED(rc)) {
	printf("    Failed to initialize pm:shell: 0x%x\n", rc);
    } else {
	printf("    pm:shell initialized\n");
    }


    printf("\nKilling BCAT\n");

    for (int attempt = 0; attempt < 100; attempt++)
    {
        pmshellTerminateProcessByTitleId(processID);

        if (R_SUCCEEDED(rc = fsMount_SystemSaveData(&tmpfs, saveID)))
        {
            printf("    BCAT is dead\n");
            break;
        }
    }

    // Took too many attempts
    if (R_FAILED(rc))
    {
        printf("    Failed to mount system save data %016lx for tid %016lx: 0x%08x", saveID, processID, rc);
        return rc;
    }

    printf("\nMounting save:/\n");
    if (fsdevMountDevice("save", tmpfs) == -1) 
    {
        printf("    Failed to mount system save data device.\n");
        return -1;
    }

    if (R_SUCCEEDED(rc)) {
        printf("    save:/ mounted\n");
        printf("\nInitializing ROMFS\n");
        rc = romfsInit();
        if (R_FAILED(rc)) {
            printf("    Failed to initialize ROMFS: 0x%08x", rc);
        }

        if (R_SUCCEEDED(rc)) {
            printf("    ROMFS initialized\n");
            printf("\nCopying fakenews save data\n");

            rc = copyFile(srcFile, destFile);
            
            if (R_FAILED(rc)) {
                printf("    Failed to copy file: 0x%08x", rc);
            }

            if (R_SUCCEEDED(rc)) {
                printf("    Done!\n");

                printf("\nClosing save file\n");

                fsdevUnmountDevice("save");

                printf("    Done!\n");

                printf("\n\nDone!\n\n");
            }
        }
    }

    printf("Press + to reboot, press - to shutdown.");

    // Main loop
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) {
            bpcInitialize();
            bpcRebootSystem();
        } else if (kDown & KEY_MINUS) {
            bpcInitialize();
            bpcShutdownSystem();
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
