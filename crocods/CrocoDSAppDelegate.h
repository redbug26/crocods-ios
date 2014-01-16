//
//  biopocketAppDelegate.h
//  biopocket
//
//  Created by Miguel Vanhove on 3/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>

@class crocodsViewController;

@interface crocodsAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *mainWindow; // Main App Window
	UINavigationController *navigationController;
        
    bool _useIcade;
    bool _useFling;
    bool _useExternalKeyboard;
}

+(crocodsAppDelegate *)delegate;


@property (nonatomic, assign) bool useIcade;
@property (nonatomic, assign) bool useFling;
@property (nonatomic, assign) bool useExternalKeyboard;

@end
