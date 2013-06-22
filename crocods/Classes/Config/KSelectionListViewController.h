//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol KSelectionListViewControllerDelegate;

@interface KSelectionListViewController : UITableViewController {
    NSArray *items;
    NSString *subtitle;
    NSInteger selectedIndex;
    id<KSelectionListViewControllerDelegate> delegate;
    id<NSObject> reference;
}

@property (nonatomic, strong) NSArray *items;
@property (nonatomic) NSInteger selectedIndex;
@property (nonatomic, strong) id<KSelectionListViewControllerDelegate> delegate;
@property (nonatomic, strong) id<NSObject> reference;
@property (nonatomic, strong) NSString *subtitle;
@end

@protocol KSelectionListViewControllerDelegate <NSObject>
- (void)selectionListViewController:(KSelectionListViewController*)controller selectedIndex:(NSInteger)selectedIndex withReference:(id<NSObject>)reference;
@end
