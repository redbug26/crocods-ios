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
   
   Copyright (c) 2008 Seleuco. This microlib is based in the findings of  wiki.gp2x.org, 
   GnoStiC, kounch, Hermes, Rlyeh, and others... thanks to all.
   
*/

#ifndef __MICROLIB_H__
#define __MICROLIB_H__

#define SOUND_FREQ 22050

#ifdef __cplusplus
extern "C" {
#endif

void queue(unsigned char *p,unsigned size);
unsigned short dequeue(unsigned char *p,unsigned size);
    
//init,end, Seleuco
void microlib_init(void);
void microlib_end(void);
 

int            sound_open(int rate, int bits, int stereo);
int            sound_close(void);
void           sound_volume(int, int);
int            sound_send(void *samples,int nsamples);

#ifdef __cplusplus
}
#endif

    
    
#endif
