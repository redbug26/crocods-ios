//
//  AboutViewController.m
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import "AboutViewController.h"


@implementation AboutViewController

@synthesize webView;

@synthesize toolbar;

- (id)init {
    if (self =[super init]) {
        // Create toolbar (to make sure that we can access it at any time)
		_toolbar = [[UIToolbar alloc] initWithFrame:CGRectZero];

        self.title = NSLocalizedString(@"About Crocods", @"Series title");
        self.tabBarItem.image = [UIImage imageNamed:@"about.png"];   
    }
    return self;
}

- (void)loadView {
    
    NSLog(@"loading view - old");
    
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    
    CGRect bounds = [[UIScreen mainScreen] applicationFrame];
    
    NSLog(@"scroll: %f/%f", bounds.size.width, bounds.size.height  );
    
    
    UIView *scrollView = [[UIView alloc] initWithFrame:bounds];
//    [scrollView setBackgroundColor:[UIColor grayColor]];
    [scrollView setBackgroundColor:[[UIColor alloc] initWithPatternImage:[UIImage imageNamed:@"about-background.png"]]];

    UIWebView *aWebView = [[UIWebView alloc] initWithFrame:[scrollView bounds]];
    
    
    self.webView = aWebView;
    aWebView.scalesPageToFit = YES;
    aWebView.autoresizesSubviews = YES;
    aWebView.autoresizingMask=(UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth);
    
    //set the web view and acceleration delagates for the web view to be itself
    [aWebView setDelegate:self];
    //determine the path the to the index.html file in the Resources directory
    //build the URL and the request for the index.html file
    
    NSURL *baseURL = [NSURL fileURLWithPath:[[NSBundle mainBundle] resourcePath]];   //  @"about.html"]];
    NSURL *aURL = [[NSURL alloc] initWithScheme:[baseURL scheme] host:[baseURL host] path:[[baseURL path] stringByAppendingPathComponent:@"about.html"]];
    
    NSURLRequest *aRequest = [NSURLRequest requestWithURL:aURL];
    [aWebView loadRequest:aRequest];
    
    aWebView.backgroundColor = [UIColor groupTableViewBackgroundColor];
    aWebView.opaque = NO;
    
    [scrollView addSubview:aWebView];
    
    self.view = scrollView;
    
    
    CGRect frame = self.view.bounds;

    
    // Create action button. This shows a selection of available actions in context of the displayed page
	_actionButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAction
																  target:self
																  action:@selector(showAvailableActions)];
	
	// Create reload button to reload the current page
	_reloadButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                  target:self
                                                                  action:@selector(reload)];
	
	// Create loading button that is displayed if the web view is loading anything
	UIActivityIndicatorView *activityView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
	[activityView startAnimating];
	_loadingButton = [[UIBarButtonItem alloc] initWithCustomView:activityView];
	
	// Shows the next page, is disabled by default. Web view checks if it can go forward and disables the button if neccessary
	_forwardButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"PWWebViewControllerArrowRight.png"] style:UIBarButtonItemStylePlain
													 target:self
													 action:@selector(goForward)];
	_forwardButton.enabled = NO;
	
	// Shows the last page, is disabled by default. Web view checks if it can go back and disables the button if neccessary
	_backButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"PWWebViewControllerArrowLeft.png"] style:UIBarButtonItemStylePlain
												  target:self
												  action:@selector(goBack)];
	_backButton.enabled = NO;
    
    // auto fill
    
    _autoFill = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"autofill.png"] style:UIBarButtonItemStylePlain
                                                target:self
                                                action:@selector(autoFill)];
	_autoFill.enabled = NO;
    
    
	// Setup toolbar
	_toolbar.frame = CGRectMake(frame.origin.x, frame.origin.y + frame.size.height - 44.0, frame.size.width, 44.0);
	_toolbar.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
								 UIViewAutoresizingFlexibleTopMargin |
								 UIViewAutoresizingFlexibleHeight);
	[self.view addSubview:_toolbar];
	
	// Flexible space
	_flexibleSpace = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:nil action:NULL];
	
	// Assign buttons to toolbar
	_toolbar.items = [NSArray arrayWithObjects:_actionButton, _flexibleSpace, _backButton, _flexibleSpace, _autoFill, _flexibleSpace, _forwardButton, _flexibleSpace, _reloadButton, nil];

}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return YES;
}


#pragma mark -
#pragma mark Button actions

- (void)showAvailableActions
{
	// Create action sheet without any buttons
	UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:[self.webView.request.URL absoluteString]
															 delegate:self
													cancelButtonTitle:nil destructiveButtonTitle:nil otherButtonTitles:nil];
	
	// Add buttons
	[actionSheet addButtonWithTitle:NSLocalizedString(@"Open in Safari", nil)];
	
    
	// Add cancel button and mark is as cancel button
	[actionSheet addButtonWithTitle:NSLocalizedString(@"Cancel", nil)];
	actionSheet.cancelButtonIndex = actionSheet.numberOfButtons - 1;
	
	// Assign tag, show it from toolbar and release it
	actionSheet.tag = kPWWebViewControllerActionSheetTag;
	[actionSheet showFromToolbar:_toolbar];
}

- (void)reload
{
	[self.webView reload];
}

- (void)goBack
{
	if (self.webView.canGoBack == YES) {
		// We can go back. So make the web view load the previous page.
		[self.webView goBack];
		
		// Check the status of the forward/back buttons
		[self checkNavigationStatus];
	}
}

- (void)goForward
{
	if (self.webView.canGoForward == YES) {
		// We can go forward. So make the web view load the next page.
		[self.webView goForward];
		
		// Check the status of the forward/back buttons
		[self checkNavigationStatus];
	}
}


#pragma mark -
#pragma mark UIWebViewDelegate

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
    if ((navigationType == UIWebViewNavigationTypeLinkClicked) && ([[[request URL] absoluteString] hasSuffix:@"dsk"] || [[[request URL] absoluteString] hasSuffix:@"zip"])) {
        [[UIApplication sharedApplication] openURL:request.URL];
        return NO;
    }
    
    return YES;
}

/*
 - (BOOL)webView:(UIWebView *)wv shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {
 
 // Determine if we want the system to handle it.
 NSURL *url = request.URL;
 if (![url.scheme isEqual:@"http"] && ![url.scheme isEqual:@"https"]) {
 if ([[UIApplication sharedApplication]canOpenURL:url]) {
 [[UIApplication sharedApplication]openURL:url];
 return NO;
 }
 }
 return YES;
 }
 */

- (void)webViewDidStartLoad:(UIWebView *)webView
{
	// Change toolbar items
	_toolbar.items = [NSArray arrayWithObjects:_actionButton, _flexibleSpace, _backButton, _flexibleSpace, _autoFill, _flexibleSpace, _forwardButton, _flexibleSpace, _loadingButton, nil];
	
	// Set title
	self.navigationItem.title = NSLocalizedString(@"Loading...", nil);
}

- (void)webViewDidFinishLoad:(UIWebView *)webView0
{
	// Change toolbar items
	_toolbar.items = [NSArray arrayWithObjects:_actionButton, _flexibleSpace, _backButton, _flexibleSpace, _autoFill, _flexibleSpace, _forwardButton, _flexibleSpace, _reloadButton, nil];
	
	// Set title
	NSString *title = [webView0 stringByEvaluatingJavaScriptFromString:@"document.title"];
	self.navigationItem.title = title;
    
	
	// Check if forward/back buttons are available
	[self checkNavigationStatus];
}




- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
	// Change toolbar items
	_toolbar.items = [NSArray arrayWithObjects:_actionButton, _flexibleSpace, _backButton, _flexibleSpace, _autoFill, _flexibleSpace, _forwardButton, _flexibleSpace, _reloadButton, nil];
	
	// Check if forward/back buttons are available
	[self checkNavigationStatus];
	
	// Set title
	self.navigationItem.title = NSLocalizedString(@"Page not found", nil);
	
	// Display an alert view that tells the userr what went wrong.
	UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:NSLocalizedString(@"Connection did fail", nil)
														message:[error localizedDescription]
													   delegate:nil
											  cancelButtonTitle:NSLocalizedString(@"OK", nil)
											  otherButtonTitles:nil];
	[alertView show];
}


#pragma mark -
#pragma mark UIActionSheetDelegate

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
	if (actionSheet.tag == kPWWebViewControllerActionSheetTag && buttonIndex != actionSheet.cancelButtonIndex) {
		// It is one of your action sheets and it was not canceled
		if (buttonIndex == kPWWebViewControllerActionSheetSafariIndex) {
			// Open URL in Safari
			[[UIApplication sharedApplication] openURL:self.webView.request.URL];
		} else if (buttonIndex == kPWWebViewControllerActionSheetMailIndex) {
			
        }
	}
}


#pragma mark -
#pragma mark Private methods

- (void)checkNavigationStatus
{
	// Check if we can go forward or back
	_backButton.enabled = self.webView.canGoBack;
	_forwardButton.enabled = self.webView.canGoForward;
}


#pragma mark -



@end
