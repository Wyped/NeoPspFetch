#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <psputils.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "libs/libpspexploit.h"
#include "kernel.h"


PSP_MODULE_INFO("NeoPspFetch", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4*1024);

#define printf pspDebugScreenPrintf

#define color(x) pspDebugScreenSetTextColor(x)
#define RED 0xff0000ff
#define BLUE 0xffff0000
#define GREEN 0xff00ff00
#define LGRAY 0xffd3d3d3
#define WHITE 0xffffffff
#define YELLOW 0xff00ffff
#define ORANGE 0xff007fff

#define SCREEN_W 68
#define SCREEN_H 33

#define ASCII_HEIGHT_MAX 30
#define ASCII_WIDTH_MAX 44
#define GAP 0
#define INFO_WIDTH 44
#define INFO_INDENT 2
#define BLOCK_LEFT_MARGIN 2
#define INFO_LINES_MAX 17

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272


// --- ASCII art sets ---
const char* ascii_art_psp1000[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++  1000  PHAT  ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_psp2000[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++  2000  SLIM  ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_psp3000[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++ 3000  BRIGHT ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_pspN1000[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++   N1000 GO   ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_pspE1000[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++ E1000 STREET ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_pspDefault[] = {
    "",
    "",
    "",
    "",
    "       ++++++++",
    "    ++++      ++++",
    "  ++     SONY     ++",
    " ++                ++",
    "+######   #### ######+",
    "+######   #    ######+",
    "+#     ####    #    ++",
    " ++                ++",
    "  ++   UNKNOWN!   ++",
    "    ++++      ++++",
    "       ++++++++",
    "",
    "",
    "",
    "",
    NULL
};

const char* ascii_art_battery[] = {  
        "                 =######",       
        "     ++++***############",       
        "    +*****##############",        
        "    +****###############",       
        "    +****###############",       
        "    +****###############",        
        "    +****###############",        
        "    +****### BATTERY ###",        
        "    +****###############",        
        "    +****###  +   -  ###",        
        "    +****###############",        
        "    +****###############",         
        "    +****###############",        
        "    +***################",        
        "    +***################",         
        "    +****###############",        
        "    +****###############",         
        "       ##           ### ",                                               
    NULL
};
const char* ascii_art_umd[] = {
    "   .--------.",
    "  / .------. \\",
    " / /        \\ \\",
    "| |   UMD    | |",
    " \\ \\        / /",
    "  \\ '------' /",
    "   '--------'",
    NULL
};
const char* ascii_art_credits[] = {
    "#################################",
    "#########+++++++++###############",
    "######++++++++++++++#############",
    "#####+++#+#++++++++++############",
    "#####++#+++++#++##++++###########",
    "####++++#++++###-.#+++#####+.####",
    "#####+++#####-----##++#####...###",
    "####++####..---####++#######...##",
    "#####+++##.----..##########...###",
    "######+#+#------#+#+#######..####",
    "#######+#+###-#++++##############",
    "########-#----#+#+--####-#+.#####",
    "##--#---------#--#----#-#########",
    "##--------------+-#----##########",
    "##-##..--------#.-#---###########",
    "####......-##......--+###########",
    "###-...............--############",
    "####..............#---###########",
    "######..-#...-...##---+##########",
    "######............-----##########",
    "#######..........##---###########",
    "#####............################",
    NULL
};



// --- Exit callback ---
int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}
int callback_thread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}
int setup_callbacks(void) {
    int thid = sceKernelCreateThread("update_thread", callback_thread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0)
        sceKernelStartThread(thid, 0, 0);
    return thid;
}



// --- Draw screen and formating functions ---
// --- Border ---
void drawBorder() {
    color(ORANGE);
    for (int x = 0; x < SCREEN_W; x++) {
        pspDebugScreenSetXY(x, 0); printf("*");
    }
    for (int y = 1; y < SCREEN_H-1; y++) {
        pspDebugScreenSetXY(0, y); printf("*");
        pspDebugScreenSetXY(SCREEN_W-1, y); printf("*");
    }
    for (int x = 0; x < SCREEN_W; x++) {
        pspDebugScreenSetXY(x, SCREEN_H-1); printf("*");
    }
    color(WHITE);
    pspDebugScreenSetXY(1,1);
}

// --- Print safely with width limit ---
void printSafeAt(int x, int y, const char* s, int max_width) {
    char buf[128];
    strncpy(buf, s, max_width);
    buf[max_width] = '\0';
    pspDebugScreenSetXY(x, y);
    printf("%s", buf);
}


// --- Battery Info Helper Functions ---
void printBatteryLifeTimeAt(int x, int y) {
    int ChargerPresence = scePowerIsPowerOnline();
    int BatLifeTime = scePowerGetBatteryLifeTime();
    pspDebugScreenSetXY(x, y);

    if (BatLifeTime < 0) {
        color(RED);
        printf("Battery Life left: ERROR");
    } else if (ChargerPresence == 1 && BatLifeTime == 0) {
        color(YELLOW);
        printf("Battery Life left: -");
    } else {
        int Hours = BatLifeTime / 60;
        int Minutes = BatLifeTime % 60;
        color(GREEN);
        printf("Battery Life left: %d Hours %d Minutes", Hours, Minutes);
    }
    color(WHITE);
}

void printBatteryStatusAt(const char* label, int status, int x, int y) {
    pspDebugScreenSetXY(x, y);
    color(WHITE);
    if (status < 0) {
        printf("%s: ERROR", label);
    } else if (status == 0) {
        printf("%s: In Use", label);
    } else if (status == 1) {
        printf("%s: Charging", label);
    } else {
        printf("%s: Unknown", label);
    }
}

void printIntAt(const char* label, int value, const char* unit, int x, int y, u32 clr) {
    pspDebugScreenSetXY(x, y);
    color(clr);
    if (value < 0) {
        printf("%s: ERROR", label);
    } else if (unit && unit[0]) {
        printf("%s: %d %s", label, value, unit);
    } else {
        printf("%s: %d", label, value);
    }
    color(WHITE);
}

// --- Draw ascii and info block with colored info ---
void drawAsciiAndInfoBlock_colored(
    const char** ascii_art,
    int ascii_width,
    int ascii_height,
    void (*fill_info_colored)(int x, int y),
    int info_lines
) {
    int block_width = ascii_width + GAP + INFO_WIDTH;
    int block_height = (ascii_height > info_lines) ? ascii_height : info_lines;

    int start_x = ((SCREEN_W - block_width) / 2) - BLOCK_LEFT_MARGIN;
    if (start_x < 1) start_x = 1;
    int start_y = (SCREEN_H - block_height) / 2;
    if (start_y < 1) start_y = 1;

    // Draw ASCII art
    for (int i = 0; i < ascii_height && ascii_art[i]; ++i) {
        printSafeAt(start_x, start_y + i, ascii_art[i], ascii_width);
    }
    // Draw colored info lines
    int info_x = start_x + ascii_width + GAP + INFO_INDENT;
    int info_y = start_y;
    fill_info_colored(info_x, info_y);
}

// --- Centered bottom help text ---
void printCenteredBottom(const char* text, u32 textColor) {
    int len = strlen(text);
    if (len > SCREEN_W - 2) len = SCREEN_W - 2;
    int x = (SCREEN_W - len)/2;
    int y = SCREEN_H-2;
    color(textColor);
    printSafeAt(x > 1 ? x : 1, y, text, SCREEN_W-2);
    color(WHITE);
}

// --- End of drawing and formating functions ---

// --- Take BMP Screenshot ---
void take_screenshot() {
    // Get VRAM pointer (RGBA8888)
    void* vram = NULL;
    int mode, width, height, bufferwidth, pixelformat;
    sceDisplayGetMode(&mode, &width, &height);
    sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, 1);

    // Validate format
    if (pixelformat != PSP_DISPLAY_PIXEL_FORMAT_8888) {
        printf("Unsupported pixel format for screenshot\n");
        return;
    }


    // Get local PSP time
    /* ScePspDateTime dt;
    sceRtcGetCurrentClockLocalTime(&dt);

    char filename[128];
    snprintf(filename, sizeof(filename),
        "ms0:/PICTURE/NeoPspFetch_%04d%02d%02d_%02d%02d%02d.bmp",
        dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second); */


    // Use standard C library time (may differ from XMB time)
    // This is a workaround as using psp rtc is currently messing with how i implemented libpspexploit
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Create NeoPspFetch folder if it doesn't exist
    int dirCheck = sceIoDopen("ms0:/PICTURE/NeoPspFetch");
    if (dirCheck < 0) {
        int mkres = sceIoMkdir("ms0:/PICTURE/NeoPspFetch", 0777);
        if (mkres < 0) {
            printf("Could not create NeoPspFetch folder!\n");
            return;
        }
    } else {
        sceIoDclose(dirCheck);
    }

    char filename[128];
    snprintf(filename, sizeof(filename),
        "ms0:/PICTURE/NeoPspFetch/NeoPspFetch_%02d%02d%02d.bmp",
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("Screenshot save error!\n");
        return;
    }

    // BMP header
    unsigned char header[54] = {
        0x42, 0x4D,
        0,0,0,0,
        0,0, 0,0,
        54,0,0,0,
        40,0,0,0,
        0,0,0,0,
        0,0,0,0,
        1,0,
        32,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0,
        0,0,0,0
    };

    int filesize = 54 + width * height * 4;
    header[2] = (filesize      ) & 0xFF;
    header[3] = (filesize >> 8 ) & 0xFF;
    header[4] = (filesize >> 16) & 0xFF;
    header[5] = (filesize >> 24) & 0xFF;

    header[18] = (width      ) & 0xFF;
    header[19] = (width >> 8 ) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;

    header[22] = (height      ) & 0xFF;
    header[23] = (height >> 8 ) & 0xFF;
    header[24] = (height >> 16) & 0xFF;
    header[25] = (height >> 24) & 0xFF;

    fwrite(header, 1, 54, fp);

    // BMP stores bottom line first
    u32* pixels = (u32*)vram;
    for (int y = height-1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            u32 pixel = pixels[y*bufferwidth + x];
            unsigned char b = (pixel >> 16) & 0xFF;
            unsigned char g = (pixel >> 8) & 0xFF;
            unsigned char r = (pixel) & 0xFF;
            unsigned char a = (pixel >> 24) & 0xFF;
            fputc(b, fp);
            fputc(g, fp);
            fputc(r, fp);
            fputc(a, fp);
        }
    }
    fclose(fp);

    printf("\nScreenshot saved: %s\n", filename);
}

// --- Kernel Exploit - libPspExploit ---
void doKernelExploit () {
    printf("Initializing Kernel Exploit...\n");
    int ret = pspXploitInitKernelExploit();
    if (ret == 0) {
        printf("Corrupting Kernel...\n");
        ret = pspXploitDoKernelExploit();
        if (ret != 0) {
            printf("ERROR: Kernel Exploit failed\n");
            printf(" The program will automatically quit after 8 seconds...\n");
			color(WHITE);
			sceKernelDelayThread(8*1000*1000);
        }
    } else {
        printf("ERROR: Could not initialize Kernel Exploit\n");
        printf(" The program will automatically quit after 8 seconds...\n");
		color(WHITE);
		sceKernelDelayThread(8*1000*1000);
    }
}

// --- Detect if PSP GO - Returns 1 if ef0: is present, 0 otherwise ---
int is_psp_go(void) {
    SceUID fd = sceIoDopen("ef0:/");
    if (fd >= 0) {
        sceIoDclose(fd);
        return 1;
    }
    return 0;
}

// --- Print version.txt ---
void printVersionTxt(int x, int y_start) {
    SceUID fd;
    char buffer[256];
    int bytesRead;

    fd = sceIoOpen("flash0:/vsh/etc/version.txt", PSP_O_RDONLY, 0);
    if (fd < 0) {
        color(RED); 
        pspDebugScreenSetXY(x, y_start);
        printf("Error opening version.txt");
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    bytesRead = sceIoRead(fd, buffer, sizeof(buffer) - 1);
    sceIoClose(fd);

    if (bytesRead < 0) {
        color(RED); 
        pspDebugScreenSetXY(x, y_start);
        printf("Error reading version.txt");
        return;
    }
    buffer[bytesRead] = '\0';

    // Split into 5 lines and print
    int line = 0;
    char *saveptr, *curr = strtok_r(buffer, "\r\n", &saveptr);
    while (curr && line < 5) {
        color(LGRAY);
        pspDebugScreenSetXY(x, y_start + line);
        printSafeAt(x, y_start + line, curr, INFO_WIDTH - 2);
        curr = strtok_r(NULL, "\r\n", &saveptr);
        line++;
    }
}

// --- Parse firmware hex string to dec ---
float parse_hex_to_version(unsigned int hex) {
    unsigned char major = (hex >> 24) & 0xFF;       // First byte
    unsigned char minor = (hex >> 16) & 0xFF;       // Second byte
    unsigned char patch = (hex >> 8) & 0xFF;        // Third byte

    float version = major + ((minor * 10 + patch) / 100.0f);
    return version;
}

// --- Get Firmware Version ---
void printFirmwareVersion(char* buf, size_t buflen) {
    u32 fw = sceKernelDevkitVersion();
    float readablefw = parse_hex_to_version(fw);
    snprintf(buf, buflen, "%.2f", readablefw);
}


// --- Pages ---

// --- Info blocks with color output ---
void fill_info_main_colored(int x, int y) {
    color(YELLOW); pspDebugScreenSetXY(x, y+0); printf("Model: ");
    color(WHITE);  pspDebugScreenSetXY(x+7, y+0); printf("%s", ident_get_model());

    color(GREEN); pspDebugScreenSetXY(x, y+2); printf("Current FW: ");
    color(WHITE);  pspDebugScreenSetXY(x+12, y+2); 
    char fwStr[32];
    printFirmwareVersion(fwStr, sizeof(fwStr));
    printf("%s", fwStr);

    color(RED); pspDebugScreenSetXY(x, y+3); printf("Shipped FW: ");
    color(WHITE);  pspDebugScreenSetXY(x+12, y+3); printf("%s", ident_get_shippedfw());
    
    printVersionTxt(x, y+4);

    color(ORANGE); pspDebugScreenSetXY(x, y+10); printf("Baryon: ");
    color(WHITE);  pspDebugScreenSetXY(x+8, y+10); printf("0x%08x", ident_get_baryon());

    color(ORANGE); pspDebugScreenSetXY(x, y+11); printf("Tachyon: ");
    color(WHITE);  pspDebugScreenSetXY(x+9, y+11); printf("0x%08x", ident_get_tachyon());


    color(BLUE); pspDebugScreenSetXY(x, y+13); printf("Bluetooth MAC: ");
    color(WHITE);  pspDebugScreenSetXY(x+15, y+13);
    const unsigned char* m = ident_get_idsbtmac();
    int isGo = is_psp_go();
    if (isGo != 1) {
        printf("Unsupported");
    } else {
        if (m == NULL || 
            (m[0] == 0 && m[1] == 0 && m[2] == 0 && m[3] == 0 && m[4] == 0 && m[5] == 0)) {
            printf("unsupported");
        } else {
            printf("%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);
        }
    }

    color(BLUE); pspDebugScreenSetXY(x, y+14); printf("WiFi Region: ");
    color(WHITE);  pspDebugScreenSetXY(x+13, y+14); printf("%s", ident_get_wifi_region());


    color(YELLOW); pspDebugScreenSetXY(x, y+16); printf("NeoPspFetch - ");
    color(WHITE);  pspDebugScreenSetXY(x+14, y+16); printf("by Wyped");

    color(WHITE); // reset
}

void fill_info_battery_colored(int x, int y) {
    int isGo = is_psp_go();
    int BatPresence = scePowerIsBatteryExist();

    color(BLUE); pspDebugScreenSetXY(x, y+0); printf("Battery Info:");

    color(YELLOW); pspDebugScreenSetXY(x, y+2); printf("Charger detected: ");
    color(WHITE);  pspDebugScreenSetXY(x+18, y+2); printf("%s", scePowerIsPowerOnline() ? "Yes" : "No");

    if (BatPresence == 0) {
        const char* noBat = "NO BATTERY DETECTED";
        color(YELLOW); pspDebugScreenSetXY(x, y+4);  printf("Battery Present: ");
        color(RED);    pspDebugScreenSetXY(x+17, y+4);  printf("%s", noBat);
        color(YELLOW); pspDebugScreenSetXY(x, y+5);  printf("Battery Charging: ");
        color(RED);    pspDebugScreenSetXY(x+18, y+5);  printf("%s", noBat);
        color(YELLOW); pspDebugScreenSetXY(x, y+6);  printf("Battery Status: ");
        color(RED);    pspDebugScreenSetXY(x+16, y+6);  printf("%s", noBat);
        
        color(YELLOW); pspDebugScreenSetXY(x, y+8);  printf("Battery Life: ");
        color(RED);    pspDebugScreenSetXY(x+14, y+8);  printf("%s", noBat);
        color(YELLOW); pspDebugScreenSetXY(x, y+9);  printf("Battery Life left: ");
        color(RED);    pspDebugScreenSetXY(x+19, y+9);  printf("%s", noBat);
        
        color(YELLOW); pspDebugScreenSetXY(x, y+11);  printf("Battery Remaining Capacity: ");
        color(RED);    pspDebugScreenSetXY(x+28, y+11);  printf("%s", noBat);
        color(YELLOW); pspDebugScreenSetXY(x, y+12); printf("Battery Total Capacity: ");
        color(RED);    pspDebugScreenSetXY(x+24, y+12); printf("%s", noBat);
        
        color(YELLOW); pspDebugScreenSetXY(x, y+14); printf("Battery Temperature: ");
        color(RED);    pspDebugScreenSetXY(x+21, y+14); printf("%s", noBat);
        color(YELLOW); pspDebugScreenSetXY(x, y+15); printf("Battery Voltage: ");
        color(RED);    pspDebugScreenSetXY(x+17, y+15); printf("%s", noBat);
        color(WHITE);
        return;
    }

    color(YELLOW); pspDebugScreenSetXY(x, y+4);  printf("Battery Present: ");
    color(WHITE);  pspDebugScreenSetXY(x+17, y+4); printf("Yes");

    color(YELLOW); pspDebugScreenSetXY(x, y+5);  printf("Battery Charging: ");
    color(WHITE);  pspDebugScreenSetXY(x+18, y+5); printf("%s", scePowerIsBatteryCharging() ? "Yes" : "No");

    color(YELLOW); pspDebugScreenSetXY(x, y+6);  printf("Battery Status: ");
    int batStatus = scePowerGetBatteryChargingStatus();
    color(WHITE);  pspDebugScreenSetXY(x+16, y+6);
    if (batStatus < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
    } else if (batStatus == 0) {
        color(ORANGE);
        printf("In Use");
        color(WHITE);
    } else if (batStatus == 1) {
        color(GREEN);
        printf("Charging");
        color(WHITE);
    } else {
        color(RED);
        printf("Unknown");
        color(WHITE);
    }

    color(WHITE);  pspDebugScreenSetXY(x, y+7); printf("");

    color(YELLOW); pspDebugScreenSetXY(x, y+8);  printf("Battery Life: ");
    int batLife = scePowerGetBatteryLifePercent();
    color(WHITE);  pspDebugScreenSetXY(x+14, y+8);
    if (batLife < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
    } else {
        printf("%d %%", batLife);
    }

    color(YELLOW); pspDebugScreenSetXY(x, y+9);  printf("Battery Life left: ");
    int charger = scePowerIsPowerOnline();
    int batLifetime = scePowerGetBatteryLifeTime();
    color(WHITE);  pspDebugScreenSetXY(x+19, y+9);
    if (batLifetime < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
    } else if (charger == 1 && batLifetime == 0) {
        printf("-");
    } else {
        int hours = batLifetime / 60;
        int mins  = batLifetime % 60;
        printf("%d Hours %d Minutes", hours, mins);
    }


    if (isGo == 1) {
        color(YELLOW); pspDebugScreenSetXY(x, y+11);  printf("Battery Remaining Capacity: ");
        color(RED);    pspDebugScreenSetXY(x+28, y+11);  printf("UNSUPPORTED");
        color(YELLOW); pspDebugScreenSetXY(x, y+12); printf("Battery Total Capacity: ");
        color(RED);    pspDebugScreenSetXY(x+24, y+12); printf("UNSUPPORTED");
        color(YELLOW); pspDebugScreenSetXY(x, y+14); printf("Battery Temperature: ");
        color(RED);    pspDebugScreenSetXY(x+21, y+14); printf("UNSUPPORTED");
    } else {
        color(YELLOW); pspDebugScreenSetXY(x, y+11);  printf("Battery Remaining Capacity: ");
        int rem = scePowerGetBatteryRemainCapacity();
        color(WHITE);  pspDebugScreenSetXY(x+28, y+11);
        if (rem < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
        } else {
            printf("%d mAh", rem);
        }

        color(YELLOW); pspDebugScreenSetXY(x, y+12); printf("Battery Total Capacity: ");
        int full = scePowerGetBatteryFullCapacity();
        color(WHITE);  pspDebugScreenSetXY(x+24, y+12);
        if (full < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
        } else {
            printf("%d mAh", full);
        }

        color(WHITE);  pspDebugScreenSetXY(x, y+13); printf("");
        
        color(YELLOW); pspDebugScreenSetXY(x, y+14); printf("Battery Temperature: ");
        int temp = scePowerGetBatteryTemp();
        color(WHITE);  pspDebugScreenSetXY(x+21, y+14);
        if (temp < 0) {
            color(RED);
            printf("ERROR");
            color(WHITE);
        } else {
            printf("%d Celsius", temp);
        }
    }

    color(YELLOW); pspDebugScreenSetXY(x, y+15); printf("Battery Voltage: ");
    int volt = scePowerGetBatteryVolt();
    color(WHITE);  pspDebugScreenSetXY(x+17, y+15);
    if (volt < 0) {
        color(RED);
        printf("ERROR");
        color(WHITE);
    } else {
        printf("%d mV", volt);
    }
    color(WHITE);
}

void fill_info_credits_colored(int x, int y) {
    color(YELLOW);  pspDebugScreenSetXY(x, y+0); printf("NeoPspFetch v1.1");
        
    color(WHITE); pspDebugScreenSetXY(x, y+2); printf("Programmed by ");
    color(GREEN); pspDebugScreenSetXY(x, y+3); printf("Wyped");

    color(YELLOW); pspDebugScreenSetXY(x, y+6); printf("Credits: ");

    color(WHITE); pspDebugScreenSetXY(x, y+8); printf("Based on pspIdent by");
    color(BLUE); pspDebugScreenSetXY(x, y+9); printf("Yoti and friends");

    color(WHITE); pspDebugScreenSetXY(x, y+11); printf("libPspExploit by");
    color(RED); pspDebugScreenSetXY(x, y+12); printf("qwikrazor87, CelesteBlue,");
    color(RED); pspDebugScreenSetXY(x, y+13); printf("Acid_Snake, Davee, mcidclan,");
    color(RED); pspDebugScreenSetXY(x, y+14); printf("wally4000 and more...");

    color(WHITE); pspDebugScreenSetXY(x, y+16); printf("PSPDEV SDK by ");
    color(ORANGE); pspDebugScreenSetXY(x, y+17); printf("PSPDEV Team");

    color(LGRAY); pspDebugScreenSetXY(x, y+19); printf("Special thanks to");
    color(LGRAY); pspDebugScreenSetXY(x, y+20); printf("everybody who contributed");
    color(LGRAY); pspDebugScreenSetXY(x, y+21); printf("to the PSP hacking scene!");

}

/* void fill_info_kernel_colored(int x, int y) {
    color(YELLOW); pspDebugScreenSetXY(x, y+0); printf("Kernel Info: ");
    color(WHITE);  pspDebugScreenSetXY(x+12, y+0); printf("[future info here]");
    color(WHITE);
} */

// --- Display Pages ---
void displayMainPage(int page, int pageCount) {
    drawBorder();
    int ascii = ident_get_ascii();
        switch (ascii)
        {
        case 1:
            drawAsciiAndInfoBlock_colored(ascii_art_psp1000, 22, 19, fill_info_main_colored, 17);
            break;
        case 2:
            drawAsciiAndInfoBlock_colored(ascii_art_psp2000, 22, 19, fill_info_main_colored, 17);
            break;
        case 3:
            drawAsciiAndInfoBlock_colored(ascii_art_psp3000, 22, 19, fill_info_main_colored, 17);
            break;
        case 4:
            drawAsciiAndInfoBlock_colored(ascii_art_pspN1000, 22, 19, fill_info_main_colored, 17);
            break;
        case 5:
            drawAsciiAndInfoBlock_colored(ascii_art_pspE1000, 22, 19, fill_info_main_colored, 17);
            break;
        default:
            drawAsciiAndInfoBlock_colored(ascii_art_pspDefault, 22, 19, fill_info_main_colored, 17);
            break;
        }
    char bottom[64];
    snprintf(bottom, sizeof(bottom), "[Pg %d/%d] Use < > to switch, SELECT to ScrShot, START to exit", page+1, pageCount);
    printCenteredBottom(bottom, ORANGE);
}

void displaySecondPage(int page, int pageCount) {
    drawBorder();
    drawAsciiAndInfoBlock_colored(ascii_art_battery, 25, 18, fill_info_battery_colored, 16);
    char bottom[64];
    snprintf(bottom, sizeof(bottom), "[Pg %d/%d] Use < > to switch, SELECT to ScrShot, START to exit", page+1, pageCount);
    printCenteredBottom(bottom, BLUE);
}

void displayThirdPage(int page, int pageCount) {
    drawBorder();
    drawAsciiAndInfoBlock_colored(ascii_art_credits, 34, 22, fill_info_credits_colored, 22);
    char bottom[64];
    snprintf(bottom, sizeof(bottom), "[Pg %d/%d] Use < > to switch, SELECT to ScrShot, START to exit", page+1, pageCount);
    printCenteredBottom(bottom, GREEN);
}

/* void displayFourthPage(int page, int pageCount) {
    drawBorder();
    drawAsciiAndInfoBlock_colored(ascii_art_kernel, 13, 6, fill_info_kernel_colored, 1);
    char bottom[64];
    snprintf(bottom, sizeof(bottom), "[Page %d/%d] Use LEFT/RIGHT to switch, START to exit", page+1, pageCount);
    printCenteredBottom(bottom, RED);
} */


// --- Main program ---
int main(int argc, char *argv[])  {
    setup_callbacks();
    pspDebugScreenInit();

    printf("Starting NeoPspFetch...\n");

    doKernelExploit();
    pspXploitExecuteKernel(ident_collect_kernel_info);

    int page = 0;
    int pageCount = 3;
    int oldButtons = 0;
    int lastPage = -1;

    while (1) {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START) {
            break;
        }

        if ((pad.Buttons & PSP_CTRL_RIGHT) && !(oldButtons & PSP_CTRL_RIGHT)) {
            page = (page + 1) % pageCount;
        }
        if ((pad.Buttons & PSP_CTRL_LEFT) && !(oldButtons & PSP_CTRL_LEFT)) {
            page = (page + pageCount - 1) % pageCount;
        }

         // Take screenshot on SELECT button press
        if ((pad.Buttons & PSP_CTRL_SELECT) && !(oldButtons & PSP_CTRL_SELECT)) {
            take_screenshot();
        }

        oldButtons = pad.Buttons;

        if (page != lastPage) {
            sceDisplayWaitVblankStart();
            pspDebugScreenClear();
            if (page == 0) {
                displayMainPage(page, pageCount);
            } else if (page == 1) {
                displaySecondPage(page, pageCount);
            } else if (page == 2) {
                displayThirdPage(page, pageCount);
            } /* else if (page == 3) {
                displayFourthPage(page, pageCount);
            } */
            lastPage = page;
        }

        sceDisplayWaitVblankStart();
    }

    return 0;
}