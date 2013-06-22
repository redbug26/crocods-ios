/*
 
 
 */

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>


#import "KKDrawView.h"

#ifndef DISTRIBUTION

#define CHECK_GL_ERRORS() \
do {                                                                                            	\
GLenum error = glGetError();                                                                	\
if(error != GL_NO_ERROR) {                                                                   	\
NSLog(@"OpenGL: %s [error %d]", __FUNCTION__, (int)error);					\
assert(0); \
} \
} while(false)

#else

#define CHECK_GL_ERRORS()

#endif



@implementation KKDrawView;

@synthesize width0, height0;
@synthesize dwWidth, dwHeight;
@synthesize borderX, borderY;

// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame width:(int)width height:(int)height fps:(int)fps {
    
    if ((self = [super initWithFrame:frame])) {
        
        borderX=0;
        borderY=0;
        
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        EAGLContext *testContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        
        
        isSGX = testContext ? YES : NO;
        
        // isSGX n'est seulement valable que pour l'iphone edge ou 3G ou l'ipod touch 1 ou 2 gen (donc ecran de 320x480 maxi) 
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            return nil;
        }
        
        self.multipleTouchEnabled = YES;
        
        // ---
        
		displayLinkSupported = FALSE;
		displayLink = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending) {
			displayLinkSupported = TRUE;
        }
        
        
        //---
        
        glBindTexture(GL_TEXTURE_2D, m_tex);
        
        if (isSGX) {
            // Effet blur
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {                
            // Effect pixel
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        
        
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        CHECK_GL_ERRORS();
        
        //---
        
        glBindTexture(GL_TEXTURE_2D, 0);	
        
        [self layoutSubviews];
                
        if (isSGX) {
            width0=width;
            height0=height;
            
            rx=1.0;
            ry=1.0;
            
            dwWidth = width;
            dwHeight = height;
            dwSize =  dwWidth * dwHeight * sizeof(UInt16);
            xPitch = 2;
            yPitch = 2 * dwWidth;
            pSurface = (UInt16*)malloc(dwSize);
            
        } else {            
            if (width>255) {
                width0=512;
            } else {
                width0=256;
            }
            if (height>255) {
                height0=512;
            } else {
                height0=256;
            }
            
            rx=(float)(width)/((float)width0);
            ry=(float)(height)/((float)height0);
            
            dwWidth = width;
            dwHeight = height;
            dwSize =  width0 * height0 * sizeof(UInt16);
            xPitch = 2;
            yPitch = 2 * width0;
            pSurface = (UInt16*)malloc(dwSize);
        }
        
        
        if ([self respondsToSelector:@selector(CreateSurfaces:)]) {
            [self CreateSurfaces:self];
        }
        
        
        // Dans le viewWillAppear (de préférence)
        
        
        m_displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(doFrame)];
        m_displayLink.frameInterval = (60/fps); // 60/frameInterval fps
        [m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        

        
    }
    
    return self;
}

- (void)setIsPaused:(bool)isPaused0 {
    m_displayLink.paused=isPaused0;
}

- (bool)isPaused {
    return m_displayLink.isPaused;
}

- (void)doFrame {
    [self blit];    
}


#define RANDOM_INT(__MIN__, __MAX__) ((__MIN__) + random() % ((__MAX__+1) - (__MIN__)))


# pragma mark Init



# pragma mark Draw


- (void)blit {
    
    UInt16 *m_pixelData=pSurface;
    
    [EAGLContext setCurrentContext:context];
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    
    
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glOrthof(dwWidth, 0, 0, dwHeight, 10.0f, -10.0f);

  //  glOrthof(dwWidth-borderX, 0, 0, dwHeight-borderY, 10.0f, -10.0f);
    glMatrixMode(GL_MODELVIEW);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
    
    //    glEnable(GL_BLEND);
    //  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    
    const GLfloat quadVertices[] = { 0,0,0,  0,dwHeight,0, dwWidth,0,0,  dwWidth,dwHeight,0};
    
    
    rx=(float)(dwWidth - (borderX*2))/((float)width0);
    ry=(float)(dwHeight - (borderY*2))/((float)height0);
    
    /*
    const GLfloat quadVertices[] = {
        -borderX,-borderY,0,  -borderX,dwHeight+borderY,0, dwWidth+borderX,-borderY,0,  dwWidth+borderX,dwHeight+borderY,0
    };
    */
    
    const GLfloat quadTexCoords[] = {
        rx, ry, rx, 0,  0, ry,  0, 0 
    };	
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glVertexPointer(3, GL_FLOAT, 0, quadVertices);
    glTexCoordPointer(2, GL_FLOAT, 0, quadTexCoords);
    
    // draw text INSTR
    // glBindTexture(GL_TEXTURE_2D, texture[3]);
    
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 width0, height0,0,
                 GL_RGB  , 
                 GL_UNSIGNED_SHORT_5_6_5,
                 m_pixelData); 
    
    CHECK_GL_ERRORS();
    
    
    glPushMatrix();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glPopMatrix();
    
    glBindTexture(GL_TEXTURE_2D, 0);    
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
        
    
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}


- (void)layoutSubviews {
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];    
}

- (BOOL)createFramebuffer {
    
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    // if (USE_DEPTH_BUFFER) {
    
    glGenRenderbuffersOES(1, &depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    
    // }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	
	glViewport(0, 0, backingWidth, backingHeight);
    
    CHECK_GL_ERRORS();
    
    
    return YES;
}


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}

- (void)Shutdown {
    NSLog(@"shutdown");
        
    [m_displayLink invalidate]; // Arrete l'affichage et provoque un retaincounte en moins de self
}

- (void)dealloc {
    
    [self destroyFramebuffer];
    
	// release textures
	glDeleteTextures(1, &m_tex);
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
	
    
    NSLog(@"******** dealloc kkdrawview");
}





@end