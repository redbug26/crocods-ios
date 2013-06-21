#ifndef st_oglview_h_
#define st_oglview_h_

#import "KKDrawView.h"

#import <UIKit/UIKit.h>
#import <UIKit/UIView.h>

#import "iCadeReaderView.h"

@class crocodsViewController;

struct moment {
	int temps;
	int gramme;
	int miam;
};

@interface OGLView : KKDrawView <iCadeEventDelegate>
{
    
    crocodsViewController *parent;
    
    int bx,by;
}

@property (nonatomic, assign) crocodsViewController *parent;


- (int)CreateSurfaces:(id)i;

- (void)Go;

@end


#endif
