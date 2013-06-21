#include "nds.h"
#include <string.h>


#include <stdarg.h>
#include <ctype.h>

#include "types.h"
#include "autotype.h"
#include "plateform.h"
#include "snapshot.h"
#include "config.h"
#include "sound.h"

#ifdef USE_Z80_ORIG
#include "z80.h"
#endif

#ifdef USE_Z80_ASM2
#include "z80_asm.h"
#endif


#include "ppi.h"
#include "vga.h"
#include "upd.h"
#include "crtc.h"



#include "cpcfont.h"

IPC ipc;
int isPaused;
int *borderX, *borderY;

#define timers2ms(tlow,thigh)(tlow | (thigh<<16)) >> 5

#define AlphaBlendFast(pixel,backpixel) (((((pixel) & 0x7bde) >> 1) | (((backpixel) & 0x7bde) >> 1)) | 0x8000)

extern int turbo;        // dans main.c

int usestylus=0, usestylusauto=1;
int usemagnum=0;

u16 BG_PALETTE[32];

static int hack_tabcoul=0;

static u16 AlphaBlend(u16 pixel, u16 backpixel, u16 opacity)
{
	// Blend image with background, based on opacity
	// Code optimization from http://www.gamedev.net/reference/articles/article817.asp
	// result = destPixel + ((srcPixel - destPixel) * ALPHA) / 256

	u16 dwAlphaRBtemp = (backpixel & 0x7c1f);
	u16 dwAlphaGtemp = (backpixel & 0x03e0);
	u16 dw5bitOpacity = (opacity >> 3);

	return (
		((dwAlphaRBtemp + ((((pixel & 0x7c1f) - dwAlphaRBtemp) * dw5bitOpacity) >> 5)) & 0x7c1f) |
		((dwAlphaGtemp + ((((pixel & 0x03e0) - dwAlphaGtemp) * dw5bitOpacity) >> 5)) & 0x03e0) | 0x8000
	);
}


#define MAX_ROM_MODS 2
#include "rom_mods.h"

/*
inline char toupper(const char toLower)
        {
            if ((toLower >= 'a') && (toLower <= 'z'))
                return char(toLower - 0x20);
            return toLower;
        }
*/

int emulator_patch_ROM (u8 *pbROMlo)
{
    u8 *pbPtr;
    // int CPCkeyboard=1; // French 
    int CPCkeyboard=0;  // Default
	
    if (CPCkeyboard<1) {
        return 0;
    }
	
	// pbPtr = pbROMlo + 0x1d69; // location of the keyboard translation table on 664 or 6128
	pbPtr = pbROMlo + 0x1eef; // location of the keyboard translation table on 6128    
	memcpy(pbPtr, cpc_keytrans[CPCkeyboard-1], 240); // patch the CPC OS ROM with the chosen keyboard layout
	
	pbPtr = pbROMlo + 0x3800;
	memcpy(pbPtr, cpc_charset[CPCkeyboard-1], 2048); // add the corresponding character set
	
	return 0;
}

#define MAXFILE 1024

int resize=1;

static char currentfile[256];
static int currentsnap=0; // 0,1 ou 2
int snapsave=0;

u16 *menubuffer;
u16 *menubufferlow;

int consolepos=0;
static char consolestring[1024];

int Fmnbr;

void myconsoleClear(void);

void SetRect(RECT *R, int left, int top, int right, int bottom);
void FillRect(RECT *R, u16 color);
void DrawRect(RECT *R, u16 color);

void UpdateKeyMenu(void);


int UpdateInk=1;

pfctDraw DrawFct;

static int x0,y0;
static int maxy;

// 384


pfctExecInstZ80 ExecInstZ80;
pfctResetZ80 ResetZ80;
pfctSetIRQZ80 SetIRQZ80;

static u8 bit_values[8] = {
	   0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};



int dispframerate=0;
static int frame=0, msgframe=0;
static char msgbuf[33] = {0};

static int Regs1=0, Regs2=0, Regs6=0, Regs7=0;       // Utilisé par le resize automatique

u16 *kbdBuffer;

int inMenu=0;
int keyEmul=3; // 2: Emul du clavier, 3: normal




static u16 *MemBitmap;
static u16 MemBitmap_width;
int Turbo = 1;

#ifdef USE_DEBUG
int DebugMode = 0;
int Cont = 1;
#endif

static u16 TabPoints[ 4 ][ 256 ][ 4 ];
static u8 TabPointsDef[ 4 ][ 256 ][ 4 ];

char *keyname0[] = {
    	"CURSOR_UP", // = 0
		"CURSOR_RIGHT",
		"CURSOR_DOWN",
		"F9",
		"F6",
		"F3",
		"SMALL_ENTER",
		"FDOT",
		/* line 1", bit 0..bit 7 */
		"CURSOR_LEFT",
		"COPY",
		"F7",
		"F8",
		"F5",
		"F1",
		"F2",
		"F0",
		/* line 2", bit 0..bit 7 */
		"CLR",
		"OPEN_SQUARE_BRACKET",
		"RETURN",
		"CLOSE_SQUARE_BRACKET",
		"F4",
		"SHIFT",
		"FORWARD_SLASH",
		"CONTROL",
		/* line 3", bit 0.. bit 7 */
		"HAT",
		"MINUS",
		"AT",
		"P",
		"SEMICOLON",
		"COLON",
		"BACKSLASH",
		"DOT",
		/* line 4", bit 0..bit 7 */
		"ZERO",
		"9",
		"O",
		"I",
		"L",
		"K",
		"M",
		"COMMA",
		/* line 5", bit 0..bit 7 */
		"8",
		"7",
		"U",
		"Y",
		"H",
		"J",
		"N",
		"SPACE",
		/* line 6", bit 0..bit 7 */
		"6",
		"5",
		"R",
		"T",
		"G",
		"F",
		"B",
		"V",
		/* line 7", bit 0.. bit 7 */
		"4",
		"3",
		"E",
		"W",
		"S",
		"D",
		"C",
		"X",
		/* line 8", bit 0.. bit 7 */
		"1",
		"2",
		"ESC",
		"Q",
		"TAB",
		"A",
		"CAPS_LOCK",
		"Z",
		/* line 9", bit 7..bit 0 */
		"JOY_UP",
		"JOY_DOWN",
		"JOY_LEFT",
		"JOY_RIGHT",
		"JOY_FIRE1",
		"JOY_FIRE2",
		"SPARE",
		"DEL", 
		
		/* no key press */
		"NIL"
};

int RgbCPCdef[ 32 ] =  {
	    0x7F7F7F,                 // Blanc            (13)
		0x7F7F7F,                 // Blanc            (13)
		0x00FF7F,                 // Vert Marin       (19)
		0xFFFF7F,                 // Jaune Pastel     (25)
		0x00007F,                 // Bleu              (1)
		0xFF007F,                 // Pourpre           (7)
		0x007F7F,                 // Turquoise        (10)
		0xFF7F7F,                 // Rose             (16)
		0xFF007F,                 // Pourpre           (7)
		0xFFFF00,                 // Jaune vif        (24)
		0xFFFF00,                 // Jaune vif        (24)
		0xFFFFFF,                 // Blanc Brillant   (26)
		0xFF0000,                 // Rouge vif         (6)
		0xFF00FF,                 // Magenta vif       (8)
		0xFF7F00,                 // Orange           (15)
		0xFF7FFF,                 // Magenta pastel   (17)
		0x00007F,                 // Bleu              (1)
		0x00FF7F,                 // Vert Marin       (19)
		0x00FF00,                 // Vert vif         (18)
		0x00FFFF,                 // Turquoise vif    (20)
		0x000000,                 // Noir              (0)
		0x0000FF,                 // Bleu vif          (2)
		0x007F00,                 // Vert              (9)
		0x007FFF,                 // Bleu ciel        (11)
		0x7F007F,                 // Magenta           (4)
		0x7FFF7F,                 // Vert pastel      (22)
		0x7FFF00,                 // Vert citron      (21)
		0x7FFFFF,                 // Turquoise pastel (23)
		0x7F0000,                 // Rouge             (3)
		0x7F00FF,                 // Mauve             (5)
		0x7F7F00,                 // Jaune            (12)
		0x7F7FFF                  // Bleu pastel      (14)
};

void TraceLigne8B512( int y, signed int AdrLo, int AdrHi );


typedef struct {
	int normal;
} CPC_MAP;

int cpckeypressed[NBCPCKEY];

RECT keypos[NBCPCKEY] = { 
    {0,51,14,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{15,51,33,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{34,51,52,72}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	{53,51,70,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{71,51,87,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{88,51,104,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                
	{105,51,121,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{122,51,138,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{139,51,155,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                        
	{156,51,172,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{173,51,189,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{190,51,206,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	{207,51,223,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_LEFT
	{224,51,240,72},   //    0x41 | MOD_CPC_SHIFT,   // CPC_UP
	{241,51,255,72},  // CP_RIGHT
	    
	{0,73,14,94},    // (0)
	{15,73,33,94},   //    0x80 | MOD_CPC_SHIFT,   // CPC_1
	{34,73,52,94},   //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{53,73,70,94}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{71,73,87,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{88,73,104,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{105,73,121,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{122,73,138,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{139,73,155,94}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{156,73,172,94}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{173,73,189,94}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
	{190,73,206,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_=
	{207,73,223,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_LAMBDA
	{224,73,240,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_CLR
	{241,73,256,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_DEL

	{0,95,19,116},
	{20,95,38,116}, //    0x83,                   // CPC_a
	{39,95,57,116}, //    0x73,                   // CPC_z
	{58,95,76,116}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{77,95,95,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{96,95,114,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{115,95,133,116}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{134,95,152,116}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{153,95,171,116}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{172,95,190,116}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{191,95,207,116}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{208,95,224,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{225,95,241,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{242,95,256,138},//    0x22,                   // CPC_RETURN
	
	{0,117,21,138},
	{22,117,40,138}, //    0x83,                   // CPC_A
	{41,117,59,138}, //    0x73,                   // CPC_S
	{60,117,78,138}, //    0x71 | MOD_CPC_SHIFT,   // CPC_D
	{79,117,97,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_F
	{98,117,116,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_G
	{117,117,135,138}, //    0x60 | MOD_CPC_SHIFT,   // CPC_H
	{136,117,154,138}, //    0x51 | MOD_CPC_SHIFT,   // CPC_J
	{155,117,173,138}, //    0x50 | MOD_CPC_SHIFT,   // CPC_K
	{174,117,190,138}, //    0x41 | MOD_CPC_SHIFT,   // CPC_L
	{191,117,207,138}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{208,117,224,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{225,117,241,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5        
	
	{0,139,28,160},  // SHIFT
	{29,139,47,160}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{48,139,66,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{67,139,85,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{86,139,104,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{105,139,123,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{124,139,142,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{143,139,161,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{162,139,178,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{179,139,195,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{196,139,212,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{213,139,229,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{230,139,256,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	
	{0,161,57,182}, //    0x55,                   // CPC_j
	{58,161,95,182}, //    0x55,                   // CPC_j        
	{96,161,207,182}, //    0x55,                   // CPC_j        
	{208,161,256,182} //    0x55,                   // CPC_j        	
};

/*
RECT keypos[NBCPCKEY] = { 
	{2,116,15,130},    // (0)
	{16,116,29,130},   //    0x80 | MOD_CPC_SHIFT,   // CPC_1
	{30,116,43,130},   //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{44,116,57,130}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{58,116,71,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{72,116,85,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{86,116,99,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{100,116,113,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{114,116,127,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{128,116,141,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{142,116,155,130}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
	{156,116,169,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{170,116,183,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{184,116,197,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{198,116,211,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{213,116,226,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{227,116,240,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{241,116,254,130}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	
	{2,131,19,145},
	{20,131,33,145}, //    0x83,                   // CPC_a
	{34,131,47,145}, //    0x73,                   // CPC_z
	{48,131,61,145}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{62,131,75,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{76,131,89,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{90,131,103,145}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{104,131,117,145}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{118,131,131,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{132,131,145,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{146,131,159,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{160,131,173,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{174,131,187,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{191,131,211,160},//    0x22,                   // CPC_RETURN
	{213,131,226,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{227,131,240,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{241,131,254,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                
	
	{2,146,22,160},
	{23,146,36,160}, //    0x83,                   // CPC_a
	{37,146,50,160}, //    0x73,                   // CPC_z
	{51,146,64,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{65,146,78,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{79,146,92,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{93,146,106,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{107,146,120,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{121,146,134,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{135,146,148,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{149,146,162,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{163,146,176,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{177,146,190,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5        
	{213,146,226,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{227,146,240,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{241,146,254,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0                        
	
	{2,161,29,175},
	{30,161,43,175}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
	{44,161,57,175}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
	{58,161,71,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{72,161,85,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{86,161,99,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{100,161,113,175}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
	{114,161,127,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{128,161,141,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{142,161,155,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
	{156,161,169,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
	{170,161,183,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
	{184,161,211,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
	{213,161,227,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{228,161,240,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{241,161,254,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0        
	
	{1,176,34,190}, //    0x55,                   // CPC_j
	{35,176,55,190}, //    0x55,                   // CPC_j        
	{56,176,167,190}, //    0x55,                   // CPC_j        
	{168,176,211,190}, //    0x55,                   // CPC_j        
	{213,176,227,190}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
	{228,176,240,190}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
	{241,176,254,190}
};
*/



CPC_SCANCODE keyown[11];

CPC_MAP keymap[NBCPCKEY] = {
	{ CPC_FDOT },	    
	{ CPC_F1 },   
	{ CPC_F2 },   
	{ CPC_F3 },       
	{ CPC_F4 },   
	{ CPC_F5 },      
	{ CPC_F6 },
	{ CPC_F7 },
	{ CPC_F8 },
	{ CPC_F9 },
	{ CPC_F0 }, 
	{ CPC_CURSOR_UP },            	
	{ CPC_CURSOR_LEFT },   
	{ CPC_CURSOR_DOWN },            
	{ CPC_CURSOR_RIGHT },

	{ CPC_ESC },
	{ CPC_1 },
	{ CPC_2 },
	{ CPC_3 },
	{ CPC_4 },
	{ CPC_5 },
	{ CPC_6 },            
	{ CPC_7 },
	{ CPC_8 },      
	{ CPC_9 },         
	{ CPC_ZERO },         
	{ CPC_MINUS },   
	{ CPC_HAT },   
	{ CPC_CLR },
	{ CPC_DEL },
	
	{ CPC_TAB },       //
	{ CPC_Q },         
	{ CPC_W },         
	{ CPC_E },            
	{ CPC_R },            
	{ CPC_T },            
	{ CPC_Y },   
	{ CPC_U },      
	{ CPC_I },   
	{ CPC_O },   
	{ CPC_P },   
	{ CPC_AT },   
	{ CPC_OPEN_SQUARE_BRACKET },            
	{ CPC_RETURN },            
   	
	{ CPC_CAPS_LOCK },   //
	{ CPC_A },   
	{ CPC_S },   
	{ CPC_D },   
	{ CPC_F },   
	{ CPC_G },      
	{ CPC_H },   
	{ CPC_J },   
	{ CPC_K },   
	{ CPC_L },   
	{ CPC_COLON },   
	{ CPC_SEMICOLON },   
	{ CPC_CLOSE_SQUARE_BRACKET },   

	{ CPC_SHIFT },   //
	{ CPC_Z },   
	{ CPC_X },   
	{ CPC_C },   
	{ CPC_V },   
	{ CPC_B },   
	{ CPC_N },   
	{ CPC_M },            
	{ CPC_COMMA },   
	{ CPC_DOT },   
	{ CPC_FORWARD_SLASH },   
	{ CPC_BACKSLASH },   
	{ CPC_SHIFT },   
   
	
	{ CPC_CONTROL },   
	{ CPC_COPY },   
	{ CPC_SPACE },   
	{ CPC_SMALL_ENTER }
};







void SelectSNAP(void);

/*
void LireRom(struct kmenu *FM, int autostart)
{
    u8 *rom=NULL;
    u32 romsize=0;
    char autofile[256];
    
    myprintf("Loading %s", FM->object); // FM->title
    rom = FS_Readfile(FM->object, &romsize);
	
    if (rom==NULL) {
        myprintf("Rom not found");
        return;
    }
	
#ifdef USE_SNAPSHOT    
    if (!memcmp(rom, "MV - SNA", 8 )) {
        LireSnapshotMem(rom);
        currentsnap=0;
        strcpy(currentfile, FM->object);
    }
#endif
    if ( (!memcmp(rom, "MV - CPCEMU", 11)) || (!memcmp(rom,"EXTENDED", 8)) ) {        
        autofile[0]=0;
        myprintf("Load Disk");
        LireDiskMem(rom, romsize, autofile);
        myprintf("Disk loaded");
        if ((autofile[0]!=0) && (autostart)) {
            char buffer[256];
            sprintf(buffer,"run\"%s\n", autofile);
            AutoType_SetString(buffer, TRUE);
        }
        currentsnap=0;
        strcpy(currentfile, FM->object);
    }
    free(rom);

#ifdef USE_SAVESNAP
    if (snapsave) {
        SelectSNAP();
    }
#endif
    
}
 */
 

void SelectSNAP(void)
{
#ifdef USE_FAT
#ifndef USE_CONSOLE
    u16 keys_pressed;

    dmaCopy(menubufferlow, backBuffer, 256*192*2);

    currentsnap=0;

// Wait key off
    do {
        keys_pressed = ~(REG_KEYINPUT);
    } while ((keys_pressed & (KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

    DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15,  1, "Select Save Slot");

    int x;
    RECT r;

    for(x=0;x<3;x++) {
        char id[2];
        char *buf;
        char snap[256];
        int haveimg, havesnap;


        sprintf(snap, "/%s.%d", currentfile, currentsnap+1);
        buf=strchr(snap,'.');
        *buf='_';
        havesnap = FileExists(snap);

        sprintf(snap, "/%s_i.%d", currentfile, currentsnap+1);
        buf=strchr(snap,'.');
        *buf='_';
        haveimg = FileExists(snap);

        sprintf(id,"%d", x+1);
        SetRect(&r, 5+x*83, 130, 5+x*83+80, 50+130);
        FillRect(&r, RGB15((156>>3), (178>>3), (165>>3))|0x8000);
	    DrawText(backBuffer, font, 7+x*83, 132, id);
        if (!havesnap) {
	        DrawText(backBuffer, font, 7+x*83, 152, "Empty");
        }
        }

    while(1) {
         for(x=0;x<3;x++) {
            SetRect(&r, 4+x*83, 129, 6+x*83+80, 51+130);

            if (x==currentsnap) {
                DrawRect(&r,  RGB15(0,31,31) | 0x8000);
            } else {
                DrawRect(&r,  RGB15(16,0,0) | 0x8000);
            }
        }

        keys_pressed = MyReadKey();

        if ((keys_pressed & KEY_LEFT)==KEY_LEFT) {
            if (currentsnap==0) {
                currentsnap=2;
            } else {
                currentsnap--;
            }
        }
        if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) {
            if (currentsnap==2) {
                currentsnap=0;
            } else {
                currentsnap++;
            }
        }

        if ((keys_pressed & KEY_A)==KEY_A) {
            break;
        }

    }
#endif
#endif
    return;
}




void SetRect(RECT *R, int left, int top, int right, int bottom)
{
    R->left=left;
    R->top=top;
    R->right=right;
    R->bottom=bottom;
}

#ifndef USE_CONSOLE
void FillRect(RECT *R, u16 color)
{
    int x,y;
    
    for(y=R->top;y<R->bottom;y++) {
		for(x=R->left;x<R->right;x++) {
			backBuffer[x+y*256]=color;
		}
	}    
}

void DrawRect(RECT *R, u16 color)
{
    int x,y;
    
    for(y=R->top;y<R->bottom;y++) {
		backBuffer[R->left+y*256]=color;
		backBuffer[(R->right-1)+y*256]=color;
	}    
	for(x=R->left;x<R->right;x++) {
		backBuffer[x+R->top*256]=color;
		backBuffer[x+(R->bottom-1)*256]=color;
    }
}

void DrawLift(RECT *max, RECT *dest, u16 coloron, u16 coloroff)
{
	int sizelift=8;
	RECT *r;
	RECT r0;
	/*	
	if ( (max->right-max->left) > (dest->right-dest->left) ) {
	int x1,x2;
	
	 int dr0,dr,dl,mr,ml;
	 
	  dr=dest->right;
	  if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
	  dr0=dest->right-sizelift;
	  } else {
	  dr0=dr;
	  }
	  dl=dest->left;
	  mr=max->right;
	  ml=max->left;
	  
	   x1=dl+((dl-ml)*(dr0-dl))/(mr-ml);
	   x2=dr0-((mr-dr)*(dr0-dl))/(mr-ml);
	   
		r=&r0;
		
		 SetRect(r, x1, dest->top, x2, dest->top+sizelift);
		 FillRect(r, coloroff);
		 
		  SetRect(r, x1, dest->top, x2, dest->top+sizelift);
		  FillRect(r, coloron);
		  
		   SetRect(r, x1, dest->top, x2, dest->top+sizelift);
		   FillRect(r, coloroff);		
		   }
	*/	
	
	if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
		int y1,y2;
		
		int dt0,dt,db,mt,mb;
		
		db=dest->bottom;
		dt=dest->top;
		
		if ( (max->right-max->left) > (dest->right-dest->left) ) {
			dt0=dest->top+sizelift;
		} else {
			dt0=dt;
		}
		mb=max->bottom;
		mt=max->top;
		
		y1=dt0+((dt-mt)*(db-dt0))/(mb-mt);
		y2=db-((mb-db)*(db-dt))/(mb-mt);
		
		//	y2=y1+((db-dt)*(db-dt))/(mb-mt);
		
		if (y2>mb) {
			y2=mb;  // pas normal... mais ca arrive :(
		}
		
		r=&r0;
		
		if (dest->top<y1) {
			SetRect(r, dest->right - sizelift, dest->top, dest->right, y1);
			FillRect(r, coloroff);
        }		    
		
		SetRect(r, dest->right - sizelift, y1, dest->right, y2);
		FillRect(r, coloron);
		
		if (dest->bottom>y2) {
			SetRect(r, dest->right - sizelift, y2, dest->right, dest->bottom);
			FillRect(r, coloroff);  
		}
	}
	
	return;
}
#endif

// -1: dernier couleur 
// 0: vert
// 1: couleur
// 3: inactif

#define RGB15(R,G,B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3));

void SetPalette(int color)
{
    
    static int lastcolour=1;
    int i;
	
    if ( (color==0) || (color==1) ) {
        lastcolour=color;
        return;
    }    
	
    if (color==-1) {
        color=lastcolour;
    }
    
    if (color==1) {
        for ( i = 0; i < 32; i++ ) {
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;
			
            BG_PALETTE[i]=RGB15(r,g,b);
        }             
        lastcolour=color;
    }
	
    if (color==0) {        
		for ( i = 0; i < 32; i++ ) {
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;
			
			
			g=(r+g+b)/3;
			b=0;
			r=0;		
			
			BG_PALETTE[i]=RGB15(r,g,b);
		}             
		lastcolour=color;
	}
    if (color==3) {
        
		for ( i = 0; i < 32; i++ ) {
			int z;
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;
			
			z=(r+g+b)/3;
			
			BG_PALETTE[i]=RGB15(z,z,z);
		}             
	}
     
}

void RedefineKey(int key)
{
#ifndef USE_CONSOLE
    int x,y,n;
    dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

    keyEmul=3;

    while(((~IPC->buttons) & (1 << 6))==0);

    x=IPC->touchXpx;
    y=IPC->touchYpx;

    for (n=0;n<NBCPCKEY;n++) {
        if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
            keyown[key]=keymap[n].normal;
            break;
        }
    }
    UpdateKeyMenu();
#endif
}

void UpdateTitlePalette(struct kmenu *current)
{
/*
if (lastcolour==1) {
    sprintf(current->title,"Monitor: [COLOR] - Green");
} else {
    sprintf(current->title,"Monitor: Color - [GREEN]");
}
*/
}

// Retour: 1 -> return emulator
//         0 -> return to parent
//         2 -> return to item (switch)


int ExecuteMenu(int n, struct kmenu *current)
{
    switch(n) {
            case ID_SWITCH_MONITOR:
                return 0;
                break;
			case ID_COLOR_MONITOR:
				SetPalette(1);   
				return 0;
				break;
			case ID_GREEN_MONITOR:
				SetPalette(0);   
				return 0;
				break;
            case ID_SCREEN_AUTO:
                resize=1;
				DrawFct = TraceLigne8B512;
                Regs1=0; 
                Regs2=0; 
                Regs6=0;    
                Regs7=0;
                return 1;
                break;
			case ID_SCREEN_320:
                resize=2;
				DrawFct = TraceLigne8B512;
           //     BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
           //     BG3_CX = (XStart*4) << 8; 
				x0=(XStart*4);
                y0=36;
                maxy=64;
				return 0;
				break;
			case ID_SCREEN_NORESIZE:
                resize=3;
				DrawFct = TraceLigne8B512;
           //     BG3_XDX = 256; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
           //     BG3_CX = (XStart*4) << 8; 
				x0=(XStart*4);
                y0=40;
                maxy=80;
				return 0;
				break;
			case ID_SCREEN_OVERSCAN:
                resize=4;
				DrawFct = TraceLigne8B512;
             //   BG3_XDX = 384; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
             //   BG3_CX = 0;
				x0=(XStart*4);
                y0=0;
				return 0;
				break;
			case ID_KEY_KEYBOARD:
				keyEmul=2; //  Emul du clavier
				return 0;
				break;

			case ID_KEY_KEYPAD:
                keyown[0]=CPC_CURSOR_UP;
                keyown[1]=CPC_CURSOR_DOWN;
                keyown[2]=CPC_CURSOR_LEFT;
                keyown[3]=CPC_CURSOR_RIGHT;
                keyown[4]=CPC_RETURN;
                keyown[5]=CPC_SPACE;
                keyown[6]=CPC_SPACE;

				keyEmul=3; //  Emul du clavier fleche
				return 0;
				break;
			case ID_KEY_JOYSTICK:
                keyown[0]=CPC_JOY_UP;
                keyown[1]=CPC_JOY_DOWN;
                keyown[2]=CPC_JOY_LEFT;
                keyown[3]=CPC_JOY_RIGHT;
                keyown[4]=CPC_RETURN;
                keyown[5]=CPC_JOY_FIRE1;
                keyown[6]=CPC_JOY_FIRE2;

				keyEmul=3; //  Emul du joystick
				return 0;
				break;
			case ID_DISPFRAMERATE:
				dispframerate=1;
				return 0;
				break;
			case ID_NODISPFRAMERATE:
				dispframerate=0;
				return 0;
				break;
            case ID_RESET:
                myprintf("Reset CPC");
                ResetCPC();
                return 0;
                break;
		    case ID_SAVESNAP:
              {
                char *buf;
                char snap[256];

                sprintf(snap, "/%s_i.%d", currentfile, currentsnap+1);
                buf=strchr(snap,'.');
                *buf='_';
                SauveScreen(snap);

                sprintf(snap, "/%s.%d", currentfile, currentsnap+1);
                buf=strchr(snap,'.');
                *buf='_';
                SauveSnap(snap);

                return 1;
                break;
               }
            case ID_FILE:
               // LireRom(current,1);
                return 1;
                break;
            case ID_DISK:
                //LireRom(current,0);
                return 1;
                break;
            case ID_REDEFINE_UP:
                RedefineKey(0);
                return 2;
                break;
            case ID_REDEFINE_DOWN:
                RedefineKey(1);
                return 2;
                break;
            case ID_REDEFINE_LEFT:
                RedefineKey(2);
                return 2;
                break;
            case ID_REDEFINE_RIGHT:
                RedefineKey(3);
                return 2;
                break;
            case ID_REDEFINE_START:
                RedefineKey(4);
                return 2;
                break;
            case ID_REDEFINE_A:
                RedefineKey(5);
                return 2;
                break;
            case ID_REDEFINE_B:
                RedefineKey(6);
                return 2;
                break;
            case ID_REDEFINE_X:
                RedefineKey(7);
                return 2;
                break;
            case ID_REDEFINE_Y:
                RedefineKey(8);
                return 2;
                break;
            case ID_REDEFINE_L:
                RedefineKey(9);
                return 2;
                break;
            case ID_REDEFINE_R:
                RedefineKey(0);
                return 2;
                break;
            case ID_HACK_TABCOUL:
                hack_tabcoul =  (hack_tabcoul==1) ? 0:1;
                return 2;
                break;
            case ID_ACTIVE_MAGNUM:
                usemagnum=1;
                break;
			default:
                break;
            }
            return 1;
}




u16 MyReadKey(void)
{
    /*
     u16 keys_pressed, my_keys_pressed;
	
    do {
		keys_pressed = ~(REG_KEYINPUT);
    } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))==0);
	
    my_keys_pressed = keys_pressed;
	
    do {
		keys_pressed = ~(REG_KEYINPUT);
    } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);
	*/
    return 0; // my_keys_pressed;
}

void InitCalcPoints( void )
{
    int a, b, c, d, i;
    
    // Pour le mode 0
    for ( i = 0; i < 256; i++ )
	{
        a = ( i >> 7 )
			+ ( ( i & 0x20 ) >> 3 )
			+ ( ( i & 0x08 ) >> 2 )
			+ ( ( i & 0x02 ) << 2 );
        b = ( ( i & 0x40 ) >> 6 )
			+ ( ( i & 0x10 ) >> 2 )
			+ ( ( i & 0x04 ) >> 1 )
			+ ( ( i & 0x01 ) << 3 );
        TabPointsDef[ 0 ][ i ][ 0 ] = (u8)a;
        TabPointsDef[ 0 ][ i ][ 1 ] = (u8)a;
        TabPointsDef[ 0 ][ i ][ 2 ] = (u8)b;
        TabPointsDef[ 0 ][ i ][ 3 ] = (u8)b;
	}
	
    // Pour le mode 1
    for ( i = 0; i < 256; i++ )
	{
        a = ( i >> 7 ) + ( ( i & 0x08 ) >> 2 );
        b = ( ( i & 0x40 ) >> 6 ) + ( ( i & 0x04 ) >> 1 );
        c = ( ( i & 0x20 ) >> 5 ) + ( i & 0x02 );
        d = ( ( i & 0x10 ) >> 4 ) + ( ( i & 0x01 ) << 1 );
        TabPointsDef[ 1 ][ i ][ 0 ] = (u8)a;
        TabPointsDef[ 1 ][ i ][ 1 ] = (u8)b;
        TabPointsDef[ 1 ][ i ][ 2 ] = (u8)c;
        TabPointsDef[ 1 ][ i ][ 3 ] = (u8)d;
	}
	
    // Pour le mode 2
    for ( i = 0; i < 256; i++ )
	{
        TabPointsDef[ 2 ][ i ][ 0 ] = i >> 7;
        TabPointsDef[ 2 ][ i ][ 1 ] = ( i & 0x20 ) >> 5;
        TabPointsDef[ 2 ][ i ][ 2 ] = ( i & 0x08 ) >> 3;
        TabPointsDef[ 2 ][ i ][ 3 ] = ( i & 0x02 ) >> 1;
	}
	
    // Mode 3 = Mode 0 ???
    for ( i = 0; i < 256; i++ )
        for ( a = 0; a < 4; a++ )
            TabPointsDef[ 3 ][ i ][ a ] = TabPointsDef[ 0 ][ i ][ a ];
}

void CalcPoints( void )
{
    int i,j;

    if ((lastMode>=0) && (lastMode<=3)) {
        for (i=0;i<256;i++) {
            for(j=0;j<4;j++) {
                TabPoints[lastMode][i][j] = BG_PALETTE[TabCoul[ TabPointsDef[lastMode][i][j]]];
            }
            /* *(u32*)(&TabPoints[lastMode][i][0]) = (TabCoul[ TabPointsDef[lastMode][i][0] ] << 0) + (TabCoul[ TabPointsDef[lastMode][i][1] ] << 8) + (TabCoul[ TabPointsDef[lastMode][i][2] ] << 16) + (TabCoul[ TabPointsDef[lastMode][i][3] ] << 24);
             */
        }
    }
    UpdateInk=0;
}




/********************************************************* !NAME! **************
* Nom : InitPlateforme
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation diverses
*
* Résultat    : /
*
* Variables globales modifiées : JoyKey, clav
*
********************************************************** !0! ****************/
void InitPlateforme( unsigned short *screen, u16 screen_width )
{
    myprintf("InitPlateform");
	
    MemBitmap=screen;
    MemBitmap_width=screen_width;
    
	InitCalcPoints();
	CalcPoints();
	memset( clav, 0xFF, sizeof( clav ) );
	memset( cpckeypressed, 0, sizeof(cpckeypressed));
    
    ipc.keys_pressed=0;
	
	inMenu=0;
    isPaused=0;
}


/********************************************************* !NAME! **************
* Nom : Erreur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'erreur
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Erreur( char * Msg )
{
	myprintf("Error: %s", Msg);
}


/********************************************************* !NAME! **************
* Nom : Info
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'information
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Info( char * Msg )
{
	myprintf("Info: %s", Msg);
}

//Bordure: 32 a gauche, 32 a droite
//Border: 36 en haut, 36 en bas

static int updated=0;


void TraceLigne8B512( int y, signed int AdrLo, int AdrHi )
{
    // if (y>yMax) yMax=y;
    // if (y<yMin) yMin=y;

    y-=y0;

    if ((y<0) || (y>=272)) { // A verifier (200?)
        return;
    }

    if ((!hack_tabcoul) && (UpdateInk==1)) {    // It's would be beter to put before each lines
        CalcPoints();
    } 

	updated=1;
	
	u16 *p;
	
	p = (u16*)MemBitmap;
    p += (y*MemBitmap_width);
	
	if ( AdrLo < 0 ) {
        if (resize!=1) {
        int x;
        for (x=0;x<384;x++) {
            p[x]=BG_PALETTE[TabCoul[ 16 ]];
        }
        }
	} else {
        int x;
        
        if (resize!=1) {
        for(x=0;x<XStart*4;x++) {
            *p = BG_PALETTE[TabCoul[ 16 ]];
            p++;
        }
        } else {
           // p+=XStart*4;
        }
        
		for (x = XStart; x < XEnd; x++ ) { 
            u16 *tab = &(TabPoints[ lastMode ][ MemCPC[ ( AdrLo & 0x7FF ) | AdrHi ] ][0]);
            memcpy(p,tab,4*sizeof(u16));
            p+=4;
            AdrLo++;
		}
        
        if (resize!=1) {
        for(x=0;x<(96-XEnd)*4;x++) {
            *p = BG_PALETTE[TabCoul[ 16 ]];
            p++;
        }
        }

	}

}

/********************************************************* !NAME! **************
* Nom : UpdateScreen
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affiche l'écran du CPC
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void UpdateScreen( void )
{
    
    frame++;

    if (resize==1) {       // Auto resize ?
        if ((RegsCRTC[2]!=Regs2) || (RegsCRTC[6]!=Regs6) || (RegsCRTC[1]!=Regs1) || (RegsCRTC[7]!=Regs7)) {
            int x1,x2, y1,y2;
            int height;

            x1 = max( ( 50 - RegsCRTC[2] ) << 3, 0 );
            x2 = min( x1 + ( RegsCRTC[1] << 3 ), 384 );

            // y1 = max( (272-(RegsCRTC[6]<<3))>>1, 0);
            y1 = max( ( 35 - RegsCRTC[7] ) << 3, 0);
            y2 = min( y1 + (RegsCRTC[6] << 3), 272);

		    DrawFct = TraceLigne8B512;
     //       BG3_XDX = (x2-x1); 
       //     BG3_CX = x1 << 8; 

            height=(y2-y1);
            if (height<192) height=192;

            
            x0=x1;
            y0=y1;   // Redbug
            maxy=0;

            (*borderX)=(384-(x2-x1))/2;
            (*borderY)=(272-(y2-y1))/2;            

            printf("*** Resize to %dx%d\n", x2-x1, y2-y1); // FM->title
            printf("*** Border to %dx%d\n",  (*borderX),  (*borderY)); // FM->title

            Regs1=RegsCRTC[1];
            Regs2=RegsCRTC[2];
            Regs6=RegsCRTC[6];
            Regs7=RegsCRTC[7];
        }
    }
    

    if (msgframe>frame-50*3) {
        int alpha;
        alpha=(msgframe-(frame-50*3))*4;
        if (alpha>255) alpha=255;

        cpcprint16i(0,40, msgbuf, alpha);
    }

    if (updated) {	
		// dmaCopy(MemBitmap, frontBuffer, CPC_VISIBLE_SCR_WIDTH * CPC_VISIBLE_SCR_HEIGHT);
		//
        updated=0;

        if (UpdateInk==1) {    // It's would be beter to put before each lines
            CalcPoints();
        } 
    }
}



int shifted=0;
int ctrled=0;
int copyed=0;


void Dispkey(CPC_KEY n, int status);
void DispScanCode(CPC_SCANCODE n, int status);
void PressKey(CPC_KEY n);

void PressKey(CPC_KEY n)
{
	CPC_SCANCODE cpc_scancode;
	cpc_scancode=keymap[n].normal;

	Dispkey(n, 1);
	
	if (shifted) {
		DispScanCode(CPC_SHIFT, 0 | 16);
		shifted=0;
        clav[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
	}
	if (ctrled) {
		DispScanCode(CPC_CONTROL, 0);
		ctrled=0;
		clav[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
	}
	if (copyed) {
		DispScanCode(CPC_COPY, 0);
		copyed=0;
	}
	
	clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];
	
	switch(cpc_scancode) {
	case CPC_SHIFT:
		if (shifted) {
			DispScanCode(cpc_scancode, 0 | 16);
			shifted=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			shifted=1;
		}
		break;
	case CPC_CONTROL:
		if (ctrled) {
			DispScanCode(cpc_scancode, 0 | 16);
			ctrled=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			ctrled=1;
		}
		break;
	case CPC_COPY:
		if (copyed) {
			DispScanCode(cpc_scancode, 0 | 16);
			copyed=0;
		} else {
			DispScanCode(cpc_scancode, 1 | 16);
			copyed=1;
		}
		break;
	default:
		break;
	}
}

void CPC_SetScanCode(CPC_SCANCODE cpc_scancode)
{
	clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];
    DispScanCode(cpc_scancode, 1);
}

void CPC_ClearScanCode(CPC_SCANCODE cpc_scancode)
{
	clav[(u8)cpc_scancode >> 3] |= bit_values[(u8)cpc_scancode & 7];
    DispScanCode(cpc_scancode, 0);
}


void DispScanCode(CPC_SCANCODE scancode, int status)
{
    int n;
    
    for(n=0;n<NBCPCKEY;n++) {
        if (keymap[n].normal == scancode)  {
            Dispkey(n, status);
        }
    }    
}

// 1: active
// 2: on
// 0: off

void Dispkey(CPC_KEY n, int status)
{
#ifndef USE_CONSOLE    
	int x,y;
	u16 color;

	
	if ((status&16)!=16) {
		if ((keymap[n].normal==CPC_SHIFT) || (keymap[n].normal==CPC_CONTROL) || (keymap[n].normal==CPC_COPY)) {
			return;
		}
	} 
	
	switch(status) {
	case 0:
	case 16:
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=kbdBuffer[x+y*256];
			}
		}
		break;
	case 17:		
	case 1:
		color=RGB15(15,0,0);
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
			}
		}
        cpckeypressed[n]=2;
		break;
	case 2:
	case 18:
		color=RGB15(0,15,0);
		for(y=keypos[n].top;y<keypos[n].bottom;y++) {
			for(x=keypos[n].left;x<keypos[n].right;x++) {
				backBuffer[x+y*256]=~kbdBuffer[x+y*256]|0x8000;
				// backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
			}
		}
		break;
	}
#endif
}

void DispDisk(int reading)
{
#ifndef USE_CONSOLE
    int x,y;
    RECT r;
    u16 color;
    
    // SetRect(&r, 222,88,254,113);
    SetRect(&r, 230,1,254,33);
    
	switch(reading) {
	case 0:
		for(y=r.top;y<r.bottom;y++) {
			for(x=r.left;x<r.right;x++) {
				backBuffer[x+y*256]=kbdBuffer[x+y*256];
			}
		}
		break;
	case 1:
		color=RGB15(15,0,0);
		for(y=r.top;y<r.bottom;y++) {
			for(x=r.left;x<r.right;x++) {
				// backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
				backBuffer[x+y*256]=~kbdBuffer[x+y*256]|0x8000;
			}
		}
		break;
    }   
#endif
}

int nds_ReadKey(void)
{
	if (AutoType_Active()) {
		AutoType_Update();
	} else {
		u16 keys_pressed;
		static u16 oldkey;
		int n;

 //       scanKeys();
   //     keys_pressed = keysHeld();
        
        keys_pressed = ipc.keys_pressed;
		
		memset(clav,0xFF,16);
		
		if (ipc.touchDown==1) {
			int x,y,n;
			
			x=ipc.touchXpx;
			y=ipc.touchYpx;
            
           /* if ((x>0) & (x<32) & (y>=25) & (y<=36)) {
                ExecuteMenu(ID_RESET, NULL);
                ipc.touchDown=0;
            }
            */

           if ( (x>=230) && (x<=254) && (y>=1) && (y<=33) ) { // 52
  				inMenu=1;
			}			
			
			for (n=0;n<NBCPCKEY;n++) {
				if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
					PressKey(n);
					break;
				}
			} 
        }	
        
        if (keyEmul==3) {
            if ((keys_pressed & KEY_UP)==KEY_UP)
				CPC_SetScanCode(keyown[0]);
            
            if ((keys_pressed & KEY_DOWN)==KEY_DOWN)
				CPC_SetScanCode(keyown[1]);
            
            if ((keys_pressed & KEY_LEFT)==KEY_LEFT)
				CPC_SetScanCode(keyown[2]);
            
            if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT)
				CPC_SetScanCode(keyown[3]);
            
            if ((keys_pressed & KEY_START)==KEY_START)
				CPC_SetScanCode(keyown[4]);
            
            if ((keys_pressed & KEY_A)==KEY_A) 
				CPC_SetScanCode(keyown[5]);
            
            if ((keys_pressed & KEY_B)==KEY_B) 
				CPC_SetScanCode(keyown[6]);
            
            if ((keys_pressed & KEY_X)==KEY_X) 
				CPC_SetScanCode(keyown[7]);
            
            if ((keys_pressed & KEY_Y)==KEY_Y) 
				CPC_SetScanCode(keyown[8]);
        }
    
		for(n=0;n<NBCPCKEY;n++) {
            if (cpckeypressed[n]!=0) {
			    cpckeypressed[n]--;
			    if(cpckeypressed[n]==0) {
			    	Dispkey(n, 0);
			    }
            }
		}	        
		
		oldkey = keys_pressed;
	}
	
	return 0;
}

void videoinit(void)
{
}

void nds_initBorder(int *_borderX, int *_borderY)
{
    borderX=_borderX;
        borderY=_borderY;
}

void nds_init(void)
{
    /*
    powerON(POWER_ALL_2D);
	
    irqInit();                    // IRQ basic setup
    irqSet(IRQ_VBLANK, 0);
	
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    vramSetBankA(VRAM_A_MAIN_BG_0x6000000);
    vramSetBankB(VRAM_B_MAIN_BG_0x6020000); // Pourquoi faire ?
    vramSetBankC(VRAM_C_SUB_BG_0x6200000);
    vramSetBankD(VRAM_D_LCD);
	
    BG3_CR = BG_BMP8_512x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
    BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
    BG3_XDY = 0;
    BG3_YDX = 0;
    BG3_YDY = 256; // Taille d'affichage360 - 108; // 360 = DISPLAY_X
    BG3_CX = 0<<8;
    BG3_CY = 0<<8; // 32<<8;
    frontBuffer = (u8*)BG_BMP_RAM(0);
    
    MemBitmap = frontBuffer;
    // MemBitmap=MyAlloc(256*(192+1),"MemBitmap"); // (192+1) for overflow
    
#ifndef USE_CONSOLE
    SUB_BG3_CR = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY(3);
    SUB_BG3_XDX = 256;
    SUB_BG3_XDY = 0;
    SUB_BG3_YDX = 0;
    SUB_BG3_YDY = 256;
    SUB_BG3_CX = 0;
    SUB_BG3_CY = 0;
    backBuffer=(u16*)BG_BMP_RAM_SUB(0);
	
    SUB_BG0_CR = BG_256_COLOR | BG_TILE_BASE(0) | BG_MAP_BASE(20) | BG_PRIORITY(0);
#else
    videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
    vramSetBankC(VRAM_C_SUB_BG);
    SUB_BG0_CR = BG_MAP_BASE(31);
    BG_PALETTE_SUB[255] = RGB15(31,31,31);
    consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);
#endif
	
	
	TIMER0_DATA=0;
	TIMER1_DATA=0;
	TIMER0_CR=TIMER_DIV_1024;
	TIMER1_CR=TIMER_CASCADE;
	
	// Clear the FIFO queue
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	
	IPC2->psgbuf=(u8*)calloc(SOUNDBUFCNT*3, 0);
	FifoOut(3,0,0);		// Init buffer
	FifoOut(2,0,0);		// Play sound	
	
#ifndef USE_CONSOLE    
	// Chargement du clavier   
	
	filefont = CreateFont((u8*)filefont_gif, filefont_gif_size);
	font = CreateFont((u8*)smallfont_gif, smallfont_gif_size);
	fontred = CreateFont((u8*)smallfont_red_gif, smallfont_red_gif_size);
	
    menubufferlow=(u16*)malloc(256*192*2);
	ReadBackgroundGif(menubufferlow, (u8*)background_menu_gif, background_menu_gif_size);    

	ReadBackgroundGif8(MemBitmap, (u8*)crocods_gif, crocods_gif_size);    
    dmaCopy(MemBitmap, frontBuffer, 256*192);
	
#ifndef BEEK
    kbdBuffer = (u16*)MyAlloc(SCREEN_WIDTH*SCREEN_HEIGHT*2, "Keyboard"); 
	ReadBackgroundGif(kbdBuffer, (u8*)background_gif, background_gif_size);    
#else
	sImage pcx;       
	
	loadPCX((u8*)keyboard_pcx, &pcx);
	image8to16(&pcx);     
    dmaCopy(pcx.data16, kbdBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
    imageDestroy(&pcx);
#endif
    
	// Affichage du clavier
	
    dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);
#endif
	
    myprintf("CrocoDS v0.5a");
	
    myprintf("Trying to init FAT...");
    FS_Init();

	
    myprintf("Creating menu...");
	Fmnbr=0;	
	
	struct kmenu *id;

    root.nbr=0;
    menuRomId=AddMenu(&root, "Autostart ROM", ID_MENU);
    menuDiskId=AddMenu(&root, "Load Disk", ID_MENU);
    FS_getFileList(addFile);
    
    id=AddMenu(&root, "Switch monitor", ID_MONITOR_MENU);
    AddMenu(id, "Color Monitor", ID_COLOR_MONITOR);
    AddMenu(id, "Green Monitor", ID_GREEN_MONITOR);
    
    id=AddMenu(&root, "Resize", ID_SCREEN_MENU);
    AddMenu(id, "Auto", ID_SCREEN_AUTO);
    AddMenu(id, "320x200", ID_SCREEN_320);
    AddMenu(id, "No resize", ID_SCREEN_NORESIZE);
    AddMenu(id, "Overscan", ID_SCREEN_OVERSCAN);
    
    id=AddMenu(&root, "Pad emulation", ID_KEY_MENU);
    AddMenu(id, "Keyboard Emulation", ID_KEY_KEYBOARD);
    AddMenu(id, "Set to Joystick", ID_KEY_JOYSTICK);
    AddMenu(id, "Set to Keypad", ID_KEY_KEYPAD);
    keyMenu=AddMenu(id, "Redefine Keys", ID_MENU);
    AddMenu(keyMenu, "Up: XXXXXXXXXX", ID_REDEFINE_UP);
    AddMenu(keyMenu, "Down: XXXXXXXXXX", ID_REDEFINE_DOWN);
    AddMenu(keyMenu, "Left: XXXXXXXXXX", ID_REDEFINE_LEFT);
    AddMenu(keyMenu, "Right: XXXXXXXXXX", ID_REDEFINE_RIGHT);
    AddMenu(keyMenu, "A: XXXXXXXXXX", ID_REDEFINE_A);
    AddMenu(keyMenu, "B: XXXXXXXXXX", ID_REDEFINE_B);
    AddMenu(keyMenu, "X: XXXXXXXXXX", ID_REDEFINE_X);
    AddMenu(keyMenu, "Y: XXXXXXXXXX", ID_REDEFINE_Y);
    AddMenu(keyMenu, "START: XXXXXXXXXX", ID_REDEFINE_START);
    // AddMenu(keyMenu, "L: XXXXXXXXXX", ID_REDEFINE_L);
    // AddMenu(keyMenu, "R: XXXXXXXXXX", ID_REDEFINE_R);
    
    id=AddMenu(&root, "Debug", ID_MENU);
    AddMenu(id, "Display framerate", ID_DISPFRAMERATE);
    AddMenu(id, "Don't display framerate", ID_NODISPFRAMERATE);

    id=AddMenu(&root,"Hack", ID_MENU);
    AddMenu(id, "Only one ink refresh per frame: N", ID_HACK_TABCOUL);

    AddMenu(&root, "Reset CPC", ID_RESET);

#ifdef USE_SAVESNAP
    AddMenu(&root, "Save State", ID_SAVESNAP);
#endif
	*/
    
    ExecuteMenu(ID_COLOR_MONITOR, NULL);
   ExecuteMenu(ID_SCREEN_AUTO, NULL);
    
   // ExecuteMenu(ID_SCREEN_OVERSCAN, NULL);
    ExecuteMenu(ID_KEY_JOYSTICK, NULL);
    ExecuteMenu(ID_NODISPFRAMERATE, NULL);
	ExecuteMenu(ID_HACK_TABCOUL, NULL);
    /*
    UpdateKeyMenu();
	*/
    
#ifdef USE_Z80_ORIG 
    ExecInstZ80 = ExecInstZ80_orig;
    ResetZ80 = ResetZ80_orig;
    SetIRQZ80 = SetIRQZ80_orig;
#endif
	
#ifdef USE_Z80_ASM
    ExecInstZ80= ExecInstZ80_asm;
    ResetZ80 = ResetZ80_asm;
    SetIRQZ80 = SetIRQZ80_asm;
#endif

#ifdef USE_Z80_ASM2
    ExecInstZ80= ExecInstZ80_asm2;
    ResetZ80 = ResetZ80_asm2;
    SetIRQZ80 = SetIRQZ80_asm2;
#endif

    strcpy(currentfile,"nofile");	
}

void Autoexec(void)
{
    if (Fmnbr==0) {
        SetPalette(-1);
        return;
    }
    if (Fmnbr==1) {
     //   LireRom(FirstROM,1);
    }
}

int nds_video_unlock(void)
{
	return 1; // OK
}

int nds_video_lock(void)
{
	return 1; // OK
}

void nds_video_close(void)
{
}

inline void printDebug(const char *fmt, ...)
{
#ifdef USE_DEBUG
    char s[512];
	
    va_list args;
	
    va_start(args, fmt);
    vsprintf(s,fmt,args);
    va_end(args);
	
    myprintf(s);
	
	/*
    asm volatile("mov r0, %0;"
	"swi 0xff;"
	: // no ouput
	: "r" (s)
	: "r0");
	*/                 
#endif
}



void myconsoleClear(void)
{
    memset(consolestring,0,1024);
    consolepos=0;
}


void myprintf0(const char *fmt, ...) 
{
    char tmp[512];
	
    va_list args;
	
    va_start(args, fmt);
    vsprintf(tmp,fmt,args);
    va_end(args);
	
    if (tmp[0]=='\n') {
		consolepos++;
		if (consolepos==8) {
			memcpy(consolestring,consolestring+128,1024-128);
			consolepos=7;
		}
    }
	
    memcpy(consolestring+consolepos*128, tmp, 128);
    consolestring[consolepos*128-1]=0;
}

void myprintf(const char *fmt, ...) 
{
    char tmp[512];
    int n;
	
    va_list args;
	
    va_start(args, fmt);
    vsprintf(tmp,fmt,args);
    va_end(args);


    strncpy(msgbuf, tmp, 32);
    msgbuf[32]=0;
    msgframe=frame;
    for(n=strlen(msgbuf);n<32;n++) {
        msgbuf[n]=' ';
    }
    
	
#ifdef USE_CONSOLE
    printf("%s\n", tmp);
#else

    if (backBuffer==NULL) {
        return;
    } else {
#ifdef USE_ALTERSCREEN
		int n;
		int width;
		int x,y;
		
		memcpy(consolestring+consolepos*128, tmp, 128);
		consolestring[consolepos*128-1]=0;

        if (!inMenu) {
		for(n=0;n<8;n++) {
			if (n<=consolepos) {
				if (consolestring[n*128]==1) {
					width = DrawText(backBuffer, fontred, 110, 5+n*10, consolestring+n*128+1);
				} else {
					width = DrawText(backBuffer, font, 110, n*10+5, consolestring+n*128);
				}
			} else {
				width = 0;
			}
			for(y=5+n*10;y<5+(n+1)*10;y++) {
				for(x=110+width;x<253;x++) {
					backBuffer[x+y*256]=RGB15((156>>3), (178>>3), (165>>3))|0x8000;  // 156, 178, 165
				}
			}
		}
        }
#endif
		
		consolepos++;
		if (consolepos==8) {
			memcpy(consolestring,consolestring+128,1024-128);
			consolepos=7;
		}
		// for(n=0;n<20;n++) swiWaitForVBlank();
    }
#endif
}

void ResetCPC(void)
{
	Keyboard_Reset();
	WriteVGA( 0x89 );
	ResetZ80(); 
    ResetCRTC();

    Reset8912();
}

void cpcprint(int x, int y, char *pchStr, u8 bColor)
{
	int iLen, iIdx, iRow, iCol;
	u8 bRow;
	u8 *pdwAddr;
	int n;
	
	pdwAddr = (u8*)MemBitmap + (y*256) + x;   
	
	iLen = strlen(pchStr); // number of characters to process
	for (n = 0; n < iLen; n++) {
		u8 *pdwLine;
		iIdx = (int)pchStr[n]; // get the ASCII value
		if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
			iIdx = FNT_BAD_CHAR;
		}
		iIdx -= FNT_MIN_CHAR; // zero base the index
		pdwLine = pdwAddr; // keep a reference to the current screen position
		for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
            u32 *pdPixel;

            pdPixel = (u32 *)pdwLine;
			bRow = bFont[iIdx]; // get the bitmap information for one row
			for (iCol = 0; iCol < 2; iCol++) { // loop for all columns in the font character
                *pdPixel = ((bRow & 0x80) ? (bColor<<0) : 0) +
                           ((bRow & 0x40) ? (bColor<<8) : 0) +
                           ((bRow & 0x20) ? (bColor<<16) : 0) +
                           ((bRow & 0x10) ? (bColor<<24) : 0);

				pdPixel++; // update the screen position
				bRow <<= 4; // advance to the next bit
			}
			pdwLine += 256;
			iIdx += FNT_CHARS; // advance to next row in font data
		}
		pdwAddr += FNT_CHAR_WIDTH; // set screen address to next character position
	}
}

void cpcprint16(int x, int y, char *pchStr, u16 bColor)
{
//    NSLog(@"cpcprint: %d", pchStr);
}

void cpcprint16i(int x, int y, char *pchStr, int alpha)
{
}



u8 *MyAlloc(int size, char *title)
{
	u8 *mem;
	mem=(u8*)malloc(size);
	if (mem==NULL) {
		myprintf("Allocate %s (%d): FAILED", title, size);
		// while(1) swiWaitForVBlank();
	} else {
		//    myprintf("Allocate %s (%d): OK", title, size);
	}
	return mem;
}

