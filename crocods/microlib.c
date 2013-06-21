/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 
 Copyright (c) 2010 Seleuco.
 
 */

#include "microlib.h"
#include <stdlib.h>
#include <stdio.h>

//#include "IPhone/helpers.h"

#import <AudioToolbox/AudioQueue.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>
#import <pthread.h>



/* Audio Resources */
#define AUDIO_BUFFERS 2

typedef struct AQCallbackStruct {
    AudioQueueRef queue;
    UInt32 frameCount;
    AudioQueueBufferRef mBuffers[AUDIO_BUFFERS];
    AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

AQCallbackStruct in;

int soundInit = 0;

float __audioVolume = 1.0;

int audioBufferSize = 0;

#define BUF (audioBufferSize )

unsigned long oldtime2 = 0;

//#define QSIZE (882 * 2)
#define TAM (4096)
#define TAMB (882 * 2* 50 * 10)

signed char data[TAMB * 2];

long i1=0;
long i2=0;

unsigned char ptr_buf[TAM];

unsigned head = 0;
unsigned tail = 0;
int stereof = 0;

pthread_mutex_t mut;



//SOUND

int fullQueue(unsigned short size);
int emptyQueue(void);


int fullQueue(unsigned short size){
    
    if(head < tail)
	{
		return head + size >= tail;
	}
	else if(head > tail)
	{
		return (head + size) >= TAM ? (head + size)- TAM >= tail : false;
	}
	else return false;
}

int emptyQueue(void){
	return head == tail;
}

void queue(unsigned char *p,unsigned size)
{
    
    //printf("-->meten %d h: %d t: %d TAM %d\n",size,head,tail,TAM);
    
	//while(1){
    //printf("Hago lock en queue\n");
    
    //printf("Fin Hago lock en queue\n");
    if(fullQueue(size))
    {
        //printf("---> LLENA\n");
        //printf("Hago unlock en queue\n");
        //pthread_mutex_unlock(&mut);
        //printf("Fin Hago unlock en queue\n");
        
        //long oldtime = getTicks();
        do
        {
            //long time = getTicks();
            usleep(100);
            /*
             if((time - oldtime) > 20)
             {
             sched_yield();
             oldtime = time;
             }
             */
        }
        while (fullQueue(size));
        
        
        //sched_yield();
        //continue;
    }
    
    unsigned newhead;
    if(head + size < TAM)
    {
        memcpy(ptr_buf+head,p,size);
        newhead = head + size;
    }
    else
    {
        memcpy(ptr_buf+head,p, TAM -head);
        memcpy(ptr_buf,p + (TAM-head), size - (TAM-head));
        newhead = (head + size) - TAM;
    }
    pthread_mutex_lock(&mut);
    
    head = newhead;
    //printf("QUEUE2 head %d tail %d size %d\n",head, tail,size);
    //printf("Hago unlock en queue\n");
    pthread_mutex_unlock(&mut);
    //printf("Fin Hago unlock en queue\n");
    //break;
	//}
    
}

unsigned short dequeue(unsigned char *p,unsigned size)
{
    //printf("-->sacan %d h: %d t: %d TAM %d\n",size,head,tail,TAM);
	unsigned real;
    //	while(1){
    //printf("Hago lock en dequeue\n");
    
    //printf("Fin Hago lock en dequeue\n");
    
    if(emptyQueue())
    {
        //printf("---> VACIA\n");
        //printf("Hago unlock en dequeue\n");
        //pthread_mutex_unlock(&mut);
        //printf("Fin Hago unlock en dequeue\n");
        memset(p,0,size);//TODO ver si quito para que no petardee
        return size;
        //return 0;
    }
    
    pthread_mutex_lock(&mut);
    
    unsigned datasize = head > tail ? head - tail : (TAM - tail) + head ;
    real = datasize > size ? size : datasize;
    
    if(tail + real < TAM)
    {
        memcpy(p,ptr_buf+tail,real);
        tail+=real;
    }
    else
    {
        memcpy(p,ptr_buf + tail, TAM - tail);
        memcpy(p+ (TAM-tail),ptr_buf , real - (TAM-tail));
        tail = (tail + real) - TAM;
    }
    
    //printf("-->DEQUEUE head %d tail %d size %d\n",head, tail,size);
    //printf("Hago unlock en dequeue\n");
    pthread_mutex_unlock(&mut);
    //printf("Fin Hago unlock en dequeue\n");
    //		break;
    //	}
    return real;
    
    
    
}



int sound_send(void *samples,int nsamples)
{
	queue(samples,nsamples * 2);
    return 1;
}


static OSStatus playbackCallback(void *inRefCon,
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inBusNumber,
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData) {
    // Notes: ioData contains buffers (may be more than one!)
    // Fill them up as much as you can. Remember to set the size value in each buffer to match how
    // much data is in the buffer.
    
	//printf("@@ num buf %d size %d frmames %d @@  ",ioData->mNumberBuffers,ioData->mBuffers[0].mDataByteSize,inNumberFrames);
    
	unsigned  char *coreAudioBuffer;
    //coreAudioBuffer = (unsigned char*) ioData->mBuffers[0].mData;
    //ioData->mBuffers[0].mDataByteSize = 0;
    //memset(coreAudioBuffer,0,inNumberFrames *4);
    /*
     ioData->mBuffers[0].mDataByteSize = BUF;
     
     dequeue((signed short *)coreAudioBuffer,BUF);
     */
    
    
    int i;
    for (i = 0 ; i < ioData->mNumberBuffers; i++){
        coreAudioBuffer = (unsigned char*) ioData->mBuffers[i].mData;
        ioData->mBuffers[i].mDataByteSize = dequeue(coreAudioBuffer,/*BUF*/stereof ? inNumberFrames * 4: inNumberFrames * 2);
        //ioData->mBuffers[i].mDataByteSize = BUF;
        
    }
    
    
    /*
     for (int i = 0 ; i < ioData->mNumberBuffers; i++){
     AudioBuffer buffer = ioData->mBuffers[i];
     UInt16 *frameBuffer = buffer.mData;
     UInt16 packet;
     //loop through the buffer and fill the frames
     for (int j = 0; j < inNumberFrames; j++){
     // get NextPacket returns a 32 bit value, one frame.
     packet = [file getNextPacket];
     frameBuffer[j] = packet;
     }
     }
     */
    return noErr;
}

static void AQBufferCallback(
							 void *userdata,
							 AudioQueueRef outQ,
							 AudioQueueBufferRef outQB)
{
	unsigned char *coreAudioBuffer;
	coreAudioBuffer = (unsigned char*) outQB->mAudioData;
    
	//outQB->mAudioDataByteSize = BUF;
	//AudioQueueSetParameter(outQ, kAudioQueueParam_Volume, __audioVolume);
    
	int res = dequeue(coreAudioBuffer,BUF);
	outQB->mAudioDataByteSize = res;
    
	AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
}

#define kOutputBus 0

static AudioComponentInstance audioUnit;


int sound_open(int rate, int bits, int stereo)
{
    Float64 sampleRate = rate;
    
    stereof = stereo;
    
    if( soundInit == 1 ) {
        sound_close();
    }
    pthread_mutex_init (&mut,NULL);
    
    audioBufferSize =  (rate / 50) * 2 * (stereo==1 ? 2 : 1) ;
    
    OSStatus status;
    
    // Describe audio component
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_RemoteIO;
    //desc.componentType = kAudioUnitType_Output;
    //desc.componentSubType = kAudioUnitSubType_GenericOutput;
    
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    // Get component
    AudioComponent inputComponent = AudioComponentFindNext(NULL, &desc);
    
    // Get audio units
    status = AudioComponentInstanceNew(inputComponent, &audioUnit);
    
    UInt32 flag = 1;
    // Enable IO for playback
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output,
                                  kOutputBus,
                                  &flag,
                                  sizeof(flag));
    
    AudioStreamBasicDescription audioFormat;
    /*
     UInt32 size = sizeof(AudioStreamBasicDescription);
     
     AudioUnitGetProperty (audioUnit,
     kAudioUnitProperty_StreamFormat,
     kAudioUnitScope_Input,
     kOutputBus,
     &audioFormat,
     &size);
     printf("format %f %d %d %d %d %d\n",
     audioFormat.mSampleRate,
     audioFormat.mBytesPerPacket,
     audioFormat.mFramesPerPacket,
     audioFormat.mBytesPerFrame,
     audioFormat.mChannelsPerFrame,
     audioFormat.mBitsPerChannel);
     */
    memset (&audioFormat, 0, sizeof (audioFormat));
    
    // Describe format
    audioFormat.mSampleRate = sampleRate;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger  | kAudioFormatFlagIsPacked;
    audioFormat.mBytesPerPacket =  (stereo == 1 ? 4 : 2 );
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerFrame = (stereo ==  1? 4 : 2);
    audioFormat.mChannelsPerFrame = (stereo == 1 ? 2 : 1);
    audioFormat.mBitsPerChannel = 16;
    
    //MxInitialize(audioFormat.mSampleRate);
    /*
     printf("format %f %d %d %d %d %d\n",
     audioFormat.mSampleRate,
     audioFormat.mBytesPerPacket,
     audioFormat.mFramesPerPacket,
     audioFormat.mBytesPerFrame,
     audioFormat.mChannelsPerFrame,
     audioFormat.mBitsPerChannel);
     */
    // Apply format
    
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  kOutputBus,
                                  &audioFormat,
                                  sizeof(audioFormat));
    
    struct AURenderCallbackStruct callbackStruct;
    // Set output callback
    callbackStruct.inputProc = playbackCallback;
    callbackStruct.inputProcRefCon = NULL;
    status = AudioUnitSetProperty(audioUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global,
                                  kOutputBus,
                                  &callbackStruct,
                                  sizeof(callbackStruct));
    
    //set buffers sizes
    //float aBufferLength = 0.010; // In seconds
    //AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration,sizeof(aBufferLength), &aBufferLength);
    
    // TODO: Allocate our own buffers if we want
    
    // Initialise
    status = AudioUnitInitialize(audioUnit);
    
    //ARANCAR
    soundInit = 1;
    status = AudioOutputUnitStart(audioUnit);
    
    return 1;
}

int sound_close(void){
    
	if( soundInit == 1 )
	{
        pthread_mutex_destroy(&mut);
        
		AudioOutputUnitStop(audioUnit);
        
		AudioUnitUninitialize(audioUnit);
		soundInit = 0;
	}
    
    return 1;
    
}


