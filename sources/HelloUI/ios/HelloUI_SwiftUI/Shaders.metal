//
//  Shaders.metal
//  HelloUI_SwiftUI
//
//  Created by Pierre Renaux on 13/2/24.
//  Copyright © 2024 Bytecollider. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

#include "ShaderDefs.h"

struct Fragment {
    float4 position [[position]];
    float4 color;
};

vertex Fragment vertexShader(const device Vertex *vertexArray [[buffer(0)]], unsigned int vid [[vertex_id]]) {
    Vertex input = vertexArray[vid];
    
    Fragment output;
    output.position = float4(input.position.x, input.position.y, 0, 1);
    output.color = input.color;
    
    return output;
}

fragment float4 fragmentShader(Fragment input [[stage_in]]) {
    return input.color;
}
