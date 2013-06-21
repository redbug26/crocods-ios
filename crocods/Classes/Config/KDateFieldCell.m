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

#import "KDateFieldCell.h"

@implementation KDateFieldCell

@synthesize dateFieldCellDelegate;
@synthesize date;
@synthesize prefix;

- (void)setValue:(id)newValue {
    if (value != newValue) {
        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        dateFormatter.dateFormat = @"dd/MM/yyyy";
        self.date=[dateFormatter dateFromString:newValue];
        [dateFormatter setDateStyle:NSDateFormatterMediumStyle];

        self.textLabel.text = [NSString stringWithFormat:@"%@: %@", self.prefix, [dateFormatter stringFromDate:self.date]];
                
        [dateFormatter release];
        [value release];
        value = newValue;
        [newValue retain];
        
        [self setNeedsLayout];

    }
}

- (NSString*)value {
    /* NSDateFormatter* dateFormatter = [[[NSDateFormatter alloc] init] autorelease];
    dateFormatter.dateFormat = @"dd/MM/yyyy";
    return [dateFormatter stringFromDate:picker.date];
     */
    return value;
}

#pragma mark -

- (id)initWithLabel:(NSString*)labelText  {
    self = [super initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    if (self) {
  
        // Initialization code
        self.selectionStyle = UITableViewCellSelectionStyleNone;
        
        self.prefix = labelText;
        
        
        self.accessoryType = UITableViewCellAccessoryDisclosureIndicator;


    }
    return self;
}

- (void)dealloc {
    [dateFieldCellDelegate release];
    [super dealloc];
}





- (void)setEnabled:(BOOL)enabled {
    self.selectionStyle = enabled ? UITableViewCellSelectionStyleBlue : UITableViewCellSelectionStyleNone;
    self.textLabel.enabled = enabled;

}




@end
