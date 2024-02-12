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
#include "ShaderDefs.h"

@interface NiApp : NSObject

- (instancetype)init;
- (void)startupWithDevice:(id<MTLDevice>)device;
- (void)shutdown;
- (void)draw;
- (void)resizeWithSize:(CGSize)size;

@end

#endif /* NiApp_hpp */
