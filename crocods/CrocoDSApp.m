//
//  CrocoDSApp.m
//  CrocoDS
//
//  Created by Miguel Vanhove on 2/12/12.
//  Copyright (c) 2012 TEC Hainaut. All rights reserved.
//

#import "CrocoDSApp.h"

@implementation crocodsApp

#define GSEVENT_TYPE 2
#define GSEVENT_FLAGS 12
#define GSEVENTKEY_KEYCODE 15
#define GSEVENT_TYPE_KEYUP 11

NSString *const GSEventKeyUpNotification = @"GSEventKeyUpHackNotification";

- (void)sendEvent:(UIEvent *)event      // Not used !
{
    [super sendEvent:event];
    
    if ([event respondsToSelector:@selector(_gsEvent)]) {
        
        // Key events come in form of UIInternalEvents.
        // They contain a GSEvent object which contains
        // a GSEventRecord among other things
        
        int *eventMem;
        eventMem = (int *)[event performSelector:@selector(_gsEvent)];
        if (eventMem) {
            
            // So far we got a GSEvent :)
            
            int eventType = eventMem[GSEVENT_TYPE];
            if (eventType == GSEVENT_TYPE_KEYUP) {
                
                // Now we got a GSEventKey!
                
                
                    // Read keycode from GSEventKey
                    int tmp = eventMem[GSEVENTKEY_KEYCODE];
                    UniChar *keycode = (UniChar *)&tmp;
                                        
                    // Post notification
                    NSDictionary *inf;
                    inf = [[NSDictionary alloc] initWithObjectsAndKeys:
                           [NSNumber numberWithShort:keycode[0]],
                           @"keycode",
                           nil];
                    [[NSNotificationCenter defaultCenter] postNotificationName:GSEventKeyUpNotification object:nil  userInfo:inf];

            }}
    }
}

@end
