// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#import "niUIView.h"
#include <niAppLib.h>

#define TRACE_INPUT(FMT) //niDebugFmt(FMT)

app::AppContext gAppContext;
ni::Var OnAppStarted();

ni::tF32 kSettings_ScaleFactor = 1.0f;

static bool _GetSettingsBool(NSString* aString) {
  return [[NSUserDefaults standardUserDefaults] boolForKey:aString];
}

static void _InitializeSettings() {
  NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];
  [defs synchronize];

  NSString *settingsBundle = [[NSBundle mainBundle] pathForResource:@"Settings" ofType:@"bundle"];
  if (!settingsBundle) {
    NSLog(@"Could not find Settings.bundle");
    return;
  }

  NSDictionary *settings = [NSDictionary dictionaryWithContentsOfFile:[settingsBundle stringByAppendingPathComponent:@"Root.plist"]];
  NSArray *preferences = [settings objectForKey:@"PreferenceSpecifiers"];
  NSMutableDictionary *defaultsToRegister = [[NSMutableDictionary alloc] initWithCapacity:[preferences count]];

  for (NSDictionary *prefSpecification in preferences)
  {
    NSString *key = [prefSpecification objectForKey:@"Key"];
    if (key)
    {
      // Check if value is registered or not in userDefaults
      id currentObject = [defs objectForKey:key];
      if (currentObject == nil)
      {
        // Not registered: set value from Settings.bundle
        id objectToSet = [prefSpecification objectForKey:@"DefaultValue"];
        [defaultsToRegister setObject:objectToSet forKey:key];
        NSLog(@"Setting object %@ for key %@", objectToSet, key);
      }
      else
      {
        // Already registered
        NSLog(@"Key %@ is already registered with Value: %@).", key, currentObject);
      }
    }
  }

  [defs registerDefaults:defaultsToRegister];
  [defaultsToRegister release];
  [defs synchronize];
}

@interface niUIView() {
}
- (void)setupGL;
- (void)tearDownGL;
@end

@implementation niUIView

- (void)dealloc
{
  [self tearDownGL];

  if ([EAGLContext currentContext] == self.context) {
    [EAGLContext setCurrentContext:nil];
  }

  [super dealloc];
}

- (instancetype)initWithFrame:(CGRect)frame context: (EAGLContext*)context
{
  self = [super initWithFrame:frame context: context];
  if (!self) {
    NSLog(@"Failed to create self !");
    return self;
  }
  if (!self.context) {
    NSLog(@"Failed to create ES context");
  }

  // Load the configuration
  _InitializeSettings();
  if (_GetSettingsBool(@"SettingsDrawProfiler")) {
    gAppContext._config.drawFPS = 2;
  }
  else if (_GetSettingsBool(@"SettingsDrawFPS")) {
    gAppContext._config.drawFPS = 1;
  }
  else {
    gAppContext._config.drawFPS = 0;
  }
  const bool bSettingRetina = _GetSettingsBool(@"SettingsRetina");
  const bool bSettingMSAA4X = _GetSettingsBool(@"SettingsMSAA4X");
  const bool bSetting30FPS = _GetSettingsBool(@"Settings30FPS");
  const bool bSettingMultiTouch = _GetSettingsBool(@"SettingsMultiTouch");

  // Setup the view
  self.multipleTouchEnabled = bSettingMultiTouch;
  self.drawableDepthFormat = GLKViewDrawableDepthFormat24;
  self.drawableMultisample = bSettingMSAA4X ? GLKViewDrawableMultisample4X : GLKViewDrawableMultisampleNone;
  if (!bSettingRetina) {
    self.contentScaleFactor = 1.0;
  }
  kSettings_ScaleFactor = self.contentScaleFactor;

  [self setupGL];

  return self;
}

- (void)setupGL
{
  [EAGLContext setCurrentContext:self.context];
  niAppLib_SetBuildText();
  app::AppGenericStartup(&gAppContext, "HelloUI", 400, 300, ni::Runnable<ni::tpfnRunnable>(OnAppStarted), NULL);
}

- (void)tearDownGL
{
  app::AppShutdown(&gAppContext);
  [EAGLContext setCurrentContext:self.context];
}

- (void)update
{
}

- (void)drawRect:(CGRect)rect
{
  // niLog(Info, niFmt("... drawRect: %s, %s", rect.size.width, rect.size.height));

  // Scale to the actual device resolution here.
  // for retinal display scale will be 2;
  rect.size.width *= kSettings_ScaleFactor;
  rect.size.height *= kSettings_ScaleFactor;

  app::AppGenericResize(&gAppContext, rect.size.width, rect.size.height);
  app::AppUpdate(&gAppContext);
  app::AppRender(&gAppContext);

  // We must disable the scissor test so that the default multisampling code
  // in GLKView works correctly.
  {
    glDisable(GL_SCISSOR_TEST);
  }
}

const int MAX_TOUCHES = 10;
static ni::sVec2f  _lastTouchPosition[MAX_TOUCHES] = {0};
static ni::tIntPtr _touches[MAX_TOUCHES] = {0};

static int _GetFingerIdFromTouch(UITouch* touch) {
  for (int i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i] == (ni::tIntPtr)touch) {
      return i;
    }
  }
  return -1;
}
static int _AddNewTouch(UITouch* touch) {
  for (int i = 0; i < MAX_TOUCHES; ++i) {
    if (!_touches[i]) {
      _touches[i] = (ni::tIntPtr)touch;
      return i;
    }
  }
  return -1;
}
static int _RemoveTouch(const int fingerId) {
  niAssert(fingerId < MAX_TOUCHES);
  _touches[fingerId] = 0;
  return -1;
}
static int _CountNumTouches() {
  int count = 0;
  for (int i = 0; i < MAX_TOUCHES; ++i) {
    if (_touches[i]) {
      ++count;
    }
  }
  return count;
}
const ni::sVec2f _GetTouchLocation(CGPoint locationInView) {
  ni::sVec2f r = {
    (float)locationInView.x * 1,
    (float)locationInView.y * 1
  };
  // niLog(Info,niFmt("_GetTouchLocation(%g,%g,%g) -> %g, %g",
  //                  locationInView.x, locationInView.y,
  //                  mainScreenScale,
  //                  r.x, r.y));
  return r;
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event{
  for (UITouch* touch in touches) {
    int fingerId = _GetFingerIdFromTouch(touch);
    if (fingerId == -1) {
      fingerId = _AddNewTouch(touch);
    }
    else {
      continue;
    }

    const ni::sVec2f vTouchLocation = _GetTouchLocation([touch locationInView:self]);
    _lastTouchPosition[fingerId] = vTouchLocation;
    app::AppGenericFingerPress(&gAppContext, fingerId, ni::eTrue, vTouchLocation.x, vTouchLocation.y, 1.0f);

    TRACE_INPUT(("iOS FINGER DOWN[%d]: %s", fingerId, vTouchLocation));
  }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event{
  for (UITouch* touch in touches) {
    const int fingerId = _GetFingerIdFromTouch(touch);
    if (fingerId == -1) {
      // not in the list ??
      continue;
    }

    const ni::sVec2f vTouchLocation = _GetTouchLocation([touch locationInView:self]);
    app::AppGenericFingerMove(&gAppContext, fingerId, vTouchLocation.x, vTouchLocation.y, 1.0f);
    {
      // Apply scaling to the relative move speed so that the movement speed is consistent with the
      // screen's DPI relative to the actual scale currently active. (So that a retina display's
      // relative movement isn't halved when using a kSettings_ScaleFactor of 1)
      const ni::tF32 relativeMoveSpeedScale = [UIScreen mainScreen].scale;
      const ni::sVec2f delta = (vTouchLocation - _lastTouchPosition[fingerId]) * relativeMoveSpeedScale;
      _lastTouchPosition[fingerId] = vTouchLocation;
      app::AppGenericFingerRelativeMove(&gAppContext, fingerId, delta.x, delta.y, 1.0f);
    }

    TRACE_INPUT(("iOS FINGER[%d]: MOVE: %s", fingerId, vTouchLocation));
  }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event{
  for (UITouch* touch in touches) {
    const int fingerId = _GetFingerIdFromTouch(touch);
    if (fingerId == -1) {
      // not in the list ??
      continue;
    }

    const ni::sVec2f vTouchLocation = _GetTouchLocation([touch locationInView:self]);
    app::AppGenericFingerPress(&gAppContext, fingerId, ni::eFalse, vTouchLocation.x, vTouchLocation.y, 1.0f);

    TRACE_INPUT(("iOS FINGER UP[%d]: %s", fingerId, vTouchLocation));
    _RemoveTouch(fingerId);
  }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event{
  // Redirect to touchesEnded
  [self touchesEnded:touches withEvent: event];
}
@end
