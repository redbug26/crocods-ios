//
//  main.m
//  biopocket
//
//  Created by Miguel Vanhove on 3/08/11.
//  Copyright 2011 TEC Hainaut. All rights reserved.
//

#import <UIKit/UIKit.h>

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	int retVal = UIApplicationMain(argc, argv, @"crocodsApp", @"crocodsAppDelegate");
	[pool release];
	return retVal;
}
