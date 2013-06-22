//
//  MyKeyboard.m
//  crocods
//
//  Created by Miguel Vanhove on 21/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#import "MyKeyboard.h"

@implementation MyKeyboard

- (id)init
{
    self = [super init];
    if (self) {
        self.userInteractionEnabled = YES;    
       
        NSLog(@"mykeyboard init");
    }
    
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    NSLog(@"began mykeyboard");
}


@end
