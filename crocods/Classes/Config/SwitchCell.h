/*
 */

#import <UIKit/UIKit.h>

@interface SwitchCell : UITableViewCell {
    UISwitch *switchControl;
}

@property (nonatomic, retain) UISwitch *switchControl;

- (id)initWithLabel:(NSString*)labelText reuseIdentifier:(NSString *)reuseIdentifier;
- (void)setEnabled:(BOOL)enabled;

@end
