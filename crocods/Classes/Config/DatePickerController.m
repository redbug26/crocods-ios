//
//  TDDatePickerController.m
//
//  Created by Nathan  Reed on 30/09/10.
//  Copyright 2010 Nathan Reed. All rights reserved.
//

#import "DatePickerController.h"


@implementation DatePickerController
@synthesize datePicker, delegate,reference;

-(id)init {
    self = [super init];
    if (self) {
        UIDatePicker *_datePicker = [[UIDatePicker alloc] init] ; // ]WithFrame:self.view.frame];
        _datePicker.date = [NSDate date];
        _datePicker.datePickerMode = UIDatePickerModeDate;
        
        CGSize pickerSize = [_datePicker sizeThatFits:CGSizeZero];

        _datePicker.frame = CGRectMake(0,
                                           self.view.frame.size.height - pickerSize.height, 
                                           self.view.frame.size.width, pickerSize.height);
        
        _datePicker.autoresizingMask = UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
        
        self.datePicker=_datePicker;
        
        [self.view addSubview:_datePicker];
        [_datePicker release];
        
        self.view.backgroundColor=[UIColor groupTableViewBackgroundColor];

        
    }
    return self;
}


-(void)viewDidLoad {
    [super viewDidLoad];

    UIBarButtonItem *anotherButton = [[UIBarButtonItem alloc] initWithTitle:@"Now" style:UIBarButtonItemStylePlain
                                                                     target:self action:@selector(setDateToNow:)];      
    self.navigationItem.rightBarButtonItem = anotherButton;
    


    
	/* for (UIView* subview in datePicker.subviews) {
		subview.frame = datePicker.bounds;
	} */
    
    
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return YES;
}

-(void)setDateToNow:(id)sender
{
    self.datePicker.date = [NSDate date];
}


#pragma mark -
#pragma mark Memory Management

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
    [super viewDidUnload];

	self.datePicker = nil;
	self.delegate = nil;

}

- (void)dealloc {
    
    // Notify the delegate
    if ([delegate respondsToSelector:@selector(selectionDatePickerController:date:withReference:)]) {
        [delegate selectionDatePickerController:self date:self.datePicker.date withReference:reference];
    }
    
	self.datePicker = nil;
	self.delegate = nil;

    [super dealloc];
}


@end


