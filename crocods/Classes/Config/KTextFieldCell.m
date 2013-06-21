/*
 * Copyright 2011 Jason Rush and John Flanagan. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "KTextFieldCell.h"
#import <UIKit/UIPasteboard.h>

@implementation KTextFieldCell

@synthesize textField;
@synthesize textFieldCellDelegate;

- (id)initWithStyle:(UITableViewCellStyle)style reuseIdentifier:(NSString *)reuseIdentifier {
    self = [super initWithStyle:style reuseIdentifier:reuseIdentifier];
    if (self) {
        // Initialization code
        self.selectionStyle = UITableViewCellSelectionStyleNone;
        
        textField = [[UITextField alloc] init];
        textField.delegate = self;
        textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
    //    textField.textColor = [UIColor colorWithRed:.285 green:.376 blue:.541 alpha:1];
      //  textField.font = [UIFont systemFontOfSize:16];
        textField.returnKeyType = UIReturnKeyNext;
        [self addSubview:textField];
    }
    return self;
}

- (void)dealloc {
    [textField release];
    [textFieldCellDelegate release];
    [super dealloc];
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    CGRect rect = self.contentView.frame;
    
    textField.frame = CGRectMake(rect.origin.x + 110, rect.origin.y, rect.size.width - 120, rect.size.height);
}




- (void)textFieldDidBeginEditing:(UITextField *)field {
    // Scroll to the top
    UITableView *tableView = (UITableView*)self.superview;
    [tableView setContentOffset:CGPointMake(0.0, 0.0) animated:YES];

}

- (void)textFieldDidEndEditing:(UITextField *)field {
    // Ensure our gesture recgonizer is on top


}

- (BOOL)textFieldShouldReturn:(UITextField *)field {
    if ([textFieldCellDelegate respondsToSelector:@selector(textFieldCellWillReturn:)]) {
        [textFieldCellDelegate textFieldCellWillReturn:self];
    }
    
    return NO;
}

- (void)setEnabled:(BOOL)enabled {
    self.textLabel.enabled = enabled;
    self.textField.enabled = enabled;
    self.textField.textColor = enabled ? [UIColor colorWithRed:.285 green:.376 blue:.541 alpha:1] : [UIColor colorWithRed:.6 green:.6 blue:.6 alpha:1];
}

@end
