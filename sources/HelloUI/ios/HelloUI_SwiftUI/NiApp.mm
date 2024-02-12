//
//  NiApp.cpp
//  HelloUI_SwiftUI
//
//  Created by Pierre Renaux on 13/2/24.
//  Copyright © 2024 Bytecollider. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NiApp.h"

@implementation NiApp

- (instancetype)init {
    if (self = [super init]) {
        // Initialize context or any other setup here
    }
    return self;
}

- (void)startupWithDevice:(id<MTLDevice>)device {
    NSLog(@"NiApp::Startup called with device: %@", [device name]);
}

- (void)shutdown {
    NSLog(@"NiApp::Shutdown called");
}

- (void)draw {
    NSLog(@"NiApp::Draw called");
}

- (void)resizeWithSize:(CGSize)size {
    NSLog(@"NiApp::Resize called with size: %g, %g", size.width, size.height);
}

@end
