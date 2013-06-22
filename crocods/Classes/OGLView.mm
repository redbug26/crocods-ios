#include "OGLView.h"
#include <QuartzCore/QuartzCore.h>

#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>
#include <OpenGLES/ES1/glext.h>

#include <string.h>
#include "CrocoDSViewController.h"
#include "CrocoDSAppDelegate.h"

#include "nds.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

#include "snapshot.h"

#include "types.h"
#include "plateform.h"
#include "vga.h"
#include "ppi.h"
#include "upd.h"
#include "autotype.h"
#include "sound.h"
#include "config.h"
#include "crtc.h"
#include "snapshot.h"
#include "microlib.h"

#include "z80.h"

#include <sys/time.h>

#define pperday 10
#define PI 3.1415926535
#define CX ((320-(pperday*31))/2)
#define SIZEX 320
#define SIZEY 460

@implementation OGLView

@synthesize parent;

long nds_GetTicks(void);

// Temps en millisecondes.

long nds_GetTicks(void) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    
    return ((unsigned long long)current_time.tv_sec * 1000000LL + current_time.tv_usec);
}

- (void)GoSound
{
    sound_open(SOUND_FREQ,8,1);
    Loop_Sound(SOUND_FREQ);
}


- (void)Go
{
    @autoreleasepool {
    
        [NSThread setThreadPriority:1.0];
        
        unsigned long TimeOut = 0, OldTime = 0;
        long tz80=0;
        char framebuf[128];
        int oldDriveBusy=0;
        int turbo=0;
        
        framebuf[0]=0;
        
        
        do
	{
            tz80-=nds_GetTicks();
            ExecInstZ80();    // Fais tourner le CPU tant CptInstr < CYCLELIGNE
            tz80+=nds_GetTicks();
            
            TimeOut += (RegsCRTC[0] + 1);
		
		if (!CalcCRTCLine()) {         // Arrive en fin d'ecran ?
			// Rafraichissement de l'ecran...
                
                if (dispframerate) {
		        cpcprint16i(0,192-8, framebuf, 255);
                }
                
			UpdateScreen();
			nds_ReadKey();
			
			// Disk
			if (DriveBusy!=oldDriveBusy) {
			    DispDisk(DriveBusy);
			    oldDriveBusy=DriveBusy;
			}
			DriveBusy=0;
                
                
			// Synchronisation de l'image ‡ 50 Hz
			// Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne
                
                if (DoResync) {
                    int Time;
                    
                    self.borderX=bx;
                    self.borderY=by;
                    
                    if (dispframerate) {
                        int time;
                        time=(nds_GetTicks() * 1024 - OldTime);
                        
                        if (time!=0) {
                            sprintf(framebuf, " %4lu%% %4lu %4lu + %4ld           ", (u32)((TimeOut * 100) / time) , (u32)(TimeOut / 1024), (u32)(time/1024) - tz80, tz80);
                            NSLog(@"framebuf: %s", framebuf);
                        }
	     		    //sprintf(framebuf, " %08X %02X %02X %02X", IPC2->psgbuf, IPC2->psgbuf[0], IPC2->psgbuf[1], IPC2->psgbuf[2]);
                    }
                    
                    
                    if (!turbo) {
				    Time = nds_GetTicks();
				    while( Time - OldTime < TimeOut ) {
					    Time = nds_GetTicks();
                        }
                        
                    }
                    
		    	tz80=0;
                    
			    OldTime = nds_GetTicks();
                    TimeOut=0;
                    
                    while(isPaused);
                }
		}
	}
        while(1); //  ! finMain );
    
    
    }
    
}

/* CreateSurface */
- (int)CreateSurfaces:(id)i {
    iCadeReaderView *control = [[iCadeReaderView alloc] initWithFrame:CGRectZero];
    [self addSubview:control];
    control.active = YES;
    control.delegate = self;
    
    NSData *cpc6128=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"cpc6128" ofType:@"bin"]];
    NSData *romdisc=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"romdisc" ofType:@"bin"]];
    
    nds_initBorder(&bx, &by);
    nds_init();
	
	AutoType_Init();
	
    if ( ReadConfig() )
	{
		NSLog(@"Read config: OK");
		if ( InitMemCPC((char*)[cpc6128 bytes], (char*)[romdisc bytes]) )
		{
			NSLog(@"Init Memory CPC: OK");
			
            ResetZ80();
            ResetUPD();
            //	ResetCRTC();
            InitPlateforme(pSurface, width0);
            
            NSLog(@"Begin emulation");
            
            Autoexec();
            
            [self performSelectorInBackground:@selector(Go) withObject:nil];
            [self performSelectorInBackground:@selector(GoSound) withObject:nil];
            
            //           EjectDiskUPD();
            // PauseSound();
			
		}
		else
		{
			NSLog(@"Roms du CPC non trouvÈes");
		}
	}
    
    else {
        NSLog(@ "Fichier de configuration du CPC non trouvÈ." );
    }
    
    
    return 0;
}


- (void)dealloc
{
    sound_close();
    
}




-(NSInteger)supportedInterfaceOrientations:(UIWindow *)window{
    
    NSLog(@"window: %@", window);
    
    return 24;
}

- (BOOL)shouldAutorotate {
    NSLog(@"shouldAutorotate");
    
    return true;
}

// --- Key

- (void)characterDown:(char)ch {
    if ([crocodsAppDelegate delegate].useExternalKeyboard) {
        char string[2];
        string[0]=ch;
        string[1]=0;
        AutoType_SetString(string, false);
    }
}

- (void)buttonDown:(iCadeState)button {
    
    if (([crocodsAppDelegate delegate].isPro) & ([crocodsAppDelegate delegate].useIcade)) {
        
        switch (button) {
            case iCadeButtonA:
                ipc.keys_pressed |= KEY_A;
                break;
            case iCadeButtonB:
                ipc.keys_pressed |= KEY_B;
                break;
            case iCadeJoystickUp:
                ipc.keys_pressed |= KEY_UP;
                break;
            case iCadeJoystickRight:
                ipc.keys_pressed |= KEY_RIGHT;
                break;
            case iCadeJoystickDown:
                ipc.keys_pressed |= KEY_DOWN;
                break;
            case iCadeJoystickLeft:
                ipc.keys_pressed |= KEY_LEFT;
                break;
            default:
                break;
                
        }
    }
    
    if ([crocodsAppDelegate delegate].useExternalKeyboard) {
        //        AutoType_SetString(@"", false);
        
    }
    
}

- (void)buttonUp:(iCadeState)button {
    
    if (([crocodsAppDelegate delegate].isPro) & ([crocodsAppDelegate delegate].useIcade)) {
        switch (button) {
            case iCadeButtonA:
                ipc.keys_pressed &= (~KEY_A);
                break;
            case iCadeButtonB:
                ipc.keys_pressed &= (~KEY_B);
                break;
            case iCadeJoystickUp:
                ipc.keys_pressed &= (~KEY_UP);
                break;
            case iCadeJoystickRight:
                ipc.keys_pressed &= (~KEY_RIGHT);
                break;
            case iCadeJoystickDown:
                ipc.keys_pressed &= (~KEY_DOWN);
                break;
            case iCadeJoystickLeft:
                ipc.keys_pressed &= (~KEY_LEFT);
                break;
            default:
                break;
        }
    }
    
    if ([crocodsAppDelegate delegate].useExternalKeyboard) {
        
    }
    
}






@end
