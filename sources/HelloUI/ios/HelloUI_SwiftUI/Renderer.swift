//
//  Renderer.swift
//  HelloUI_SwiftUI
//
//  Created by Pierre Renaux on 13/2/24.
//  Copyright © 2024 Bytecollider. All rights reserved.
//

import MetalKit

class Renderer: NSObject, MTKViewDelegate {
    
    let niApp: NiApp
    var parent: ContentView
    var metalDevice: MTLDevice!
    
    init(_ parent: ContentView) {
        self.parent = parent
        if let metalDevice = MTLCreateSystemDefaultDevice() {
            self.metalDevice = metalDevice
        }
          
        self.niApp = NiApp()
        self.niApp.startup(with: self.metalDevice)
        
        super.init()
    }
    
    deinit {
        self.niApp.shutdown()
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        self.niApp.resize(with: view, size: size)
    }
    
    func draw(in view: MTKView) {
        guard let drawable = view.currentDrawable else {
            return
        }
        self.niApp.draw()
    }
}
