//
//  MyCatalog.m
//  crocods
//
//  Created by Miguel Vanhove on 23/08/11.
//  Copyright 2011 Kyuran. All rights reserved.
//

#import "MyCatalog.h"

#import "nds.h"
#import "upd.h"
#import "autotype.h"
#import "snapshot.h"
#import "plateform.h"

@interface MyCatalog (PrivateMethod)
-(void)filterEntriesForSearchText:(NSString*)searchText;
@end

@implementation MyCatalog

@synthesize _entries;
@synthesize _qualifiedEntries;
@synthesize searchDisplayController;
@synthesize delegate;

- (id)init {
    if (self =[super initWithStyle:UITableViewStylePlain]) {
        self.title = NSLocalizedString(@"Files", @"Files");
        self.tabBarItem.image = [UIImage imageNamed:@"files.png"];
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refreshData:) name:@"EntryUpdated" object:nil];	
    
	UISearchBar * searchBar = [[UISearchBar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
	searchBar.delegate = self;
	self.tableView.tableHeaderView = searchBar;
	
	searchDisplayController = [[UISearchDisplayController alloc] initWithSearchBar:searchBar contentsController:self];	
	[searchDisplayController setDelegate:self];
	[searchDisplayController setSearchResultsDelegate:self];
	[searchDisplayController setSearchResultsDataSource:self];
	
	
	self.tableView.scrollEnabled = YES;
    
    
    self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancel:)];
   
    UIView *loadingParam = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 100)];
    // contentView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    loadingParam.backgroundColor = [UIColor scrollViewTexturedBackgroundColor];

    // ----------------------------;
    // UILabel -> label1;
    // ----------------------------;
    
    UILabel *label1 = [[UILabel alloc] initWithFrame:CGRectMake(16, 16, 170, 25)];
    [loadingParam addSubview:label1];
    label1.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
    label1.text = @"Auto start";
    label1.backgroundColor = [UIColor clearColor];
    
    
    // ----------------------------;
    // UISwitch -> switch1;
    // ----------------------------;
    
    UISwitch *switch1 = [[UISwitch alloc] init];
    [switch1 sizeToFit];
    CGRect switch1Rect = switch1.frame;
    switch1Rect.origin.x = 186;
    switch1Rect.origin.y =  15;
    switch1.frame = switch1Rect;
    [loadingParam addSubview:switch1];
    [switch1 addTarget: self action: @selector(autoStartFlip:) forControlEvents: UIControlEventValueChanged];
    switch1.alpha = 1.0;
    switch1.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
    switch1.on = YES;
    switch1.enabled = YES;
    switch1.onTintColor = [UIColor colorWithRed:0.0 green:0.5 blue:0.92 alpha:1.0];
    
    
    // ----------------------------;
    // UILabel -> label2;
    // ----------------------------;
    
    UILabel *label2 = [[UILabel alloc] initWithFrame:CGRectMake(16, 49, 170, 25)];
    [loadingParam addSubview:label2];
    label2.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
    label2.text = @"Reboot";
    label2.backgroundColor = [UIColor clearColor];
    
    
    // ----------------------------;
    // UISwitch -> switch2;
    // ----------------------------;
    
    UISwitch *switch2 = [[UISwitch alloc] init];
    [switch2 sizeToFit];
    CGRect switch2Rect = switch2.frame;
    switch2Rect.origin.x = 186;
    switch2Rect.origin.y =  48;
    switch2.frame = switch2Rect;
    [loadingParam addSubview:switch2];
    [switch2 addTarget: self action: @selector(rebootWhenStartFlip:) forControlEvents: UIControlEventValueChanged];

    switch2.alpha = 1.0;
    switch2.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
    switch2.on = YES;
    switch2.enabled = YES;
    switch2.onTintColor = [UIColor colorWithRed:0.0 green:0.5 blue:0.92 alpha:1.0];
    
    autoStart=true;
    rebootWhenStart=true;
    
    
    self.tableView.tableFooterView = loadingParam;
    
    self.title=@"Select file";
}

- (void)autoStartFlip:(id)sender {
    autoStart=!autoStart;
}

- (void)rebootWhenStartFlip:(id)sender {
    rebootWhenStart=!rebootWhenStart;
}


-(void)cancel:(id)sender {
    [self.delegate performSelector:@selector(dismissSettingsView)];

 //   [delegate pagelistPickerControllerDidCancel:self];
}

- (void)viewWillAppear:(BOOL)animated {
    NSIndexPath *selectedIndexPath = [self.tableView indexPathForSelectedRow];
    
    // Reload the cell in case the title was changed by the entry view
    if (selectedIndexPath != nil) {
        [self.tableView reloadRowsAtIndexPaths:[NSArray arrayWithObject:selectedIndexPath] withRowAnimation:UITableViewRowAnimationNone];
        [self.tableView selectRowAtIndexPath:selectedIndexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
    }
    
    searchDisplayController.searchBar.placeholder = [NSString stringWithFormat:@"Search %@", self.title];
    
    CGFloat searchBarHeight = searchDisplayController.searchBar.frame.size.height;
    if (self.tableView.contentOffset.y < searchBarHeight) {
        self.tableView.contentOffset = CGPointMake(0, searchBarHeight);
    }
    
    [self refreshData:nil];

    
    [super viewWillAppear:animated];
}



- (void)dealloc {
    
    NSLog(@"dealoc pagelistcontroller");
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)viewDidUnload {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	self.searchDisplayController = nil;
}


// Ensure that the view controller supports rotation and that the split view can therefore show in both portrait and landscape.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}


#pragma mark Table view methods (index)

- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView {
    return indexLetters;
}

- (void)RebuildIndex:(NSMutableArray *)entries0
{
    NSLog(@"rebuild index");
    
    _currentEntries=entries0;
    
    //---create the index---
    indexLetters = [[NSMutableArray alloc] init];
    indexLettersAll = [[NSMutableArray alloc] init];
    
    NSLog(@"Rebuild index: %d", [entries0 count]);
    
    if ([entries0 count]==0) return;
    
    for (int i=0; i<[entries0 count]; i++){
        unsigned short alphabet;
        //---get the first char of each state---
        
        NSString *file = [entries0 objectAtIndex:i];
        
        if ([file length]>0) {
            alphabet = [file characterAtIndex:0];
            if ((alphabet>='a') && (alphabet<='z')) {
                alphabet+='A'-'a';
            } else if ((alphabet<'A') || (alphabet>'Z')) {
                alphabet='-';
            }
        } else {
            alphabet='-';
        }
        NSString *uniChar = [NSString stringWithFormat:@"%C", alphabet];
        
        //---add each letter to the index array---
        if (![indexLetters containsObject:uniChar])
        {            
            [indexLetters addObject:uniChar];
        }     
        [indexLettersAll addObject:uniChar];
    }
}

/*
 
 - (NSInteger)tableView:(UITableView *)tableView sectionForSectionIndexTitle:(NSString *)title atIndex:(NSInteger)index {
 // Return the index for the given section title
 return [indexLetters indexOfObject:title];
 NSLog(@"%@: %d", title, index);
 return 0;
 }
 */

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    return [indexLetters objectAtIndex:section];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return [indexLetters count];
}


// Customize the number of rows in the table view.
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    //---get the letter in each section; e.g., A, B, C, etc.---
    NSString *alphabet = [indexLetters objectAtIndex:section];
    
    //---get all states beginning with the letter---
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF beginswith[c] %@", alphabet];
    
    NSArray *states = [indexLettersAll filteredArrayUsingPredicate:predicate];
    return [states count];    
	
    //---return the number of states beginning with the letter---
    
    
}

#pragma mark Table view methods


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    int row;
    
    if ([indexLetters count]==0) {
        return nil;
    }
    
    static NSString *CellIdentifier = @"Cell";  
    
    NSString *alphabet = [indexLetters objectAtIndex:indexPath.section];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF < %@", alphabet];
    NSArray *states = [indexLettersAll filteredArrayUsingPredicate:predicate];
    row = [states count] + indexPath.row;
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:CellIdentifier];
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	}
    
    NSString *file=[_currentEntries objectAtIndex:row];
    
	cell.textLabel.text = file;
    //	cell.detailTextLabel.text = [[_currentEntries objectAtIndex:row] getUserName];	
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    int row;
    
    if ([indexLetters count]==0) {
        return;
    }
    
    NSString *alphabet = [indexLetters objectAtIndex:indexPath.section];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF < %@", alphabet];
    NSArray *states = [indexLettersAll filteredArrayUsingPredicate:predicate];
    row = [states count] + indexPath.row;
    
    
    NSString *fileName=[_currentEntries objectAtIndex:row];
   
    NSString* extension = [[fileName pathExtension] lowercaseString];
    if ([extension isEqualToString:@"sna"]) {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *fullFile = [(NSString *)[paths objectAtIndex:0] stringByAppendingPathComponent:fileName];
        
        NSData *rom=[[NSData alloc] initWithContentsOfFile:fullFile];     
        LireSnapshotMem((u8*)[rom bytes]);
    }
    if ([extension isEqualToString:@"dsk"]) {
         char autofile[256];
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *fullFile = [(NSString *)[paths objectAtIndex:0] stringByAppendingPathComponent:fileName];

         NSData *dsk=[[NSData alloc] initWithContentsOfFile:fullFile]; 
         LireDiskMem((u8*)[dsk bytes], [dsk length], autofile);
        
         if ((autoStart) && (autofile[0]!=0)) {
         char buffer[256];
         sprintf(buffer,"run\"%s\n", autofile);
         AutoType_SetString(buffer, rebootWhenStart);
         }
    }
    [self.delegate performSelector:@selector(dismissSettingsView)];
}


#pragma mark -
#pragma mark Content Filtering

- (void)filterEntriesForSearchText:(NSString*)searchText{
	[_qualifiedEntries removeAllObjects];
	
	for (NSString *entry in _entries){
		NSRange range = [entry rangeOfString:searchText options:(NSCaseInsensitiveSearch|NSDiacriticInsensitiveSearch)];
		if([entry length]&&range.location!=NSNotFound){
			[_qualifiedEntries addObject:entry];
		}
    }
    
    [self RebuildIndex:_qualifiedEntries];
	
	NSLog(@"%d", [_qualifiedEntries count]);
}



#pragma mark -
#pragma mark UISearchDisplayController Delegate Methods
-(void)searchDisplayControllerDidBeginSearch:(UISearchDisplayController *)controller{
    NSLog(@"searchDisplayControllerDidBeginSearch");
    
}

- (BOOL)searchDisplayController:(UISearchDisplayController *)controller shouldReloadTableForSearchString:(NSString *)searchString{
    NSLog(@"shouldReloadTableForSearchString");
    [self filterEntriesForSearchText:searchString];
    return YES;
}


- (BOOL)searchDisplayController:(UISearchDisplayController *)controller shouldReloadTableForSearchScope:(NSInteger)searchOption{
    NSLog(@"shouldReloadTableForSearchScope");
    [self filterEntriesForSearchText:[self.searchDisplayController.searchBar text]];
    return YES;
}

-(void)searchDisplayControllerDidEndSearch:(UISearchDisplayController *)controller{
    NSLog(@"searchDisplayControllerDidEndSearch");
    //    [self RebuildIndex:_entries];
    _currentEntries=_entries;
    [self refreshData:nil];
}

NSInteger entrySort(NSString *e1, NSString * e2, void *context){
	return [e1 caseInsensitiveCompare:e2];
}

-(void)refreshData:(NSNotification *)notification {	
	NSLog(@"refresh");
    
	if(!_entries){
		_entries = [[NSMutableArray alloc]initWithCapacity:24];
	}else{
		[_entries removeAllObjects];
	}
	
	if(!_qualifiedEntries){
		_qualifiedEntries = [[NSMutableArray alloc]initWithCapacity:24];
	}else{
		[_qualifiedEntries removeAllObjects];
	}
	
	NSMutableArray * entries = [[NSMutableArray alloc]initWithCapacity:24];
	
    NSArray* validExtensions = [NSArray arrayWithObjects:@"sna", @"dsk", nil];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *DATA_DIR = [(NSString *)[paths objectAtIndex:0] stringByAppendingPathComponent:@""];
    
    NSFileManager * fileManager = [NSFileManager defaultManager];
    NSArray * contents = [fileManager contentsOfDirectoryAtPath:DATA_DIR error:nil];
    for(NSString * fileName in contents) {
        NSString* extension = [[fileName pathExtension] lowercaseString];
        
        if ((![fileName hasPrefix:@"."]) && ([validExtensions indexOfObject:extension] != NSNotFound)) {
            [entries addObject:fileName];
        }
    }

    
	[self._entries addObjectsFromArray:[entries sortedArrayUsingFunction:entrySort context:nil]];	
	
    if (_currentEntries==_qualifiedEntries) { // Lorsque notification = "EntryUpdated", ca peut changer
        [self filterEntriesForSearchText:[self.searchDisplayController.searchBar text]];
    } else {
        [self RebuildIndex:_entries];
    } 
    
	[self.tableView reloadData];
}
@end
