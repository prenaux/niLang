// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface niUIView : GLKView

- (instancetype)initWithFrame:(CGRect)frame context: (EAGLContext*)context;

@end
