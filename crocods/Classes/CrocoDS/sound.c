#include "nds.h"
#include "types.h"

#include "sound.h"
#include "plateform.h"
#include "microlib.h"

#include "emu2149.h"


EMU2149_API PSG *_PSG_new (u32 c, u32 r);
void InitSound(int r);
void ProcSound(void);

int SoundPause = 1;

PSG psg;

#define CPC_CLK 2000000
#define SOUNDBUFCNT ((u16)(1024))   //367

#define TIMERCOUNT(N)  ((int)( 0x2000000 / (N) ))

s16 * soundbuf;
s16 * sbuf;
int wptr  = 0; // ÉfÅ[É^èëçûà íu
int wbufp = 0; // ÉfÅ[É^èëçûÉoÉbÉtÉ@
int SoundProcTiming = 0;


u8 PlaySound( void )
{
	return 1;
}

void PauseSound( void )
{
}


void Reset8912( void )
{
     psg.control = 0;
}

void Write8912( u32 reg, u32 val )            
{
	if (reg<0x20) {
        psg.reg[reg] = (u8) (val & 0xff);
        
        int c;
        
        if (reg > 15) return;
        
        psg.reg[reg] = (u8) (val & 0xff);
        switch (reg) {
            case 0:
            case 2:
            case 4:
            case 1:
            case 3:
            case 5:
                c = reg >> 1;
                psg.freq[c] = ((psg.reg[c * 2 + 1] & 15) << 8) + psg.reg[c * 2];
                break;
            case 6:
                psg.noise_freq = (val == 0) ? 1 : ((val & 31) << 1);
                break;
            case 7:
                psg.tmask[0] = (val & 1);
                psg.tmask[1] = (val & 2);
                psg.tmask[2] = (val & 4);
                psg.nmask[0] = (val & 8);
                psg.nmask[1] = (val & 16);
                psg.nmask[2] = (val & 32);
                break;
            case 8:
            case 9:
            case 10:
                psg.volume[reg - 8] = val << 1;
                break;
            case 11:
            case 12:
                psg.env_freq = (psg.reg[12] << 8) + psg.reg[11];
                break;
            case 13:
                psg.env_continue = (val >> 3) & 1;
                psg.env_attack = (val >> 2) & 1;
                psg.env_alternate = (val >> 1) & 1;
                psg.env_hold = val & 1;
                psg.env_face = psg.env_attack;
                psg.env_pause = 0;
                psg.env_count = 0x10000 - psg.env_freq;
                psg.env_ptr = psg.env_face?0:0x1f;
                break;
            case 14:
            case 15:
            default:
                break;
        }
        

  	}

  return;
}

int Read8912( int r )
{
    return (u8) (psg.reg[r & 0x1f]);
}


//////////////////////////////////////////////////////////////////////
EMU2149_API PSG *_PSG_new (u32 c, u32 r) {
    memset(&psg, 0, sizeof (PSG));
    
    PSG_setVolumeMode (&psg, EMU2149_VOL_DEFAULT);
    psg.clk = c;
    psg.rate = r ? r : 44100;
    PSG_set_quality (&psg, 0);
    
    return &psg;
}

void InitSound(int r) {
    
	soundbuf = (s16*)malloc(SOUNDBUFCNT*sizeof(s16));	
    sbuf = soundbuf;
    
	// PSG initialize
	_PSG_new(CPC_CLK,r);
	PSG_reset( &psg );
}



void ProcSound(void) {
	signed short P;
    
	P = PSG_calc(&psg)<<1;
	*sbuf = P; 
    
	sbuf ++;
	wptr ++ ;
	if( wptr >= SOUNDBUFCNT ){
        sound_send(soundbuf, SOUNDBUFCNT);
        sbuf = soundbuf;
		wptr = 0;
	}
}


void Loop_Sound(int samplerate)
{
    InitSound(samplerate);
    
	while (1) {
        if (!isPaused) ProcSound();
    }
	return;
}







