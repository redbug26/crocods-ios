//
//  AboutViewController.m
//
//  Created by Miguel Vanhove on 31/10/10.
//  Copyright 2010 Kyuran. All rights reserved.
//

#import "ConfigViewController.h"

#import "CrocoDSAppDelegate.h"

@implementation ConfigViewController


enum {
    SECTION_KEYBOARD,
    SECTION_NUMBER
};

enum {
    ROW_KEYBOARD_ICADE,
    ROW_KEYBOARD_EXTERNALKEYBOARD,
    ROW_KEYBOARD_FLING,
    ROW_KEYBOARD_NUMBER
};


- (id)init {
    if (self =[super initWithStyle:UITableViewStyleGrouped]) {
        self.title = NSLocalizedString(@"Settings", @"Settings");
        self.tabBarItem.image = [UIImage imageNamed:@"files.png"];
    }
    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    iCade = [[SwitchCell alloc] initWithLabel:@"iCade" reuseIdentifier:nil];
    
    externalKeyboard = [[SwitchCell alloc] initWithLabel:@"External Keyboard" reuseIdentifier:nil];

    fling = [[SwitchCell alloc] initWithLabel:@"Fling support" reuseIdentifier:nil];
    
    fling.switchControl.on = ([crocodsAppDelegate delegate].useFling);
    [fling.switchControl addTarget:self action:@selector(toggleFling:) forControlEvents:UIControlEventValueChanged];

    iCade.switchControl.on = ([crocodsAppDelegate delegate].useIcade);
    [iCade.switchControl addTarget:self action:@selector(toggleICade:) forControlEvents:UIControlEventValueChanged];

    externalKeyboard.switchControl.on = ([crocodsAppDelegate delegate].useExternalKeyboard);
    [externalKeyboard.switchControl addTarget:self action:@selector(toggleExternalKeyboard:) forControlEvents:UIControlEventValueChanged];

}

- (void)toggleFling:(id)sender {
    [crocodsAppDelegate delegate].useFling = !([crocodsAppDelegate delegate].useFling);
}


- (void)toggleICade:(id)sender {
    [crocodsAppDelegate delegate].useIcade = !([crocodsAppDelegate delegate].useIcade);
}


- (void)toggleExternalKeyboard:(id)sender {
    [crocodsAppDelegate delegate].useExternalKeyboard = !([crocodsAppDelegate delegate].useExternalKeyboard);
}


- (void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
    
    // Update which controls are enabled
    [self updateEnabledControls];
}

- (void)viewDidDisappear:(BOOL)animated {
    [self SaveSettings];
    [super viewDidDisappear:animated];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{
    return 36.0;
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    return 36.0;
}

- (CGFloat)tableView:(UITableView *)tableView heightForFooterInSection:(NSInteger)section {
    if (section==SECTION_KEYBOARD) {
        return 32.0;
    }
    return 0.0;
}


- (void)updateEnabledControls {
    
    NSLog(@"updateEnabledControls");
    
    
}

- (void)SaveSettings {
   
    
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	return YES;
}



- (void)toggleDropbox:(id)sender {
    [self SaveSettings];
}



-(void)viewDidAppear:(BOOL)animated{
	[super viewDidAppear:animated];
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}



- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    switch (indexPath.section) {
        case SECTION_KEYBOARD:
            switch (indexPath.row) {
                case ROW_KEYBOARD_ICADE:
                    return iCade;
                case ROW_KEYBOARD_EXTERNALKEYBOARD:
                    return externalKeyboard;
                case ROW_KEYBOARD_FLING:
                    return fling;
            }
            break;
    }   
    return nil;
    
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == SECTION_KEYBOARD) {
        if (indexPath.row == ROW_KEYBOARD_ICADE) {

        }
    }
}

- (void)selectionListViewController:(KSelectionListViewController *)controller selectedIndex:(NSInteger)selectedIndex withReference:(id<NSObject>)reference {
    NSIndexPath *indexPath = (NSIndexPath*)reference;
    
    if (indexPath.section == SECTION_KEYBOARD) {
    } 
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return SECTION_NUMBER;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    switch (section) {
        case SECTION_KEYBOARD:
            return ROW_KEYBOARD_NUMBER;

    }
    return 0;
}

- (NSString*)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
    switch (section) {
        case SECTION_KEYBOARD:
            return @"Keyboard";

    }
    return nil;
}

- (NSString*)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section {
    if (section==SECTION_KEYBOARD) {
        return [NSString stringWithFormat:@"CrocoDS v%@", [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"]];
//        return [NSString stringWithFormat:@"iCade is only available in Pro Version"];
    }
    /*
    if (section==SECTION_KEYBOARD) {
        return [NSString stringWithFormat:@"CrocoDS v%@", [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"]];
    } */
    return @"";
}


@end
