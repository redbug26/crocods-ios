//
//  MyCatalog.h
//  crocods
//
//  Created by Miguel Vanhove on 23/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

@interface MyCatalog : UITableViewController <UISearchDisplayDelegate, UISearchBarDelegate> {
    NSMutableArray *kfiles;
    
    NSMutableArray *indexLetters;
    NSMutableArray *indexLettersAll;
    
    NSMutableArray * _currentEntries;
    
	NSMutableArray * _entries;
	NSMutableArray * _qualifiedEntries;
    
	UISearchDisplayController *searchDisplayController;
    
    UIResponder *delegate;
    
    bool autoStart, rebootWhenStart;
}


@property (nonatomic, strong) NSMutableArray * _entries;
@property (nonatomic, strong) NSMutableArray * _qualifiedEntries;
@property (nonatomic, strong) UISearchDisplayController *searchDisplayController;
@property (nonatomic, strong) UIResponder *delegate;

-(void)refreshData:(NSNotification *)notification;
-(void)RebuildIndex:(NSMutableArray *)_entries0;

- (void)filterEntriesForSearchText:(NSString*)searchText;

NSInteger entrySort(NSString *e1, NSString * e2, void *context);

@end
