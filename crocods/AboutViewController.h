//
//  AboutViewController.h
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>

// You can use this to identify a action sheet tag that was created by PWWebViewController
#define kPWWebViewControllerActionSheetTag         5000
#define kPWWebViewControllerActionSheetMailIndex   1
#define kPWWebViewControllerActionSheetSafariIndex 0

@interface AboutViewController : UIViewController<UIWebViewDelegate, UIActionSheetDelegate> {
    UIWebView *webView;

	// Toolbar and used buttons
	UIToolbar *_toolbar;
	UIBarButtonItem *_actionButton;
	UIBarButtonItem *_reloadButton;
	UIBarButtonItem *_loadingButton;
	UIBarButtonItem *_forwardButton;
	UIBarButtonItem *_backButton;
	UIBarButtonItem *_flexibleSpace;
    UIBarButtonItem *_autoFill;
    
	/* This is used to store the request if the view is loaded.
     Important if view was released because of low memory conditions */
	NSURLRequest *_request;
}

@property (nonatomic, strong) UIWebView *webView;
@property (weak, nonatomic, readonly) UIToolbar *toolbar;


// Shows all available actions in a UIActionSheet that can be performed on the current page
- (void)showAvailableActions;

// Reloads the current website
- (void)reload;

// Go one site back, if available
- (void)goBack;

// Go on site forward, if available
- (void)goForward;

@end
