//
//  nds.h
//  crocods
//
//  Created by Miguel Vanhove on 17/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#ifndef crocods_nds_h
#define crocods_nds_h

#define 	BIT(n)   (1 << (n))

enum  	KEYPAD_BITS { 
    KEY_A = BIT(0), KEY_B = BIT(1), KEY_SELECT = BIT(2), KEY_START = BIT(3), 
    KEY_RIGHT = BIT(4), KEY_LEFT = BIT(5), KEY_UP = BIT(6), KEY_DOWN = BIT(7), 
    KEY_R = BIT(8), KEY_L = BIT(9), KEY_X = BIT(10), KEY_Y = BIT(11), 
    KEY_TOUCH = BIT(12), KEY_LID = BIT(13) 
};

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned long int u32;
typedef signed long int s32;
#endif
