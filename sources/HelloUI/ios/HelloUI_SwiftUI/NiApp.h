//
//  NiApp.hpp
//  HelloUI_SwiftUI
//
//  Created by Pierre Renaux on 13/2/24.
//  Copyright © 2024 Bytecollider. All rights reserved.
//

#ifndef NiApp_hpp
#define NiApp_hpp

#import <Metal/Metal.h>
#import <MetalKit/MTKView.h>
#include "ShaderDefs.h"
#ifdef __cplusplus
#import <niAppLib.h>
#endif

@interface NiApp : NSObject

- (instancetype)init;
- (void)startupWithDevice:(id<MTLDevice>)device;
- (void)shutdown;
- (void)draw;
- (void)resizeWithView:(MTKView*)mtkView size:(CGSize)size;

@end

#endif /* NiApp_hpp */
