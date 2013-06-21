#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#import "plateform.h"

typedef struct tagPOINT
{
    UInt16  x;
    UInt16  y;
} POINT;


typedef struct
{
    BOOL rotstop;                   // stop self rotation
    BOOL touchInside;               // finger tap inside of the object ? 
    BOOL scalestart;                // start to scale the obejct ?
    CGPoint pos;                    // position of the object on the screen
    CGPoint startTouchPosition;     // Start Touch Position
    CGPoint currentTouchPosition;   // Current Touch Position
    GLfloat pinchDistance;          // distance between two fingers pinch
    GLfloat pinchDistanceShown;     // distance that have shown on screen
    GLfloat rotation;               // OpenGL rotation factor of the object
    GLfloat rotspeed;               // control rotation speed of the object
    
    GLfloat scale;                  // OpenGL scale factor of the object
    GLfloat scaleTemporary;         // OpenGL scale factor of the object
    
    GLfloat centre[3];              // Centre of the scale Orthothing.
    GLfloat Orthof[6];

    
    // GLfloat 
} ObjectData;


@protocol KKDRaw_Application

@optional
- (int)CreateSurfaces:(id)i;
- (int)InitInstance:(id)i;
- (int)ExitInstance:(id)i;
- (int)ProcessNextFrame;

- (int)StylusDown:(POINT)p;
- (int)StylusUp:(POINT)p;
- (int)StylusMove:(POINT)p;

@end


/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface KKDrawView : UIView <KKDRaw_Application> {
    
    
@private

    /* The pixel dimensions of the backbuffer */
    GLint backingWidth;
    GLint backingHeight;
    
    EAGLContext *context;
    
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
    /* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
    GLuint depthRenderbuffer;
    
	BOOL displayLinkSupported;
	// Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	// CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	// The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
	// isn't available.
	id displayLink;

	int isSGX;
    
    float rx, ry;
	
    GLuint m_tex;

    CADisplayLink* m_displayLink;
    

    int borderX, borderY;
    
    int     dwWidth;
    int     dwHeight;
    int      xPitch;
    int      yPitch;
    int      xyOffset;
    int     dwSize;
    int     dwOrientation;
    int     dwFlags;

    
@public 
    bool isPaused;
    UInt16 *    pSurface;
    int width0, height0;

}

@property (nonatomic, assign) bool isPaused;

@property (nonatomic, assign) int  width0;
@property (nonatomic, assign) int  height0;
@property (nonatomic, assign) int  dwWidth;
@property (nonatomic, assign) int  dwHeight;
@property (nonatomic, assign) int borderX;
@property (nonatomic, assign) int borderY;

- (id)initWithFrame:(CGRect)frame width:(int)width height:(int)height fps:(int)fps;

- (void)blit;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

- (void)Shutdown;

@end
