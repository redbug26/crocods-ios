//
//  AboutViewController.h
//  Doobs
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 TEC Hainaut. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SwitchCell.h"
#import "KDateFieldCell.h"

#import "ChoiceCell.h"
#import "SelectionListViewController.h"
#import "DatePickerController.h"

@interface ConfigViewController : UITableViewController <SelectionListViewControllerDelegate> {
    
    SwitchCell *iCade;
    SwitchCell *externalKeyboard;
    SwitchCell *fling;
}

- (void)updateEnabledControls;
- (void)SaveSettings;

@end
