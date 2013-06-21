//
//  TDDatePickerController.h
//
//  Created by Nathan  Reed on 30/09/10.
//  Copyright 2010 Nathan Reed. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol DatePickerControllerDelegate;

@interface DatePickerController : UIViewController {
	id delegate;
    UIDatePicker* datePicker;
    id<NSObject> reference;

}

@property (nonatomic, retain) id delegate;
@property (nonatomic, retain) UIDatePicker* datePicker;
@property (nonatomic, retain) id<NSObject> reference;


@end


@protocol DatePickerControllerDelegate <NSObject>
- (void)selectionDatePickerController:(DatePickerController*)controller date:(NSDate*)date withReference:(id<NSObject>)reference;
@end