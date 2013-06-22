//
//  biopocketAppDelegate.m
//  biopocket
//
//  Created by Miguel Vanhove on 3/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#import "CrocoDSAppDelegate.h"
#import "CrocoDSViewController.h"

@implementation crocodsAppDelegate

@synthesize isPro;

- (void)setUseExternalKeyboard:(bool)useExternalKeyboard {
    [[NSUserDefaults standardUserDefaults] setBool:useExternalKeyboard forKey:@"useExternalKeyboard"];
    _useExternalKeyboard=useExternalKeyboard;
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)setUseFling:(bool)useFling {
    [[NSUserDefaults standardUserDefaults] setBool:useFling forKey:@"useFling"];
    _useFling=useFling;
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)setUseIcade:(bool)useIcade {
    [[NSUserDefaults standardUserDefaults] setBool:useIcade forKey:@"useIcade"];
    _useIcade=useIcade;
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (bool)useExternalKeyboard {
    return _useExternalKeyboard;
}

- (bool)useFling {
    return _useFling;
}

- (bool)useIcade {
    return _useIcade;
}




- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    _useExternalKeyboard = [[NSUserDefaults standardUserDefaults] boolForKey:@"useExternalKeyboard"];
    _useFling= [[NSUserDefaults standardUserDefaults] boolForKey:@"useFling"];
    _useIcade = [[NSUserDefaults standardUserDefaults] boolForKey:@"useIcade"];
    
//    int timeout=[[NSUserDefaults standardUserDefaults] integerForKey:@"timeout"];

    //    isPro=false;
    isPro=true;

    
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:nil];

    NSLog(@"didFinishLaunchingWithOptions");
    
    mainWindow = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	mainWindow.backgroundColor = [UIColor blackColor]; 
    
    crocodsViewController *viewcontroller = [[crocodsViewController alloc] init];
    
    mainWindow.rootViewController = viewcontroller;

    
	[mainWindow makeKeyAndVisible];

 //   [mainWindow release];
    

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{

}

- (void)applicationWillTerminate:(UIApplication *)application
{

}

- (BOOL)application:(UIApplication*)application openURL:(NSURL*)url sourceApplication:(NSString*)sourceApplication annotation:(id)annotation {
    
 /*
    if ([_fileManager.ksync handleOpenURL:url]) {
        return YES;
    }
  */
    
    // Get the filename
    NSString *filename = [url lastPathComponent];
    
    // Get the full path of where we're going to move the file
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *path = [documentsDirectory stringByAppendingPathComponent:filename];
    
    NSURL *newUrl = [NSURL fileURLWithPath:path];
    
    // Move input file into documents directory
    NSFileManager *fileManager = [NSFileManager defaultManager];
    [fileManager removeItemAtURL:newUrl error:nil];
    [fileManager moveItemAtURL:url toURL:newUrl error:nil];
    [fileManager removeItemAtPath:[documentsDirectory stringByAppendingPathComponent:@"Inbox"] error:nil];
    
    return YES;
}



+(crocodsAppDelegate *)delegate{
	return (crocodsAppDelegate *)[[UIApplication sharedApplication] delegate];
}

@end
