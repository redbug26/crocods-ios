//
//  AboutViewController.h
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SwitchCell.h"

#import "KSelectionListViewController.h"

@interface ConfigViewController : UITableViewController <KSelectionListViewControllerDelegate> {
    
    SwitchCell *iCade;
    SwitchCell *externalKeyboard;
    SwitchCell *fling;
}

- (void)updateEnabledControls;
- (void)SaveSettings;

@end
