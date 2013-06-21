//
//  biopocketAppDelegate.h
//  biopocket
//
//  Created by Miguel Vanhove on 3/08/11.
//  Copyright 2011 TEC Hainaut. All rights reserved.
//

#import <UIKit/UIKit.h>

@class crocodsViewController;

@interface crocodsAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *mainWindow; // Main App Window
	UINavigationController *navigationController;
    
    bool isPro;
    
    bool _useIcade;
    bool _useFling;
    bool _useExternalKeyboard;
}

+(crocodsAppDelegate *)delegate;


@property (atomic, assign) bool isPro;

@property (nonatomic, assign) bool useIcade;
@property (nonatomic, assign) bool useFling;
@property (nonatomic, assign) bool useExternalKeyboard;

@end
