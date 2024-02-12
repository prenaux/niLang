//
//  NiApp.cpp
//  HelloUI_SwiftUI
//
//  Created by Pierre Renaux on 13/2/24.
//  Copyright © 2024 Bytecollider. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NiApp.h"
#include <niLang/Platforms/OSX/osxMetal.h>

using namespace ni;

@interface NiApp() {}
@property (nonatomic, retain, nullable) id <MTLDevice> device;
@property (nonatomic, assign, readwrite) app::AppContext* appContext;
@end

@implementation NiApp

- (instancetype)init {
    if (self = [super init]) {
        // Initialize context or any other setup here
        self.appContext = nullptr;
    }
    return self;
}

- (void)startupWithDevice:(id<MTLDevice>)device {
    NSLog(@"NiApp::Startup called with device: %@, %@", [device name], [device description]);
    niAppLib_SetBuildText();
    
    self.device = device;

    // Load the configuration
    self.appContext = niNew app::AppContext();
    self.appContext->_config.drawFPS = 2;
    self.appContext->_config.swapInterval = 0;
    self.appContext->_config.graphicsDriver = _H("Metal");
}

- (void)shutdown {
    NSLog(@"NiApp::Shutdown called");
    if (self.appContext) {
        auto appCtx = astl::as_non_null(self.appContext);
        app::AppShutdown(appCtx);
    }
    
    // We need to invalidate the concurrent queue to make sure everying works when we reload
    ni::GetConcurrent()->Invalidate();
}

- (void)draw {
    auto appCtx = astl::as_non_null(self.appContext);
    //NSLog(@"NiApp::Draw called");
    if (appCtx->_window.IsOK()) {
        app::AppUpdate(appCtx);
        app::AppRender(appCtx);
    }
}

- (void)resizeWithView:(MTKView*)mtkView size:(CGSize)size {
    NSLog(@"NiApp::Resize called with size: %g, %g", size.width, size.height);
    auto appCtx = astl::as_non_null(self.appContext);
    if (!appCtx->_window.IsOK()) {
        // Metal setup
        ni::osxMetalSetDevice((__bridge void*)self.device);
        ni::Ptr<ni::iOSGraphicsAPI> ptrAPI = ni::osxMetalCreateAPIForMTKView(ni::osxMetalGetDevice(),(__bridge void*)mtkView);
        
        // Create iOSWindow
        ni::Ptr<ni::iOSWindow> ptrWindow = ni::CreateGenericWindow();
        ptrWindow->AttachGraphicsAPI(ptrAPI);
        ptrWindow->SetRect(ni::sRecti(0,0,(tI32)size.width,(tI32)size.height));
        niLog(Info,niFmt("Initial window rect: %s", ptrWindow->GetRect()));

        // App startup
        auto appCtx = astl::as_non_null(self.appContext);
        app::AppStartup(appCtx, ptrWindow, ni::Runnable([appCtx]() -> Var {
          // TODO: appCtx->_uiContext->GetRootWidget()->AddSink(...);
          return ni::eTrue;
        }),NULL);
    }
    app::AppGenericResize(appCtx, size.width, size.height);
}

@end
