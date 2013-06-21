//
//  AboutViewController.m
//  Doobs
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 TEC Hainaut. All rights reserved.
//

#import "CrocoDSAppDelegate.h"
#import "CrocoDSViewController.h"
#include <QuartzCore/CADisplayLink.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/glext.h>
#import "MyKeyboard.h"

#import "nds.h"
#import "upd.h"
#import "autotype.h"
#import "snapshot.h"
#import "plateform.h"

#import "AboutViewController.h"
#import "ConfigViewController.h"
#import "MyCatalog.h"
    
#define RAYON 20

@implementation crocodsViewController


- (id)init {
    if (self =[super init]) {
        self.title = NSLocalizedString(@"About", @"Series title");
        self.tabBarItem.image = [UIImage imageNamed:@"145-persondot.png"];
    }
    
    NSLog(@"init");
    
    return self;
}


-(void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    NSLog(@"response: %@", response.products);
    NSLog(@"invalid: %@", response.invalidProductIdentifiers);

    NSLog(@"response: %@", response); 

    NSLog(@"response: %@", request);

}

-(void)requestDidFinish:(SKRequest *)request
{
    [request release];
}

-(void)request:(SKRequest *)request didFailWithError:(NSError *)error
{
    NSLog(@"Failed to connect with error: %@", [error localizedDescription]);
}



- (void)viewDidLoad {
    [super viewDidLoad];
    
    
    
    SKProductsRequest *request = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:@[@"be.kyuran.crocods.pro"]]];
    request.delegate = self;
    [request start];
    
    // Copy all DSK to Documents directory
    
    NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[[NSBundle mainBundle] bundlePath] error:nil];
    NSArray *dsks = [files filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self ENDSWITH[cd] '.dsk'"]];
    
    NSString *documentsDirectory = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    
    for (NSString *dsk in dsks) {
        [[NSFileManager defaultManager] copyItemAtPath:[[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:dsk] toPath:[NSString stringWithFormat:@"%@/%@",documentsDirectory,dsk] error:nil];
    }
    
    
    portraitBounds = [[UIScreen mainScreen] applicationFrame];
    
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    
    m_oglView = [[OGLView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.width * 0.625) width:384 height:272 fps:15];
    m_oglView.backgroundColor = [UIColor colorWithRed:0/255.0 green:0/255.0 blue:0/255.0 alpha:1];
    m_oglView.parent = self;
    m_oglView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:m_oglView];
    
    self.view.backgroundColor=[UIColor blackColor];
    
    float keyboardHeight=bounds.size.width * 0.75;
    if (keyboardHeight>bounds.size.height-bounds.size.width * 0.625) {
        keyboardHeight=bounds.size.height-bounds.size.width * 0.625;
    }
    
    mykeyboard=[[MyKeyboard alloc] initWithImage:[UIImage imageNamed:@"background.png"]];
    mykeyboard.frame=CGRectMake(bounds.origin.x, bounds.size.height - keyboardHeight, bounds.size.width, keyboardHeight); //  200, width, 260);
    [self.view addSubview:mykeyboard];
    
    int w,h;
    
    w=m_oglView.frame.size.width/6;
    h=m_oglView.frame.size.height/4;
    
    
    key_up=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_up.png"]];
    key_up.frame=CGRectMake( w * 1,  h * 1, 48,48);
    key_up.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_up];
    
    key_down=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_down.png"]];
    key_down.frame=CGRectMake( w * 1,  h * 3, 48,48);
    key_down.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_down];
    
    key_left=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_left.png"]];
    key_left.frame=CGRectMake( w * 0,  h * 2, 48,48);
    key_left.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_left];
    
    key_right=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_right.png"]];
    key_right.frame=CGRectMake( w * 2,  h * 2, 48,48);
    key_right.autoresizingMask = UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_right];
    
    
    centerLocation=CGPointMake(100,100);
    key_left.center = CGPointMake(centerLocation.x - RAYON*2, centerLocation.y);
    key_right.center = CGPointMake(centerLocation.x + RAYON*2, centerLocation.y);
    key_up.center = CGPointMake(centerLocation.x, centerLocation.y - RAYON*2);
    key_down.center = CGPointMake(centerLocation.x, centerLocation.y + RAYON*2);
    
    key_a=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_a.png"]];
    key_a.frame=CGRectMake( w * 4,  h * 3, 48,48);
    key_a.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_a];
    
    key_b=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_b.png"]];
    key_b.frame=CGRectMake( w * 5,  h * 3, 48,48);
    key_b.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleTopMargin;
    [m_oglView addSubview:key_b];
    
    key_select=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_select.png"]];
    key_select.frame=CGRectMake( w * 4,  h * 0, 48,48);
    key_select.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    [m_oglView addSubview:key_select];
    
    key_start=[[UIImageView alloc] initWithImage:[UIImage imageNamed:@"key_start.png"]];
    key_start.frame=CGRectMake( w * 5,  h * 0, 48,48);
    key_start.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    [m_oglView addSubview:key_start];
}


- (void)showSettingsView {
    UIBarButtonItem *doneButton;
    
    m_oglView.isPaused=true;
    isPaused=1;
    
    UITabBarController *myNavigationController = [[UITabBarController alloc] init];
    myNavigationController.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
    
    // Catalog
    
    
    MyCatalog *catalogViewController = [[MyCatalog alloc] init];
    UINavigationController *catalogNavController = [[UINavigationController alloc] initWithRootViewController:catalogViewController];
    
    doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(dismissSettingsView)];
    catalogViewController.navigationItem.rightBarButtonItem = doneButton;
    [doneButton release];
    
    catalogViewController.delegate = (UIResponder*)self;
    
    [catalogViewController release];
    
    // Config
    
    
    ConfigViewController *settingsViewController = [[ConfigViewController alloc] init];
    UINavigationController *settingsNavController = [[UINavigationController alloc] initWithRootViewController:settingsViewController];
    
    doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(dismissSettingsView)];
    settingsViewController.navigationItem.rightBarButtonItem = doneButton;
    [doneButton release];
    
    [settingsViewController release];
    
    
    //--- About
    
    AboutViewController *aboutView = [[AboutViewController alloc] init];
    UINavigationController *aboutNavController = [[UINavigationController alloc] initWithRootViewController:aboutView];
    
    doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(dismissSettingsView)];
    aboutView.navigationItem.rightBarButtonItem = doneButton;
    [doneButton release];
    
    [aboutView release];
    
    // -- Init
    
    NSArray * controllers = [NSArray arrayWithObjects:catalogNavController, settingsNavController, aboutNavController, nil];
    myNavigationController.viewControllers = controllers;
    
    
    [self presentModalViewController:myNavigationController animated:YES];
    
    [settingsNavController release];
    [aboutNavController release];
    
    [myNavigationController release];
    
    /*
     
     ConfigViewController *settingsViewController = [[ConfigViewController alloc] initWithStyle:UITableViewStyleGrouped];
     
     UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone target:self action:@selector(dismissSettingsView)];
     settingsViewController.navigationItem.rightBarButtonItem = doneButton;
     
     [doneButton release];
     
     UINavigationController *settingsNavController = [[UINavigationController alloc] initWithRootViewController:settingsViewController];
     
     settingsNavController.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
     
     [self presentModalViewController:settingsNavController animated:YES];
     
     [settingsViewController release];
     [settingsNavController release];
     */
}


- (void)dismissSettingsView {
    
    m_oglView.isPaused=false;
    isPaused=0;
    
    [self dismissModalViewControllerAnimated:YES];
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];
    
    CGPoint location = [touch locationInView:m_oglView];
    if (location.y<m_oglView.frame.size.height) {       // Click dans l'ecran
        UIView *viewEffectX, *viewEffectY;
        
        if (location.x<m_oglView.frame.size.width/2) {
            if (location.x-centerLocation.x>RAYON) {
                viewEffectX=key_right;
                ipc.keys_pressed |= KEY_RIGHT;
            } else if (centerLocation.x-location.x>RAYON) {
                ipc.keys_pressed |= KEY_LEFT;
                viewEffectX=key_left;
            } else {
                ipc.keys_pressed &= (~KEY_LEFT);
                ipc.keys_pressed &= (~KEY_RIGHT);
                viewEffectX=nil;
                [key_right setAlpha:(1.0)];
                [key_left setAlpha:(1.0)];
            }
            if (viewEffectX!=nil) {
                [UIView beginAnimations:@"" context:NULL];
                [viewEffectX setAlpha:(0.0)];
                [UIView setAnimationDuration:0.5];
                [UIView commitAnimations];
            }
            if (location.y-centerLocation.y>RAYON) {
                ipc.keys_pressed |= KEY_DOWN;
                viewEffectY=key_down;
            } else if (centerLocation.y-location.y>RAYON) {
                ipc.keys_pressed |= KEY_UP;
                viewEffectY=key_up;
            } else {
                ipc.keys_pressed &= (~KEY_UP);
                ipc.keys_pressed &= (~KEY_DOWN);
                viewEffectY=nil;
                [key_up setAlpha:(1.0)];
                [key_down setAlpha:(1.0)];
            }
            if (viewEffectY!=nil) {
                [UIView beginAnimations:@"" context:NULL];
                [viewEffectY setAlpha:(0.0)];
                [UIView setAnimationDuration:0.5];
                [UIView commitAnimations];
            }
         }
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];
    
    CGPoint location = [touch locationInView:m_oglView];
    if (location.y<m_oglView.frame.size.height) {       // Click dans l'ecran
        UIView *viewEffect=nil;
        
        if (CGRectContainsPoint(key_left.frame, location)) {
            ipc.keys_pressed |= KEY_LEFT;
            viewEffect=key_left;
        } else if (CGRectContainsPoint(key_right.frame, location)) {
            ipc.keys_pressed |= KEY_RIGHT;
            viewEffect=key_right;
        } else if (CGRectContainsPoint(key_up.frame, location)) {
            ipc.keys_pressed |= KEY_UP;
            viewEffect=key_up;
        } else if (CGRectContainsPoint(key_down.frame, location)) {
            ipc.keys_pressed |= KEY_DOWN;
            viewEffect=key_down;
        } else if (CGRectContainsPoint(key_a.frame, location)) {
            ipc.keys_pressed |= KEY_A;
            viewEffect=key_a;
        } else if (CGRectContainsPoint(key_b.frame, location)) {
            ipc.keys_pressed |= KEY_B;
            viewEffect=key_b;
        } else if (CGRectContainsPoint(key_select.frame, location)) {
            ipc.keys_pressed |= KEY_SELECT;
            viewEffect=key_select;
        } else if (CGRectContainsPoint(key_start.frame, location)) {
            ipc.keys_pressed |= KEY_START;
            viewEffect=key_start;
        } else if (location.x<m_oglView.frame.size.width/2) {
            centerLocation=location;
            key_left.center = CGPointMake(centerLocation.x - RAYON*2, centerLocation.y);
                        key_right.center = CGPointMake(centerLocation.x + RAYON*2, centerLocation.y);
                        key_up.center = CGPointMake(centerLocation.x, centerLocation.y - RAYON*2);
                        key_down.center = CGPointMake(centerLocation.x, centerLocation.y + RAYON*2);
        }
        
        if (viewEffect!=nil) {
            [UIView beginAnimations:@"" context:NULL];
            [viewEffect setAlpha:(0.0)];
            [UIView setAnimationDuration:0.5];
            [UIView commitAnimations];
        }
        
        return;
    }
    
    location = [touch locationInView:mykeyboard];
    
    int x,y;
    
    x=(location.x*256)/mykeyboard.frame.size.width;
    y=(location.y*192)/mykeyboard.frame.size.height;
    
    //    NSLog(@"location: %f,%f", location.x, location.y);
    
    if (y<64) {
        [self showSettingsView];
        /*
         if (location.x<160) {
         char autofile[256];
         
         NSData *dsk=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"bombjack" ofType:@"dsk"]];
         LireDiskMem((u8*)[dsk bytes], [dsk length], autofile);
         
         if (autofile[0]!=0) {
         char buffer[256];
         sprintf(buffer,"run\"%s\n", autofile);
         AutoType_SetString(buffer, TRUE);
         }
         } else {
         NSData *rom=[[NSData alloc] initWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"boulder" ofType:@"sna"]];
         LireSnapshotMem((u8*)[rom bytes]);
         }
         */
        return;
    }
    
    
    ipc.touchXpx=x;
    ipc.touchYpx=y;
    ipc.touchDown=1;
    
    NSLog(@"keyboard: %d,%d", ipc.touchXpx, ipc.touchYpx);
}


- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[touches allObjects] objectAtIndex:0];
    
    CGPoint location = [touch locationInView:m_oglView];
    
    ipc.touchDown=0;
    
    if (ipc.keys_pressed!=0) {
        UIView *viewEffect=nil;
        
        if ((ipc.keys_pressed | KEY_LEFT) == KEY_LEFT) {
            viewEffect=key_left;
        } else if ((ipc.keys_pressed | KEY_RIGHT) == KEY_RIGHT) {
            viewEffect=key_right;
        } else if ((ipc.keys_pressed | KEY_UP) == KEY_UP) {
            viewEffect=key_up;
        } else if ((ipc.keys_pressed | KEY_DOWN) == KEY_DOWN) {
            viewEffect=key_down;
        } else if ((ipc.keys_pressed | KEY_A) == KEY_A) {
            viewEffect=key_a;
        } else if ((ipc.keys_pressed | KEY_B) == KEY_B) {
            viewEffect=key_b;
        } else if ((ipc.keys_pressed | KEY_SELECT) == KEY_SELECT) {
            viewEffect=key_select;
        } else if ((ipc.keys_pressed | KEY_START) == KEY_START) {
            viewEffect=key_start;
        }
        
        if (viewEffect!=nil) {
            [UIView beginAnimations:@"" context:NULL];
            [viewEffect setAlpha:(1.0)];
            [UIView setAnimationDuration:0.5];
            [UIView commitAnimations];
        }
    }
    
    [key_up setAlpha:(1.0)];
    [key_down setAlpha:(1.0)];
    [key_left setAlpha:(1.0)];
    [key_right setAlpha:(1.0)];
    
    ipc.keys_pressed=0;
    
    if (location.y<m_oglView.frame.size.height) {       // Click dans l'ecran
        
        return;
    }
}


-(NSInteger)supportedInterfaceOrientations:(UIWindow *)window{
    
    NSLog(@"window: %@", window);
    
    return 24;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    [self shouldAutorotate];
}

- (BOOL)shouldAutorotate {
    
    NSLog(@"shouldAutorotate");
    
    if (![crocodsAppDelegate delegate].isPro) {
        NSLog(@"!!!! non pro !!");
        return false;
    }
    
    UIInterfaceOrientation interfaceOrientation = [[UIDevice currentDevice] orientation];
    
    if (interfaceOrientation!=UIDeviceOrientationUnknown) {
        
        switch(interfaceOrientation) {
            case UIDeviceOrientationUnknown: NSLog(@"Unknown"); break;
            case UIDeviceOrientationPortrait: NSLog(@"Portrait"); break;            // Device oriented vertically, home button on the bottom
            case UIDeviceOrientationPortraitUpsideDown: NSLog(@"Portrait Upside Down"); break;  // Device oriented vertically, home button on the top
            case UIDeviceOrientationLandscapeLeft: NSLog(@"Landscape Left"); break;       // Device oriented horizontally, home button on the right
            case UIDeviceOrientationLandscapeRight: NSLog(@"Landscape Right"); break;     // Device oriented horizontally, home button on the left
            case UIDeviceOrientationFaceUp: NSLog(@"Face Up"); break;              // Device oriented flat, face up
            case UIDeviceOrientationFaceDown: NSLog(@"Face Down"); break;
        }
        
        if (UIInterfaceOrientationIsPortrait(interfaceOrientation)) {
            mykeyboard.hidden=false;
            
            m_oglView.frame=CGRectMake(0, 0,  portraitBounds.size.width,  portraitBounds.size.width * 0.625);
            m_oglView.autoresizingMask = UIViewAutoresizingNone;
            
        } else {
            NSComparisonResult order = [[UIDevice currentDevice].systemVersion compare: @"5.0" options: NSNumericSearch];
            if (order == NSOrderedSame || order == NSOrderedDescending) {
                m_oglView.frame=CGRectMake(0, 0,  self.view.frame.size.height, self.view.frame.size.width);         // On iOS5 only
            } else {
                if (self.view.frame.size.width>self.view.frame.size.height) {
                    m_oglView.frame=CGRectMake(0, 0,  self.view.frame.size.height, self.view.frame.size.width);
                } else {
                    m_oglView.frame=CGRectMake(0, 0,  self.view.frame.size.width, self.view.frame.size.height);
                }
            }
            m_oglView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [m_oglView sizeToFit];
            mykeyboard.hidden=true;
        }
        
    }
    
    return true;
}


- (void)dealloc {
    [key_start release];
    [key_select release];
    [key_b release];
    [key_a release];
    [key_right release];
    [key_left release];
    [key_up release];
    [key_down release];
    
    [super dealloc];
}



@end
