//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import "KSelectionListViewController.h"

@implementation KSelectionListViewController

@synthesize items;
@synthesize selectedIndex;
@synthesize delegate;
@synthesize reference;
@synthesize subtitle;


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [items count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier];
    }
    
    // Configure the cell
    cell.textLabel.text = [items objectAtIndex:indexPath.row];
    
    if (indexPath.row == selectedIndex) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        cell.textLabel.textColor = [UIColor colorWithRed:0.243 green:0.306 blue:0.435 alpha:1];
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }

    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell;
    
    if (indexPath.row != selectedIndex) {
        // Remove the checkmark from the current selection
        cell = [tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:selectedIndex inSection:0]];
        cell.accessoryType = UITableViewCellAccessoryNone;
        cell.textLabel.textColor = [UIColor blackColor];
        
        // Add the checkmark to the new selection
        cell = [tableView cellForRowAtIndexPath: indexPath]; 
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        cell.textLabel.textColor = [UIColor colorWithRed:0.243 green:0.306 blue:0.435 alpha:1];
        
        selectedIndex = indexPath.row;
        
        // Notify the delegate
        if ([delegate respondsToSelector:@selector(selectionListViewController:selectedIndex:withReference:)]) {
            [delegate selectionListViewController:self selectedIndex:selectedIndex withReference:reference];
        }
    }
    
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

- (NSString*)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    if (subtitle != nil) {
        return subtitle;
    }
    return nil;
}


@end
