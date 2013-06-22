//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SwitchCell : UITableViewCell {
    UISwitch *switchControl;
}

@property (nonatomic, strong) UISwitch *switchControl;

- (id)initWithLabel:(NSString*)labelText reuseIdentifier:(NSString *)reuseIdentifier;
- (void)setEnabled:(BOOL)enabled;

@end
