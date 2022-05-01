// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#import "AppDelegate.h"
#import "niUIView.h"
#import <AVFoundation/AVFoundation.h>
#import <QuartzCore/QuartzCore.h>

@implementation AppDelegate

- (void)dealloc
{
  [_window release];
  [super dealloc];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  // Let iTunes play Music while our app is running
  {
    NSError *error;
    BOOL success = [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:&error];
    if (!success) {
      //Handle error
      NSLog(@"%@", [error localizedDescription]);
    } else {
      // Yay! It worked!
    }
  }

  self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];

  // Initialize the niUIView
  EAGLContext * context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  niUIView* view = [[niUIView alloc] initWithFrame: [[UIScreen mainScreen] bounds] context:context];

  // Add the niUIView to the window
  [self.window addSubview:view];

  // Set the niUIView to always fit in the parent view
  [view setAutoresizingMask:(UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight)];
  [self.window setAutoresizesSubviews:YES];

  // Enable auto refresh
  view.enableSetNeedsDisplay = NO;
  CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
  [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  // displayLink.frameInterval = 2; // set to 30fps

  // Set the root view controller
  UIViewController* viewController;
  if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
    viewController = [[[UIViewController alloc] initWithNibName:@"ViewController_iPhone" bundle:nil] autorelease];
  } else {
    viewController = [[[UIViewController alloc] initWithNibName:@"ViewController_iPad" bundle:nil] autorelease];
  }
  self.window.rootViewController = viewController;

  // Get going...
  [self.window makeKeyAndVisible];

  return YES;
}

- (void)render:(CADisplayLink*)displayLink {
  GLKView * view = [self.window.subviews objectAtIndex:0];
  [view display];
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event{
  GLKView * view = [self.window.subviews objectAtIndex:0];
  [view touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event{
  GLKView * view = [self.window.subviews objectAtIndex:0];
  [view touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event{
  GLKView * view = [self.window.subviews objectAtIndex:0];
  [view touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event{
  GLKView * view = [self.window.subviews objectAtIndex:0];
  [view touchesCancelled:touches withEvent:event];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
  // Sent when the application is about to move from active to inactive
  // state. This can occur for certain types of temporary interruptions (such
  // as an incoming phone call or SMS message) or when the user quits the
  // application and it begins the transition to the background state.  Use
  // this method to pause ongoing tasks, disable timers, and throttle down
  // OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
  // Use this method to release shared resources, save user data, invalidate
  // timers, and store enough application state information to restore your
  // application to its current state in case it is terminated later.  If your
  // application supports background execution, this method is called instead
  // of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
  // Called as part of the transition from the background to the inactive
  // state; here you can undo many of the changes made on entering the
  // background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
  // Restart any tasks that were paused (or not yet started) while the
  // application was inactive. If the application was previously in the
  // background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
  // Called when the application is about to terminate. Save data if
  // appropriate. See also applicationDidEnterBackground:.
}

@end
