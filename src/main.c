#include <kernel.h>
#include <quickmenureborn/qm_reborn.h>
#include <paf/stdc.h>
#include <appmgr.h>

#ifdef _DEBUG
#define print(...) sceClibPrintf(__VA_ARGS__)
#else
#define print(...) 
#endif

#define CONFIG_FILE_PATH "ux0:data/quicklauncher.txt"

#define ICON_WIDTH 80
#define ICON_HEIGHT 80

#define START_X_POS -(SCE_PLANE_WIDTH / 2) + (ICON_WIDTH / 2)
#define MAX_X_POS (SCE_PLANE_WIDTH / 2) - (ICON_WIDTH / 2)

#define MAX_APP_PER_ROW ((int)(SCE_PLANE_WIDTH / ICON_WIDTH))

int currStrtok = 0;

//sce_paf_strtok causes crash....
char *strtok(char splitter, char *str)
{
    int len = sce_paf_strlen(str);
    for (int i = currStrtok; i <= len; i++)
    {
        if(str[i] == splitter || str[i] == '\0')
        {
            char *s = (char *)sce_paf_malloc((i - currStrtok) + 1);
            sce_paf_memset(s, 0, (i - currStrtok) + 1 );

            sce_paf_memcpy(s, str + sizeof(char) * currStrtok, (i - currStrtok));
            currStrtok = i + 1; 
            return s;
        }
    }

    return NULL;
}

int strtokNum(char splitter, const char *str)
{
    int ret = 0; 
    for (int i = 0; i < sce_paf_strlen(str); i++)
    {
        if(str[i] == splitter)
            ret++;
    }

    return ret + 1;
}

void MakeWidgetWithProperties(const char *refID, const char *parentRefID, QMRWidgetType type, float posX, float posY, float sizeX, float sizeY, float colR, float colG, float colB, float colA, const char *label)
{
    QuickMenuRebornRegisterWidget(refID, parentRefID, type);
    QuickMenuRebornSetWidgetPosition(refID, posX, posY, 0, 0);
    QuickMenuRebornSetWidgetColor(refID, colR, colG, colB, colA);
    QuickMenuRebornSetWidgetSize(refID, sizeX, sizeY, 0, 0);
    if(label != NULL)
        QuickMenuRebornSetWidgetLabel(refID, label);
}

BUTTON_HANDLER(StartNormalApp)
{
    char titleid[10];
    
    sce_paf_memset(titleid, 0, sizeof(titleid));
    sce_paf_strncpy(titleid, &id[14], 9);

    char uri[32];
    sce_paf_snprintf(uri, 32, "psgm:play?titleid=%s", titleid);

    print("Launching normal app: %s\n", titleid);
    sceAppMgrLaunchAppByUri(0x20000, uri);

    QuickMenuRebornCloseMenu();
}

BUTTON_HANDLER(StartSystemApp)
{
    char titleid[10];
    
    sce_paf_memset(titleid, 0, sizeof(titleid));
    sce_paf_strncpy(titleid, &id[14], 9);

    print("launching sys app: %s\n", titleid);
    sceAppMgrLaunchAppByName2ForShell(titleid, NULL, NULL);

    QuickMenuRebornCloseMenu();
}

int module_start()
{
    print("QuickLauncher, By Ibrahim\n");

    FILE *fp = sce_paf_fopen(CONFIG_FILE_PATH, "r");
    
    if(fp == NULL)
    {
        print("Error opening config file at: %s!\n", CONFIG_FILE_PATH);
        return SCE_KERNEL_START_NO_RESIDENT;
    }

    sce_paf_fseek(fp, 0, SEEK_END); 
    long size = sce_paf_ftell(fp);
    sce_paf_fseek(fp, 0, SEEK_SET);

    char *config = (char *)sce_paf_malloc(size + 1);
    sce_paf_memset(config, 0, size + 1);

    if(config == NULL)  
    {
        print("Error malloc(size); Failed!\n");
        return SCE_KERNEL_START_RESIDENT;
    }

    sce_paf_fread(config, size, 1, fp);

    QuickMenuRebornSeparator("QuickLauncher_seperator", SCE_SEPARATOR_HEIGHT);

    currStrtok = 0;
    int numOfApps = strtokNum(';', config);
    char *currApp = NULL;
    float currXPos = START_X_POS + ((SCE_PLANE_WIDTH - (MAX_APP_PER_ROW * ICON_WIDTH)) / 2), currPlaneHeight = ICON_HEIGHT, currYPos = 0;
    print("%f\n", ((SCE_PLANE_WIDTH - (MAX_APP_PER_ROW * ICON_WIDTH)) / 2));
    if(numOfApps < MAX_APP_PER_ROW) currXPos += ((MAX_APP_PER_ROW - numOfApps) * ICON_WIDTH) / 2;

    MakeWidgetWithProperties("QuickLauncher_main_plane", NULL, plane, 0, 0, SCE_PLANE_WIDTH, currPlaneHeight, 1,1,1,0, NULL);

    do
    {
        currApp = strtok(';', config);

        if(currApp != NULL && sce_paf_strlen(currApp) != 0)
        {
            SceBool isSystemApp = sce_paf_strstr(currApp, "NPXS") != NULL; //Not if it runs in system mode, talking about vs0: apps here
            char refID[24];
            sce_paf_snprintf(refID, sizeof(refID), "QuickLauncher_%s", currApp);
            print("Adding: %s\n", currApp);

            MakeWidgetWithProperties(refID, "QuickLauncher_main_plane", button, currXPos, currYPos, 80, 80, 1,1,1,1, NULL);

            char texRefID[28];
            sce_paf_snprintf(refID, sizeof(refID), "QuickLauncher_%s_tex", currApp);

            char texPath[0x100];
            sce_paf_snprintf(texPath, 0x100, !isSystemApp ? "ux0:app/%s/sce_sys/icon0.png" : "vs0:app/%s/sce_sys/icon0.png" , currApp);

            QuickMenuRebornRegisterTexture(texRefID, texPath);
            QuickMenuRebornSetWidgetTexture(refID, texRefID);

            currXPos += ICON_WIDTH;
            if (currXPos > MAX_X_POS)
            {
                currXPos = START_X_POS + ((SCE_PLANE_WIDTH - (MAX_APP_PER_ROW * ICON_WIDTH)) / 2);
                currYPos += ICON_HEIGHT;
                currPlaneHeight += ICON_HEIGHT;
                QuickMenuRebornSetWidgetSize("QuickLauncher_main_plane", SCE_PLANE_WIDTH, currPlaneHeight, 0, 0);
            }
            print("System App: %s\n", isSystemApp ? "True" : "False");
            QuickMenuRebornRegisterEventHanlder(refID, QMR_BUTTON_RELEASE_ID, isSystemApp ? StartSystemApp : StartNormalApp, NULL);

        }

        sce_paf_free(currApp);
    } while(currApp != NULL);

    sce_paf_free(config);

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop()
{
    QuickMenuRebornRemoveSeparator("QuickLauncher_seperator");
    QuickMenuRebornUnregisterWidget("QuickLauncher_main_plane");

    FILE *fp = sce_paf_fopen(CONFIG_FILE_PATH, "r");
    
    if(fp == NULL)
    {
        print("Error opening config file at: %s!\n", CONFIG_FILE_PATH);
        return SCE_KERNEL_START_NO_RESIDENT;
    }

    sce_paf_fseek(fp, 0, SEEK_END); 
    long size = sce_paf_ftell(fp);
    sce_paf_fseek(fp, 0, SEEK_SET);

    char *config = sce_paf_malloc(size + 1);
    sce_paf_memset(config, 0, size + 1);

    if(config == NULL)  
    {
        print("Error malloc(size); Failed!\n");
        return SCE_KERNEL_START_RESIDENT;
    }

    sce_paf_fread(config, size, 1, fp);

    currStrtok = 0;
    char *currApp = NULL;
    do
    {
        currApp = strtok(config, ';');

        if(currApp != NULL && sce_paf_strlen(currApp) != 0)
        {
            char refID[24];
            sce_paf_snprintf(refID, sizeof(refID), "QuickLauncher_%s", currApp);
            print("Removing: %s\n", currApp);

            char texRefID[28];
            sce_paf_snprintf(refID, sizeof(refID), "QuickLauncher_%s_tex", currApp);

            QuickMenuRebornUnregisterTexture(texRefID);
            QuickMenuRebornUnregisterWidget(refID);
        }

        sce_paf_free(currApp);
    } while(currApp != NULL);

    sce_paf_free(config);

    return SCE_KERNEL_STOP_SUCCESS;
}