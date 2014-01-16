//
//  KKDraw_testViewController.h
//  KKDraw-test
//
//  Created by Miguel Vanhove on 13/07/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "OGLView.h"
#import "MyKeyboard.h"

@class CADisplayLink;

@class GCController;

@interface crocodsViewController : UIViewController {
    OGLView *m_oglView;
    CADisplayLink* m_displayLink;
    
    CGPoint centerLocation;
    
    UIView                  *inputView;

    MyKeyboard *mykeyboard;
    
    CGRect portraitBounds;
    
    UIImageView *key_up;
    UIImageView *key_down;
    UIImageView *key_left;
    UIImageView *key_right;
    UIImageView *key_a;
    UIImageView *key_b;
    UIImageView *key_select;
    UIImageView *key_start;
    
    GCController *myController;
}





@end
