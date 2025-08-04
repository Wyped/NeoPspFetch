#ifndef IDENT_PBP_KERNEL_H
#define IDENT_PBP_KERNEL_H


#define printf pspDebugScreenPrintf
#define color(x) pspDebugScreenSetTextColor(x)
#define RED 0xff0000ff
#define BLUE 0xffff0000
#define GREEN 0xff00ff00
#define LGRAY 0xffd3d3d3
#define WHITE 0xffffffff
#define YELLOW 0xff00ffff
#define ORANGE 0xff007fff

static int ModelRegion[16] = {0, 0, 0, 0, 1, 4, 5, 3, 10, 2, 6, 7, 8, 9, 0, 0};
static unsigned char WiFiRegion[4][16] = {"US (1-11)", "?? (1-??)", "EU (1-13)", "JP (1-14)"};
static unsigned char wiki[4][768] = {
	"Akvavit or aquavit is a distilled spirit that is principally\nproduced in Scandinavia, where it has been produced since the 15th\ncentury. Akvavit is distilled from grain or potatoes, and is\nflavoured with a variety of herbs. It is also popular in Northern\nGermany.",
	"Bourbon is a type of barrel-aged American whiskey made primarily\nfrom corn (maize). The name derives from the French Bourbon\ndynasty, although the precise source of inspiration is uncertain;\ncontenders include Bourbon County in Kentucky and Bourbon Street\nin New Orleans, both of which are named after the dynasty.",
	"Campari is an Italian alcoholic liqueur, considered an aperitif\n(20.5%, 21%, 24%, 25%, or 28.5% ABV, depending on the country where\nit is sold), obtained from the infusion of herbs and fruit\n(including chinotto and cascarilla) in alcohol and water. It is a\ntype of bitters, characterised by its dark red colour."
	"Drambuie is a golden-coloured, 40% ABV liqueur made from Scotch\nwhisky, heather honey, herbs and spices. The brand was owned by the\nMacKinnon family for 100 years, and was bought by William Grant &\nSons in 2014."
};

void ident_collect_kernel_info(void);

const char* ident_get_model(void);
const char* ident_get_tlotr(void);
const char* ident_get_shippedfw(void);
int ident_get_ascii(void);
int ident_get_baryon(void);
int ident_get_tachyon(void);
int ident_get_pommel(void);
int ident_get_polestar(void);
int ident_get_fusecfg(void);
long long ident_get_fuseid(void);
int ident_get_generation(void);
const char* ident_get_kirk_str(void);
const char* ident_get_spock_str(void);
const char* ident_get_times(void);
unsigned int ident_get_bromver(void);
unsigned int ident_get_scramble(void);
const unsigned char* ident_get_idsbtmac(void);
const char* ident_get_wifi_region(void);
int ident_get_flag(void);
int ident_get_idswfbak(void);

// UMD drive info
/* int ident_get_umd_found(void);
const char* ident_get_umd_fw(void); */

void ident_print_warn(void);

#endif 
/*
	-Wint-conversion
	-Wincompatible-pointer-types
	-Wint-conversion
	-Wstringop-truncation
*/
