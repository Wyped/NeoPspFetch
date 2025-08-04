#include <pspsysmem_kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/libpspexploit.h"
#include "kernel.h"

// Adapted from pspIdent by Yoti and friends, huge thanks for making the sources publicaly available!

// ----------- DATA STORAGE (globals) -----------
static char c2dqaf[2];
static char c2dreg[2];
static char idswfreg[2];
static char kirk[4];
static char model[64];
static char qaflag[2];
static char region[2];
static char shippedfw[5];
static char spock[4];
static char times[64]; 
static char tlotr[64];
static int ascii; 
static int baryon; 
static int flag = 0;
static int fusecfg;
static int generation;
static int polestar;
static int pommel;
static int scramble;
static int tachyon;
static long long fuseid;
static unsigned char idsbtmac[7];
static unsigned int bromver;
static u64 tick;
static int idswfbak = 1;

// ----------- KERNEL FUNCTION POINTERS -----------
static KernelFunctions _ktbl;
KernelFunctions* k_tbl = &_ktbl;

static int (*_sceIdStorageLookup)(int, int, void*, int) = NULL;
static int (*_sceKernelDelayThread)(int) = NULL;
static int (*_sceKernelExitDeleteThread)(int) = NULL;
static int (*_sceKernelGetModel)(void) = NULL;
static int (*_sceKernelUtilsSha1Digest)(u8*, int, u8*) = NULL;
static int (*_sceRtcGetCurrentTick)(void*) = NULL;
static int (*_sceSysconGetBaryonVersion)(int*) = NULL;
static int (*_sceSysconGetPolestarVersion)(int*) = NULL;
static int (*_sceSysconGetPommelVersion)(int*) = NULL;
static char (*_sceSysconGetTimeStamp)(char*) = NULL;
static int (*_sceSysregAtaBusClockEnable)(void) = NULL;
static int (*_sceSysregGetFuseConfig)(void) = NULL;
static int (*_sceSysregGetTachyonVersion)(void) = NULL;
static int (*_sceSysregKirkBusClockEnable)(void) = NULL;
static int (*_sceUmdExecInquiryCmd)(void*, int*, int*) = NULL;
static long long (*_sceSysregGetFuseId)(void) = NULL;

// ----------- EXTERNALS -----------
#define MAX_REGION_INDEX 15 // adjust to your region table

// ----------- STRUCTS -----------
typedef struct {
	unsigned char peripheral_device_type;
	unsigned char removable;
	unsigned char standard_ver;
	unsigned char atapi_response;
	unsigned int additional;
	char vendor_id[8];
	char product_id[16];
	char product_rev[4];
	char sony_spec[0x14];
} ATAPI_INQURIY;

static ATAPI_INQURIY ai;
static u8 param[4] = {0, 0, 0x38, 0};

// ----------- INTERNAL HELPERS -----------
static int prxKernelGetModel(void) {
    if (!_sceKernelGetModel) return -1;
	int k1 = pspSdkSetK1(0);
	int g = _sceKernelGetModel();
	pspSdkSetK1(k1);
	return g;
}

static int prxNandGetScramble(void) {
	int k1 = pspSdkSetK1(0);
	u32 value;
	u32 buf[4];
	u32 sha[5];
	buf[0] = *(vu32*)(0xBC100090); // volatile
	buf[1] = *(vu32*)(0xBC100094);
	buf[2] = *(vu32*)(0xBC100090)<<1;
	buf[3] = 0xD41D8CD9;
	if (_sceKernelUtilsSha1Digest)
		_sceKernelUtilsSha1Digest((u8*)buf, sizeof(buf), (u8*)sha);
	else
		memset(sha, 0, sizeof(sha));
	value = (sha[0] ^ sha[3]) + sha[2];
	pspSdkSetK1(k1);
	return value;
}

static int prxSysregGetKirkVersion(void) {
    if (!_sceSysregKirkBusClockEnable) return 0;
	int k1 = pspSdkSetK1(0);
	_sceSysregKirkBusClockEnable();
	if (_sceKernelDelayThread)
		_sceKernelDelayThread(1000);
	int kv = *(volatile int*)0xBDE00004;
	pspSdkSetK1(k1);
	return kv;
}

static int prxSysregGetSpockVersion(void) {
	int k1 = pspSdkSetK1(0);
	if (_sceSysregAtaBusClockEnable)
		_sceSysregAtaBusClockEnable();
	if (_sceKernelDelayThread)
		_sceKernelDelayThread(1000);
	int sv = *(int*)0xBDF00004;
	pspSdkSetK1(k1);
	return sv;
}

static int prxSysregGetTachyonVersion(void) {
	int k1 = pspSdkSetK1(0);
	int sv = -1;
	if (_sceSysregGetTachyonVersion)
		sv = _sceSysregGetTachyonVersion();
	pspSdkSetK1(k1);
	return sv;
}

static unsigned int prxTachyonGetTimeStamp(void) {
	unsigned int ts = 0;
	asm volatile("cfc0 %0, $17" : "=r" (ts));
	if (ts & 0x80000000)
		ts -= 0x80000000;
	return ts;
}

// ----------- KTHREAD - Data Collection -----------
static int ident_kthread(void) {
	memset(shippedfw, 0, sizeof(shippedfw));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x51, 0, shippedfw, 4);

	if (_sceSysconGetBaryonVersion)
		_sceSysconGetBaryonVersion(&baryon);
	else
		baryon = -1;

	bromver = prxTachyonGetTimeStamp();

	if (_sceSysconGetPolestarVersion)
		_sceSysconGetPolestarVersion(&polestar);
	else
		polestar = -1;

	if (_sceSysconGetPommelVersion)
		_sceSysconGetPommelVersion(&pommel);
	else
		pommel = -1;

	tachyon = prxSysregGetTachyonVersion();

	memset(times, 0, sizeof(times));
	if (_sceSysconGetTimeStamp)
		_sceSysconGetTimeStamp(times);

	if (_sceSysregGetFuseConfig)
		fusecfg = _sceSysregGetFuseConfig();
	else
		fusecfg = -1;

	if (_sceSysregGetFuseId)
		fuseid = _sceSysregGetFuseId();
	else
		fuseid = -1;

	// ----------- GENERATION PATCH -----------
	int g = -1;
	if (_sceKernelGetModel)
		g = prxKernelGetModel();
	generation = (g >= 0 && g < 10) ? (g + 1) : 1; // Default to 1 if invalid

	int kirk_val = 0;
	if (_sceSysregKirkBusClockEnable)
		kirk_val = prxSysregGetKirkVersion();
	if (kirk_val == 0 || kirk_val == -1)
		kirk_val = 0;
	*(int*)kirk = kirk_val;

	scramble = prxNandGetScramble();

	*(int*)spock = prxSysregGetSpockVersion();

	memset(model, 0, sizeof(model));
	memset(idsbtmac, 0, sizeof(idsbtmac));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0050, 0x41, &idsbtmac, 6);

	memset(idswfreg, 0, sizeof(idswfreg));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0045, 0x00, &idswfreg, 1);
	idswfbak = 1;
	if ((int)idswfreg[0] < 0) {
		idswfbak = (int)idswfreg[0];
		idswfreg[0] = '\1';
	}
	else if ((int)idswfreg[0] > ((sizeof(WiFiRegion) / sizeof(WiFiRegion[0])) - 1)) {
		idswfbak = (int)idswfreg[0];
		idswfreg[0] = '\1';
	}
	if ((int)idswfreg[0] == 1)
		flag = 1;

	memset(c2dreg, 0, sizeof(c2dreg));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0100, 0x3D, &c2dreg, 1);

	memset(region, 0, sizeof(region));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0100, 0xF5, &region, 1);

	memset(c2dqaf, 0, sizeof(c2dqaf));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0100, 0x40, &c2dqaf, 1);
	memset(qaflag, 0, sizeof(qaflag));
	if (_sceIdStorageLookup)
		_sceIdStorageLookup(0x0100, 0xF8, &qaflag, 1);
	memset(tlotr, 0, sizeof(tlotr));

	// --- Model detection ---
	switch(tachyon) {
		case 0x00140000:
			strcpy(tlotr, "First");
			ascii = 1;
			sprintf(model, "PSP-10%02i TA-079v", ModelRegion[(int)region[0]]);
			switch(baryon) {
				case 0x00010600:
					strcat(model, "1");
					if (pommel != 0x103) flag = 1;
				break;
				case 0x00020600:
					strcat(model, "2");
					if (pommel != 0x103) flag = 1;
				break;
				case 0x00030600:
					strcat(model, "3");
					if (pommel != 0x103) flag = 1;
				break;
				case 0x00010601:
					flag = 1;
					strcpy(tlotr, "Proto Tool");
					strcpy(model, "DEM-1000(?) TMU-001(?)");
				break;
				case 0x00020601:
					if (fusecfg != 0x2501) flag = 1;
					strcpy(tlotr, "Dev Tool");
					strcpy(model, "DTP-T1000 TMU-001");
				break;
				case 0x00030601:
					if ((int)region[0] == 0x02) {
						if (fusecfg != 0x2501) flag = 1;
						strcpy(tlotr, "Test Tool");
						strcpy(model, "DTP-H1500 TMU-002");
					} else if ((int)region[0] == 0x0e) {
						if (fusecfg != 0x1b01) flag = 1;
						strcpy(tlotr, "Test Tool for AV");
						strcpy(model, "DTP-L1500 TMU-002");
					} else {
						flag = 1;
						strcpy(tlotr, "Unknown Test Tool(?)");
						strcpy(model, "Unknown model and mobo");
					}
				break;
				default:
					flag = 1;
					strcat(model, "?");
				break;
			}
		break;
		case 0x00200000:
			strcpy(tlotr, "First");
			ascii = 1;
			sprintf(model, "PSP-10%02i TA-079v", ModelRegion[(int)region[0]]);
			switch(baryon) {
				case 0x00030600:
					strcat(model, "4");
					if (pommel != 0x103) flag = 1;
				break;
				case 0x00040600:
					strcat(model, "5");
					if (pommel != 0x103) flag = 1;
				break;
				default:
					flag = 1;
					strcat(model, "?");
				break;
			}
		break;
		case 0x00300000:
			strcpy(tlotr, "First");
			ascii = 1;
			sprintf(model, "PSP-10%02i TA-081v", ModelRegion[(int)region[0]]);
			switch(pommel) {
				case 0x00000103:
					strcat(model, "1");
					if (pommel != 0x103) flag = 1;
				break;
				case 0x00000104:
					strcat(model, "2");
					if (pommel != 0x104) flag = 1;
				break;
				default:
					flag = 1;
					strcat(model, "?");
				break;
			}
		break;
		case 0x00400000:
			strcpy(tlotr, "Legolas");
			ascii = 1;
			sprintf(model, "PSP-10%02i TA-08", ModelRegion[(int)region[0]]);
			switch(baryon) {
				case 0x00114000:
					strcat(tlotr, "1");
					strcat(model, "2");
					if (pommel != 0x112) flag = 1;
				break;
				case 0x00121000:
					strcat(tlotr, "2");
					strcat(model, "6");
					if (pommel != 0x112) flag = 1;
				break;
				default:
					flag = 1;
					strcat(tlotr, "?");
					strcat(model, "?");
				break;
			}
		break;
		case 0x00500000:
			strcpy(tlotr, "Frodo");
			ascii = 2;
			sprintf(model, "PSP-20%02i TA-0", ModelRegion[(int)region[0]]);
			switch(baryon) {
				case 0x0022B200:
					strcat(model, "85v1");
					if (pommel != 0x123) flag = 1;
				break;
				case 0x00234000:
					strcat(model, "85v2");
					if (pommel != 0x123) flag = 1;
				break;
				case 0x00243000:
					switch(pommel) {
						case 0x00000123:
							if (shippedfw[3] != '5')
								strcat(model, "88v1");
							else
								strcat(model, "88v1/v2");
							if (pommel != 0x123) flag = 1;
						break;
						case 0x00000132:
							flag = 1;
							strcat(model, "90v1");
							if (pommel != 0x123) flag = 1;
						break;
						default:
							flag = 1;
							strcat(model, "??");
						break;
					}
				break;
				default:
					flag = 1;
					strcat(model, "??");
				break;
			}
		break;
		case 0x00600000:
			switch(baryon) {
				case 0x00234000:
					flag = 1;
					strcpy(tlotr, "Frodo");
					ascii = 2;
					sprintf(model, "PSP-20%02i TA-088v3/TA-085v2 (hybrid)", ModelRegion[(int)region[0]]);
				break;
				case 0x00243000:
					strcpy(tlotr, "Frodo");
					ascii = 2;
					sprintf(model, "PSP-20%02i TA-088v3", ModelRegion[(int)region[0]]);
					if (pommel != 0x123) flag = 1;
				break;
				case 0x00263100:
					strcpy(tlotr, "Samwise");
					ascii = 3;
					sprintf(model, "PSP-30%02i TA-090v", ModelRegion[(int)region[0]]);
					switch(pommel) {
						case 0x00000132: strcat(model, "2"); break;
						case 0x00000133: strcat(model, "3"); break;
						default: flag = 1; strcat(model, "?"); break;
					}
				break;
				case 0x00285000:
					strcpy(tlotr, "Samwise");
					ascii = 3;
					sprintf(model, "PSP-30%02i TA-092", ModelRegion[(int)region[0]]);
					if (pommel != 0x133) flag = 1;
				break;
				default:
					flag = 1;
					strcpy(tlotr, "???");
					strcpy(model, "PSP-?000 TA-0??");
				break;
			}
		break;
		case 0x00720000:
			strcpy(tlotr, "Strider");
			ascii = 4;
			sprintf(model, "PSP-N10%02i TA-091", ModelRegion[(int)region[0]]);
			if (pommel != 0x133) flag = 1;
		break;
		case 0x00810000:
			switch(baryon) {
				case 0x002C4000:
					strcpy(tlotr, "Samwise VA2");
					ascii = 3;
					sprintf(model, "PSP-30%02i TA-093v", ModelRegion[(int)region[0]]);
					switch(pommel) {
						case 0x00000141: strcat(model, "1"); break;
						case 0x00000143: strcat(model, "2"); break;
						default: flag = 1; strcat(model, "?"); break;
					}
				break;
				case 0x002E4000:
					strcpy(tlotr, "Samwise VA2");
					ascii = 3;
					sprintf(model, "PSP-30%02i TA-095v1", ModelRegion[(int)region[0]]);
					if (pommel != 0x154) flag = 1;
				break;
				case 0x012E4000:
					strcpy(tlotr, "Samwise VA2");
					ascii = 3;
					sprintf(model, "PSP-30%02i TA-095v3", ModelRegion[(int)region[0]]);
					if (pommel != 0x154) flag = 1;
				break;
				case 0x00323100:
					flag = 1;
					strcpy(tlotr, "Strider2");
					ascii = 4;
					sprintf(model, "PSP-N10%02i TA-094 0-ST2-001-A2", ModelRegion[(int)region[0]]);
				break;
				case 0x00324000:
					flag = 1;
					strcpy(tlotr, "Strider2");
					ascii = 4;
					sprintf(model, "PSP-N10%02i TA-094 0-ST2-001-U4", ModelRegion[(int)region[0]]);
				break;
				default:
					flag = 1;
					strcpy(tlotr, "???");
					sprintf(model, "PSP-?0%02i TA-09?", ModelRegion[(int)region[0]]);
				break;
			}
		break;
		case 0x00820000:
			strcpy(tlotr, "Samwise VA2");
			ascii = 3;
			sprintf(model, "PSP-30%02i TA-095v", ModelRegion[(int)region[0]]);
			switch(baryon) {
				case 0x002E4000:
					strcat(model, "2");
					if (pommel != 0x154) flag = 1;
				break;
				case 0x012E4000:
					strcat(model, "4");
					if (pommel != 0x154) flag = 1;
				break;
				default:
					flag = 1;
					strcat(model, "?");
				break;
			}
		break;
		case 0x00900000:
			strcpy(tlotr, "Bilbo");
			ascii = 5;
			sprintf(model, "PSP-E10%02i TA-096", ModelRegion[(int)region[0]]);
			if (shippedfw[2] != '5')
				strcat(model, "/TA-097");
			if (pommel != 0x154) flag = 1;
		break;
		default:
			flag = 1;
			strcpy(tlotr, "???");
			strcpy(model, "PSP-?000 TA-0??");
		break;
	}

	if (((unsigned char)c2dreg[0] == 0x02) && ((unsigned char)region[0] != (unsigned char)c2dreg[0]))
		strcat(tlotr, " (fake Test Tool)");

	if ((generation == 4) && (baryon == 0x002E4000)) {
		strcat(model, " (fake 04g/real 09g)");
	} else {
		char real_gen[16] = "\0";
		sprintf(real_gen, " (%02ig)", generation);
		strcat(model, real_gen);
	}

	if ((unsigned char)c2dqaf[0] == 0x8C) {
		if ((unsigned char)qaflag[0] != (unsigned char)c2dqaf[0])
			strcat(model, " [fake QAF]");
		else
			strcat(model, " [QAF]");
	} else {
		strcat(model, " [non-QAF]");
	}

	if (_sceKernelDelayThread)
		_sceKernelDelayThread(1*1000*1000);
	if (_sceKernelExitDeleteThread)
		_sceKernelExitDeleteThread(0);
	return 0;
}

// ----------- KERNEL BOOTSTRAP -----------
void ident_collect_kernel_info(void) {
	int k1 = pspSdkSetK1(0);
	int userlevel = pspXploitSetUserLevel(8);

	pspXploitRepairKernel();
	pspXploitScanKernelFunctions(k_tbl);

	char*sysreg_mod = (pspXploitFindTextAddrByName("sceLowIO_Driver") == 0) ? "sceSYSREG_Driver" : "sceLowIO_Driver";

	_sceSysregGetTachyonVersion = pspXploitFindFunction(sysreg_mod, "sceSysreg_driver", 0xE2A5D1EE);
	_sceSysconGetBaryonVersion = pspXploitFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
	_sceSysconGetPolestarVersion = pspXploitFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0xFB148FB6);
	_sceIdStorageLookup = pspXploitFindFunction("sceIdStorage_Service", "sceIdStorage_driver", 0x6FE062D1);
	_sceSysconGetPommelVersion = pspXploitFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0xE7E87741);
	_sceSysconGetTimeStamp = pspXploitFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7BCC5EAE);
	_sceSysregGetFuseConfig = pspXploitFindFunction(sysreg_mod, "sceSysreg_driver", 0x8F4F4E96);
	_sceSysregGetFuseId = pspXploitFindFunction(sysreg_mod, "sceSysreg_driver", 0x4F46EEDE);
	_sceSysregAtaBusClockEnable = pspXploitFindFunction(sysreg_mod, "sceSysreg_driver", 0x16909002);
	_sceSysregKirkBusClockEnable = pspXploitFindFunction(sysreg_mod, "sceSysreg_driver", 0xBBC721EA);
	_sceKernelGetModel = pspXploitFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x07C586A1);

	_sceKernelDelayThread = pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0xCEADEB47);
	_sceKernelExitDeleteThread = pspXploitFindFunction("sceThreadManager", "ThreadManForUser", 0x809CE29B);
	_sceRtcGetCurrentTick = pspXploitFindFunction("sceRTC_Service", "sceRtc", 0x3F7AD767);
	_sceKernelUtilsSha1Digest = pspXploitFindFunction("sceSystemMemoryManager", "UtilsForUser", 0x840259F1);

	SceUID kthreadID = k_tbl->KernelCreateThread("ident_kthread", (void*)KERNELIFY(&ident_kthread), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
	if (kthreadID >= 0){
		k_tbl->KernelStartThread(kthreadID, 0, NULL);
		k_tbl->waitThreadEnd(kthreadID, NULL);
	}

	pspXploitSetUserLevel(userlevel);
	pspSdkSetK1(k1);
}

// ----------- GETTERS -----------
const char* ident_get_model(void)      { return model; }
const char* ident_get_tlotr(void)      { return tlotr; }
const char* ident_get_shippedfw(void)  { return shippedfw; }
int ident_get_ascii(void)              { return ascii; }
int ident_get_baryon(void)             { return baryon; }
int ident_get_tachyon(void)            { return tachyon; }
int ident_get_pommel(void)             { return pommel; }
int ident_get_polestar(void)           { return polestar; }
int ident_get_fusecfg(void)            { return fusecfg; }
long long ident_get_fuseid(void)       { return fuseid; }
int ident_get_generation(void)         { return generation; }
unsigned int ident_get_bromver(void)   { return bromver; }
unsigned int ident_get_scramble(void)  { return scramble; }
const char* ident_get_times(void)      { return times; }
int ident_get_flag(void)               { return flag; }
const unsigned char* ident_get_idsbtmac(void) { return idsbtmac; }
int ident_get_idswfbak(void)           { return idswfbak; }

static void hex4_to_str(const char* src, char* dst) {
    sprintf(dst, "%02x%02x%02x%02x", (unsigned char)src[3], (unsigned char)src[2], (unsigned char)src[1], (unsigned char)src[0]);
}
const char* ident_get_kirk_str(void)  { static char buf[9]; hex4_to_str(kirk, buf); return (*(int*)kirk == 0 ? "Unknown" : buf); }
const char* ident_get_spock_str(void) { static char buf[9]; hex4_to_str(spock, buf); return buf; }

const char* ident_get_wifi_region(void) {
    if ((int)idswfreg[0] != 1 && (unsigned char)idswfreg[0] <= MAX_REGION_INDEX)
        return WiFiRegion[(int)idswfreg[0]];
    else
        return "unknown";
}

void ident_print_warn(void) {
	int i;
	printf("\033[33m"); // YELLOW
	for (i = 0; i < 51; i++) printf("-");
	printf("\n");
	printf(" Please send this screenshot to the Yoti's GitHub!\n");
	printf("  https://github.com/Yoti/psp_pspident/issues/new\n");
	for (i = 0; i < 51; i++) printf("-");
	printf("\n\n");
	printf("\033[0m"); // WHITE
}