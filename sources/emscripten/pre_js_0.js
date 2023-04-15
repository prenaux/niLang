// We don't want our files to use the preload plugins
Module.noImageDecoding = true;
Module.noAudioDecoding = true;

var NIAPP_AR = {
  container: null,
  video: null,
  playerCanvas: null,

  _startVideo: function() {
    console.log("... NIAPP_AR.startVideo");

    // TODO: Hardcoding the IDs (ni-player-container & canvas) isnt great
    //       but it makes things a lot simpler for now.
    this.container = document.getElementById('ni-player-container');
    this.playerCanvas = this.container.querySelector('#canvas');
    this.playerCanvas.classList.add('ni-player-ar-canvas');

    this.video = document.createElement('video');

    this.video.setAttribute('class', 'ni-player-ar-video');
    this.video.setAttribute('autoplay', '');
    this.video.setAttribute('muted', '');
    this.video.setAttribute('playsinline', '');
    this.container.insertBefore(this.video, this.container.firstChild);
    /* this.container.appendChild(this.video); */

    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
      throw new Error("No mediaDevices or getUserMedia.");
      return;
    }

    navigator.mediaDevices.getUserMedia({audio: false, video: {
      facingMode: 'environment',
    }}).then((stream) => {
      this.video.addEventListener( 'loadedmetadata', () => {
        var w = this.video.videoWidth;
        var h = this.video.videoHeight;
        console.log("... NIAPP_AR.startVideo: " + w + ", " + h);
        this.video.setAttribute('width', w);
        this.video.setAttribute('height', h);
        niApp.NotifyHost("NIAPP_AR.startedVideo");
      });
      this.video.srcObject = stream;
    }).catch((err) => {
      throw new Error("getUserMedia error: " + err);
    });
  },

  start: function() {
    this._startVideo();
  },
};

var NIAPP_BROWSER = (function() {
  var b = {};

  b.isTouch = ('ontouchend' in document) || navigator.maxTouchPoints > 0 || navigator.msMaxTouchPoints > 0 || "yobiatch";
  var ua = navigator.userAgent;
  b.ua = navigator.userAgent;
  b.isAndroid = !!ua.match(/Android/i);
  b.isAndroidMobile = b.isAndroid && !!ua.match(/Mobile/i);
  b.isAndroidTablet = b.isAndroid && !b.isAndroidMobile;
  b.isBlackberry = !!ua.match(/BlackBerry/i);
  b.isIPhone = !!ua.match(/iPhone/i);
  b.isIPad = !!ua.match(/iPad/i);
  b.isIPod = !!ua.match(/iPod/i);
  b.isIos = b.isIPhone || b.isIPad || b.isIPod;
  b.isOperaMini = !!ua.match(/Opera Mini/i);
  b.isIeMobile = !!ua.match(/IEMobile/i);

  b.isBot = !!ua.match(/googlebot|yahoo|msnbot|bingbot|slurp|teoma/i);

  b.isPhone = b.isAndroidMobile || b.isBlackberry || b.isIPhone || b.isIPod || b.isOperaMini || b.isIeMobile;
  b.isTablet = b.isAndroidTablet || b.isIPad;

  b.isMobile = b.isPhone || b.isTablet;
  return b;
})();

var NIAPP_CONFIG = {
  baseUrl: (function(){
    var url = window.location.href;
    var binPos = url.indexOf("niLang/bin/web-js");
    if (binPos >= 0) {
      url = url.substr(url,binPos);
    }
    return url;
  }()),

  width: 1280,
  height: 720,
  maxWidth: undefined,
  maxHeight: undefined,
  useBestResolution: true,

  captureMouseWheel: true,
  fingerRelativeLockedSpeed: 0.4,
  fingerRelativeNormalSpeed: 0.6,

  /* file systems to preload */
  fsModules: [
    'niUI',
  ],

  watchResizeDelay: undefined,
};

var NIAPP_CAPI = {
  IsInitialized: function() {
    return !!this._didInitialize;
  },

  CWrap: function(name, ret, args) {
    var mname = "niJSCC_" + name;
    var m = Module['cwrap'](mname,ret,args);
    this[name] = m;
    return m;
  },

  Initialize: function() {
    if (this._didInitialize)
      return;
    this._didInitialize = true;

    var z = '';
    var n = 'number';
    var s = 'string';
    var w = this.CWrap.bind(this);

    // Translate dom to niLang
    w('TranslateDOMKeyCode', n, [n]);
    w('TranslateDOMMouseButton', n, [n]);
    // System
    w('SetProperty', z, [s,s]);
    w('GetProperty', s, [s]);
    w('RunCommand', n, [s]);
    // Window notification
    w('HasWindow', n, []);
    w('WndNotifyResize', z, [n,n,n]);
    w('WndNotifyFocus', z, [n]);
    // Window keyboard input
    w('WndInputKey', z, [n, n]);
    w('WndInputString', z, [s]);
    // Window mouse input
    w('WndInputMouseMove', z, [n,n]);
    w('WndInputMouseRelativeMove', z, [n,n]);
    w('WndInputMouseButton', z, [n,n]);
    w('WndInputMouseDoubleClick', z, [n]);
    w('WndInputMouseWheel', z, [n]);
    // Window finger input / touch input
    w('WndInputFingerMove', z, [n,n,n,n]);
    w('WndInputFingerPress', z, [n,n,n,n,n]);
  }
}

var NIAPP_INPUT = (function NIAPP_INPUT() {
  var _isMouseHover = false;
  var _keyIsDown = {};
  var _keyDownCount = 0;
  var _mouseMove = { absX: -1, absY: -1, relX: 0, relY: 0 };
  var _mouseButton = 0;
  var _eatNextContextMenu = false;
  // Needed to avoid relative move "pop" when entering/exiting pointer locked
  var _eatNextMouseRelative = 0;

  var _isThisTarget = function(event) {
    return event.target == Module['canvas'];
  }

  var _inputDebug = undefined;
  // var _inputDebug = function(msg) { console.log(msg); }

  var _calculatePointerPos = function(event) {
    var canvas = Module["canvas"];
    var scrollX = window.scrollX || document.documentElement.scrollLeft || window.pageXOffset;
    var scrollY = window.scrollY || document.documentElement.scrollTop || window.pageYOffset;
    var rect = canvas.getBoundingClientRect();
    var x, y;
    x = event.pageX - (scrollX + rect.left);
    y = event.pageY - (scrollY + rect.top);
    var cw = canvas.width;
    var ch = canvas.height;
    x = x * (cw / rect.width);
    y = y * (ch / rect.height);
    return {x:x,y:y};
  }

  var _calculateMouseEvent = function(event) {
    var pos = _calculatePointerPos(event);
    _mouseMove.relX = event.movementX || event.mozMovementX || event.webkitMovementX || 0;
    _mouseMove.relY = event.movementY || event.mozMovementY || event.webkitMovementY || 0;
    _mouseMove.absX = pos.x;
    _mouseMove.absY = pos.y;
  };

  var _sendRelativeMouseMove = function(bt, afX, afY) {
    if (_eatNextMouseRelative > 0) {
      (_inputDebug && _inputDebug("_eatNextMouseRelative: " + _eatNextMouseRelative));
      _eatNextMouseRelative--;
    } else {
      (_inputDebug && _inputDebug("MOUSERELATIVEMOVE: target: " + event.target.id + " mx:" + afX + " my:" + afY));
      NIAPP_CAPI.WndInputMouseRelativeMove(afX, afY);
      _eatNextMouseRelative = 0;
    }
  };

  var _sendMouseMove = function(event,canRelative) {
    // console.log("_sendMouseMove: " + event.type + ", mx: " + event.pageX + ", my: " + event.pageY + ", rx: " + event.webkitMovementX + ", ry: " + event.webkitMovementY);
    var canvas = Module['canvas'];
    var lastX = _mouseMove.absX;
    var lastY = _mouseMove.absY;
    var bt = _translateMouseButton(event['button']);
    _calculateMouseEvent(event);
    if (_isLocked) {
      if (canRelative) {
        _sendRelativeMouseMove(bt, _mouseMove.relX, _mouseMove.relY);
      }
      return true;
    }
    else {
      var newX = _mouseMove.absX;
      var newY = _mouseMove.absY;

      _isMouseHover = (event.target == canvas);
      if (newX == lastX && newY == lastY)
        return _isMouseHover;

      if (_isMouseHover || (_mouseButton != 0) || (_keyDownCount != 0)) {
        (_inputDebug && _inputDebug("MOUSEMOVE: target: " + event.target.id + " mx:" + newX + " my:" + newY));
        NIAPP_CAPI.WndInputMouseMove(newX, newY);
        if (canRelative) {
          _sendRelativeMouseMove(bt, _mouseMove.relX, _mouseMove.relY);
        }
        return true;
      }
    }
  }

  var _translateKeyCode = function (keyCode) {
    return NIAPP_CAPI.TranslateDOMKeyCode(keyCode);
  }

  var _translateMouseButton = function (bt) {
    if (bt == 1) // middle click
      return 2;
    if (bt == 2) // right click
      return 1;
    return bt;
  }

  var _isLocked = false;

  var _setIsLocked = function(abLocked) {
    _isLocked = abLocked;
    _eatNextMouseRelative = 1;
    NIAPP_CAPI.SetPointerLocked(_isLocked);
  }

  var _setPointerLock = function(e,abLocked) {
    if (abLocked) {
      e.requestPointerLock = e.requestPointerLock ||
        e.mozRequestPointerLock ||
        e.webkitRequestPointerLock;
      if (e.requestPointerLock) {
        e.requestPointerLock();
      }
    }
    else {
      document.exitPointerLock = document.exitPointerLock ||
        document.mozExitPointerLock ||
        document.webkitExitPointerLock;
      if (document.exitPointerLock) {
        document.exitPointerLock();
      }
    }
  }

  return {
    //// Key Events ////
    handleKeyDown: function(event) {
      // console.log("handleKeyDown: " + event['keyCode']);

      if ((_mouseButton == 0) && !_isMouseHover && !_isLocked)
        return;

      var keyCode = _translateKeyCode(event['keyCode']);
      if (keyCode && keyCode > 0) {
        if (!_keyIsDown[keyCode]) {
          ++_keyDownCount;
          _keyIsDown[keyCode] = true;
        }
        NIAPP_CAPI.WndInputKey(keyCode,1);
        (_inputDebug && _inputDebug("KEYDOWN: count: " + _keyDownCount + ", key:" + keyCode + " event: " + event['keyCode'] + " target: " + event.target.id));
        event.preventDefault();
      }

      var key = event["key"];
      if (key && key.length === 1) {
        NIAPP_CAPI.WndInputString(""+key);
      }
    },
    handleKeyUp: function(event) {
      if ((_keyDownCount == 0) && (_mouseButton == 0) && !_isMouseHover && !_isLocked)
        return;
      var keyCode = _translateKeyCode(event['keyCode']);
      if (keyCode && keyCode > 0) {
        NIAPP_CAPI.WndInputKey(keyCode,0);
        if (_keyIsDown[keyCode]) {
          --_keyDownCount;
          _keyIsDown[keyCode] = false;
        }
        (_inputDebug && _inputDebug("KEYUP: count: " + _keyDownCount + ", key:" + keyCode + " event: " + event['keyCode'] + " target: " + event.target.id));
        event.preventDefault();
      }
    },

    //// Mouse Events ////
    handleMouseMove: function(event) {
      if (_sendMouseMove(event,true)) {
        event.preventDefault();
      }
    },
    handleMouseDown: function(event) {
      _sendMouseMove(event);
      var bt = _translateMouseButton(event['button']);
      if (_isThisTarget(event) && ((_mouseButton & (1 << bt)) == 0)) {
        (_inputDebug && _inputDebug("MOUSEDOWN: target: " + event.target.id + " bt: " + bt));
        _mouseButton |= (1 << bt);
        NIAPP_CAPI.WndInputMouseButton(bt, true);
        // PIERRE: Don't preventDefault on mouse down otherwise the canvas
        // will never get back the focus.
        if (bt == 2) {
          // Ok well prevent for the middle button, we dont want the horrible
          // scrolling thing to popup...
          event.preventDefault();
        }
      }
    },
    handleMouseUp: function(event) {
      _sendMouseMove(event);
      var bt = _translateMouseButton(event['button']);
      if ((_mouseButton & (1 << bt)) != 0) {
        (_inputDebug && _inputDebug("MOUSEUP: target: " + event.target.id + " bt: " + bt));
        _mouseButton &= ~(1 << bt);
        NIAPP_CAPI.WndInputMouseButton(bt, false);
        event.preventDefault();
        if (bt == 2) {
          // Right click
          if (!_isMouseHover) {
            _eatNextContextMenu = true;
          }
        }
      }
    },

    //// Double click ////
    handleDoubleClick: function(event) {
      if (!_isThisTarget(event))
        return;
      var bt = _translateMouseButton(event['button']);
      NIAPP_CAPI.WndInputMouseDoubleClick(bt);
      event.preventDefault();
    },

    //// Pointer Lock Lost ////
    handlePointerLockError: function(event) {
      niApp.Debug("handlePointerLockError");
      _setIsLocked(false);
    },
    handlePointerLockChange: function(event) {
      var canvas = Module['canvas'];
      _setIsLocked(
        document.pointerLockElement === canvas ||
          document.mozPointerLockElement === canvas ||
          document.webkitPointerLockElement === canvas);
      niApp.Debug("handlePointerLockChange:" + _isLocked);
    },

    //// Context Menu ////
    handleContextMenu: function(event) {
      if (_isMouseHover || _eatNextContextMenu) {
        (_inputDebug && _inputDebug("HANDLE CONTEXT MENU"));
        event.preventDefault();
        _eatNextContextMenu = false;
      }
    },

    //// Mouse Wheel ////
    handleMouseWheel: function(event) {
      if (!NIAPP_CONFIG.captureMouseWheel)
        return; // for now we dont send the mouse wheel event if its not captured...

      _sendMouseMove(event);

      if (_isThisTarget(event) || (_mouseButton != 0) || (_keyDownCount != 0)) {
        (_inputDebug && _inputDebug("MOUSEWHEEL: target: " + event.target.id));
        var e = window.event || event; // old IE support
        var delta = Math.max(-1, Math.min(1, (e.wheelDelta || -e.detail)));
        if (delta < 0) {
          NIAPP_CAPI.WndInputMouseWheel(-1);
        }
        else {
          NIAPP_CAPI.WndInputMouseWheel(1);
        }
        event.preventDefault();
      }
    },

    //// Touch Events ////
    handleTouchMove: function(event) {
      if (_isThisTarget(event)) {
        var touches = event.changedTouches;
        var numTouches  = touches.length;
        for (var i = 0; i < numTouches; ++i) {
          var touch = touches[i];
          var touchPos = _calculatePointerPos(touch);
          _inputDebug && _inputDebug("TOUCHMOVE: finger: " + touch.identifier + " x: " + touchPos.x + ", y: " + touchPos.y);
          NIAPP_CAPI.WndInputFingerMove(touch.identifier, touchPos.x, touchPos.y, 1.0);
        }
      }
    },
    handleTouchDown: function(event) {
      if (_isThisTarget(event)) {
        var touches = event.changedTouches;
        var numTouches  = touches.length;
        for (var i = 0; i < numTouches; ++i) {
          var touch = touches[i];
          var touchPos = _calculatePointerPos(touch);
          _inputDebug && _inputDebug("TOUCHDOWN: finger: " + touch.identifier + " x: " + touchPos.x + ", y: " + touchPos.y);
          NIAPP_CAPI.WndInputFingerPress(touch.identifier, true, touchPos.x, touchPos.y, 1.0);
        }

        if (event && event.target && event.target.setCapture) {
          event.target.setCapture(true);
        }
        event.preventDefault();
      }
    },
    handleTouchUp: function(event) {
      if (_isThisTarget(event)) {
        var touches = event.changedTouches;
        var numTouches  = touches.length;
        for (var i = 0; i < numTouches; ++i) {
          var touch = touches[i];
          var touchPos = _calculatePointerPos(touch);
          _inputDebug && _inputDebug("TOUCHUP: finger: " + touch.identifier + " x: " + touchPos.x + ", y: " + touchPos.y);
          NIAPP_CAPI.WndInputFingerPress(touch.identifier, false, touchPos.x, touchPos.y, 0);
        }
        event.preventDefault();
        if (event && event.target && event.target.setCapture) {
          event.target.setCapture(false);
        }
      }
    },

    addEvents: function(inputListener) {
      inputListener.addEventListener("contextmenu", this.handleContextMenu, true);

      inputListener.addEventListener("keydown", this.handleKeyDown, true);
      inputListener.addEventListener("keyup", this.handleKeyUp, true);

      if (NIAPP_BROWSER.isTouch) {
        inputListener.addEventListener("touchmove", this.handleTouchMove, true);
        inputListener.addEventListener("touchstart", this.handleTouchDown, true);
        inputListener.addEventListener("touchend", this.handleTouchUp, true);
        inputListener.addEventListener("touchcancel", this.handleTouchUp, true);
      }
      {
        inputListener.addEventListener("mousemove", this.handleMouseMove, true);
        inputListener.addEventListener("mousedown", this.handleMouseDown, true);
        inputListener.addEventListener("mouseup", this.handleMouseUp, true);

        document.addEventListener('pointerlockchange', this.handlePointerLockChange, false);
        document.addEventListener('mozpointerlockchange', this.handlePointerLockChange, false);
        document.addEventListener('webkitpointerlockchange', this.handlePointerLockChange, false);
        document.addEventListener('pointerlockerror', this.handlePointerLockError, false);
        document.addEventListener('mozpointerlockerror', this.handlePointerLockError, false);
        document.addEventListener('webkitpointerlockerror', this.handlePointerLockError, false);
      }

      // IE9, Chrome, Safari, Opera
      inputListener.addEventListener("mousewheel", this.handleMouseWheel, true);
      // Firefox
      inputListener.addEventListener("DOMMouseScroll", this.handleMouseWheel, true);
      // handle double click
      inputListener.addEventListener("dblclick", this.handleDoubleClick, true);
    },
  }
});

function niPath_Join(aArray) {
  if (arguments.length > 1) {
    aArray = Array.from(arguments);
  }
  return aArray
    .join("/")
    .replaceAll("//","/")
    /* restore protocol :// */
    .replace(":/","://");
}
function niPath_SplitDirAndFilename(aPath) {
  var r = aPath.match(/^(.*?)([^\\\/]*)$/, '$2');
  return [r[1],r[2]];
}
function niPath_GetDirname(aPath) {
  return aPath.replace(/^(.*?)([^\\\/]*)$/, '$1');
}
function niPath_GetFilename(aPath) {
  return aPath.replace(/^(.*?)([^\\\/]*)$/, '$2');
}
function niPath_NoExt(aPath) {
  return aPath.replace(new RegExp("\.[^/.]+$"), "");
}

function niFS_AddSingleFilePreloaded(aPath, aUrl) {
  var pathdir = niPath_GetDirname(aPath);
  var pathfile = niPath_GetFilename(aPath);
  /* console.log("niFS_AddSingleFilePreloaded: " + aPath + " -> " + aUrl); */
  try {
    FS.createPreloadedFile(pathdir, pathfile, aUrl, true, false);
  }
  catch (e) {
    console.error("Can't preload file:" + aPath);
  }
}
// Requires --proxy-to-worker
function niFS_AddSingleFileLazy(aPath, aUrl) {
  var pathdir = niPath_GetDirname(aPath);
  var pathfile = niPath_GetFilename(aPath);
  /* console.log("niFS_AddSingleFileLazy: " + aPath + " -> " + aUrl); */
  try {
    FS.createLazyFile(pathdir, pathfile, aUrl, true, false);
  }
  catch (e) {
    console.error("Can't add lazy file:" + aPath);
  }
}

// TODO: Auto-detect whether we should preload or lazily load by default
var niFS_AddSingleFile = niFS_AddSingleFilePreloaded;

function niFS_AddDirs(basePath, aDirs) {
  var numDirs = aDirs.length;
  for (var i = 0; i < numDirs; ++i) {
    var dir = aDirs[i];
    var path = niPath_Join(basePath,dir);
    console.log("niFS_AddDirs: " + path);
    FS.mkdir(path);
  }
}

function niFS_AddFiles(basePath, baseUrl, aFiles, aCreateFile) {
  aCreateFile = aCreateFile || niFS_AddSingleFile;
  var numFiles = aFiles.length;
  for (var i = 0; i < numFiles; ++i) {
    var fn = aFiles[i];
    var path = niPath_Join(basePath,fn);
    var url = niPath_Join(baseUrl,fn);
    aCreateFile(path, url);
  }
}

var NIAPP_FSMODULES = {
  // dirs: find "$WORK/niLang/data/test" -type d | sed "s~$WORK/niLang/data/test/~~g" | sort
  // files: find "$WORK/niLang/data/test" -type f | sed "s~$WORK/niLang/data/test/~~g" | sort
  test: function() {
    console.log("FSModule: test");
    var dir = "/Work/niLang/data/test";
    var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/test/");
    FS.mkdir(dir);
    niFS_AddDirs(dir, [
      "fonts",
      "img_diff",
      "ipcam",
      "tex",
    ]);
    niFS_AddFiles(dir, url, [
      "fonts/Copper-Kern-Bold.ttf",
      "fonts/EyeOT-Mono-Bold.ttf",
      "fonts/EyeOT-Mono-Medium.ttf",
      "fonts/EyeOT-Mono.ttf",
      "fonts/Fax Sans Beta.otf",
      "fonts/FuturaLT-Bold.ttf",
      "fonts/FuturaLT-BoldOblique.ttf",
      "fonts/FuturaLT-Book.ttf",
      "fonts/FuturaLT-BookOblique.ttf",
      "fonts/FuturaLT-Heavy.ttf",
      "fonts/FuturaLT-HeavyOblique.ttf",
      "fonts/FuturaLT-Oblique.ttf",
      "fonts/FuturaLT.ttf",
      "fonts/Junction-bold.otf",
      "fonts/Junction-light.otf",
      "fonts/Junction-regular.otf",
      "fonts/LeagueGothic-CondensedItalic.otf",
      "fonts/LeagueGothic-CondensedRegular.otf",
      "fonts/LeagueGothic-Italic.otf",
      "fonts/LeagueGothic-Regular.otf",
      "fonts/LeagueSpartan-Bold.otf",
      "fonts/LeagueSpartan-Bold.ttf",
      "fonts/Linden Hill Italic.otf",
      "fonts/Linden Hill.otf",
      "fonts/Orbitron Black.otf",
      "fonts/Orbitron Bold.otf",
      "fonts/Orbitron Light.otf",
      "fonts/Orbitron Medium.otf",
      "fonts/OstrichSans-Black.otf",
      "fonts/OstrichSans-Bold.otf",
      "fonts/OstrichSans-Heavy.otf",
      "fonts/OstrichSans-Light.otf",
      "fonts/OstrichSans-Medium.otf",
      "fonts/OstrichSansDashed-Medium.otf",
      "fonts/OstrichSansInline-Italic.otf",
      "fonts/OstrichSansInline-Regular.otf",
      "fonts/OstrichSansRounded-Medium.otf",
      "fonts/Sniglet-Regular.ttf",
      "fonts/Univers67-Condensed-Bold.ttf",
      "fonts/UniversalisADFStd-Bold.otf",
      "fonts/UniversalisADFStd-BoldCond.otf",
      "fonts/UniversalisADFStd-BoldCondIt.otf",
      "fonts/UniversalisADFStd-BoldItalic.otf",
      "fonts/UniversalisADFStd-Cond.otf",
      "fonts/UniversalisADFStd-CondItalic.otf",
      "fonts/UniversalisADFStd-Italic.otf",
      "fonts/UniversalisADFStd-Regular.otf",
      "fonts/times.ttf",
      "fonts/timesbd.ttf",
      "fonts/timesbi.ttf",
      "fonts/timesi.ttf",
      "fonts/unvr67w.ttf",
      "img_diff/AnimMixer_A.jpg",
      "img_diff/AnimMixer_B.jpg",
      "ipcam/CCTV-01.ipcam",
      "ipcam/CCTV-02.ipcam",
      "tex/Beach_cubemap_128.dds",
      "tex/Beach_probe_128.dds",
      "tex/earth_b.jpg",
      "tex/earth_clouds_d.jpg",
      "tex/earth_clouds_o.jpg",
      "tex/earth_d.jpg",
      "tex/earth_lights.jpg",
      "tex/earth_s.jpg",
      "tex/glass.tga",
      "tex/glass_cubemap.dds",
      "tex/rust_steel.jpg",
      "tex/tree.dds",
    ]);
  },

  // dirs: find "$WORK/niLang/data/Test_niLang" -type d | sed "s~$WORK/niLang/data/Test_niLang/~~g" | sort
  // files: find "$WORK/niLang/data/Test_niLang" -type f | sed "s~$WORK/niLang/data/Test_niLang/~~g" | sort
  Test_niLang: function() {
    console.log("FSModule: Test_niLang");
    var dir = "/Work/niLang/data/Test_niLang";
    var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/Test_niLang/");
    FS.mkdir(dir);
    niFS_AddFiles(dir, url, [
      "hello-file-system.txt"
    ]);
  },

  // dirs: find "$WORK/niLang/data/Test_niSound" -type d | sed "s~$WORK/niLang/data/Test_niSound/~~g"
  // files: find "$WORK/niLang/data/Test_niSound" -type f | sed "s~$WORK/niLang/data/Test_niSound/~~g"
  Test_niSound: function() {
    console.log("FSModule: Test_niSound");
    var dir = "/Work/niLang/data/Test_niSound";
    var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/Test_niSound/");
    FS.mkdir(dir);
    niFS_AddFiles(dir, url, [
      "atmo_loop_01.wav",
      "step_stone5.wav",
      "step_stone4.wav",
      "step_stone1.wav",
      "step_stone3.wav",
      "atmo_loop_01.ogg",
      "step_stone2.wav",
      "music_sketch8.ogg",
      "lobby_chat.wav",
      "lobby_chat.ogg",
      "click.wav",
      "music_punch.ogg",
      "click.ogg",
    ]);
  },

  // dirs: find "$WORK/niLang/data/niUI" -type d | sed "s~$WORK/niLang/data/niUI/~~g"
  // files: find "$WORK/niLang/data/niUI" -type f | sed "s~$WORK/niLang/data/niUI/~~g"
  niUI: function() {
    console.log("FSModule: niUI");
    var dir = "/Work/niLang/data/niUI";
    var url = niPath_Join(NIAPP_CONFIG.baseUrl, "/niLang/data/niUI");
    FS.mkdir(dir);
    niFS_AddDirs(dir, [
      "skins",
      "shaders",
      "fonts",
    ]);
    niFS_AddFiles(dir, url, [
      "error.dds",
      "skins/UIDark.png",
      "skins/default.uiskin.xml",
      "shaders/fixed_ps_decal_distancefield.cgo",
      "shaders/fixed_ps_decal_base_alphatest.cgo",
      "shaders/fixed_ps_base_opacity_alphatest.cgo",
      "shaders/fixed_vs_pnat1.cgo",
      "shaders/fixed_vs_pna.cgo",
      "shaders/fixed_vs_pnat2.cgo",
      "shaders/fixed_ps_base.cgo",
      "shaders/fixed_ps_decal_opacity_alphatest.cgo",
      "shaders/fixed_vs_pn.cgo",
      "shaders/fixed_vs_p.cgo",
      "shaders/fixed_ps_base_alphatest.cgo",
      "shaders/fixed_ps_decal_alphatest.cgo",
      "shaders/fixed_ps_translucent.cgo",
      "shaders/fixed_ps_decal_opacity_translucent.cgo",
      "shaders/fixed_ps_distancefield.cgo",
      "shaders/fixed_ps_opacity_translucent.cgo",
      "shaders/fixed_ps_decal_base_opacity_translucent.cgo",
      "shaders/fixed_ps.cgo",
      "shaders/fixed_vs_pb4inat1.cgo",
      "shaders/fixed_ps_opacity_alphatest.cgo",
      "shaders/fixed_vs_pa.cgo",
      "shaders/fixed_ps_base_distancefield.cgo",
      "shaders/fixed_ps_alphatest.cgo",
      "shaders/fixed_ps_decal.cgo",
      "shaders/fixed_ps_base_opacity_translucent.cgo",
      "shaders/fixed_ps_decal_translucent.cgo",
      "shaders/fixed_vs_pnt2.cgo",
      "shaders/fixed_ps_decal_base.cgo",
      "shaders/fixed_ps_decal_base_translucent.cgo",
      "shaders/fixed_vs_pb4int1.cgo",
      "shaders/fixed_ps_decal_base_distancefield.cgo",
      "shaders/fixed_ps_decal_base_opacity_alphatest.cgo",
      "shaders/fixed_ps_base_translucent.cgo",
      "shaders/fixed_vs_pnt1.cgo",
      "fonts/fas-900.otf",
      "fonts/far-400.otf",
      "fonts/fab-400.otf",
      "fonts/Roboto-Regular.ttf",
      "fonts/PlayfairDisplay-Bold.ttf",
      "fonts/NotoSansCJKsc-Regular.otf",
      "fonts/PlayfairDisplay-Italic.ttf",
      "fonts/Roboto-Bold.ttf",
      "fonts/PlayfairDisplay-BoldItalic.ttf",
      "fonts/NotoMono-Regular.ttf",
      "fonts/Roboto-Italic.ttf",
      "fonts/Roboto-BoldItalic.ttf",
      "fonts/PlayfairDisplay-Regular.ttf",
      "loading.tga"
    ]);
  },
};

var niApp = {
  Assert: function Assert(check, msg) {
    if (!check) {
      var theMsg = "" + msg + new Error().stack;
      console.log("ASSERT:" + theMsg);
      throw theMsg;
    }
  },

  Debug: function Debug(msg) {
    console.log(msg);
  },

  NotifyHost: function NotifyHost(aCmdLine) {
    aCmdLine = aCmdLine ? aCmdLine.trim() : "";
    if (!aCmdLine.length)
      return;
    var cmd, params;
    var spaceIndex = aCmdLine.indexOf(' ');
    if (spaceIndex >= 0) {
      cmd = aCmdLine.substr(0,spaceIndex);
      params = aCmdLine.substr(spaceIndex+1);
    }
    else {
      cmd = aCmdLine;
    }
    console.log("... niApp.NotifyHost: cmd: " + cmd + ", params: " + params);
    var notifyHostHandlers = Module['notifyHostHandlers'];
    var handler = notifyHostHandlers && notifyHostHandlers[cmd];
    handler && handler(params,cmd);
  },

  SetCanvasSize: function SetCanvasSize(canvas,width,height) {
    var contentsScale = NIAPP_CONFIG.useBestResolution ? (window.devicePixelRatio || 1) : 1;
    /* console.log("... niApp.SetCanvasSize: ", canvas, " w: ", width, " h: ", height, " contentsScale: ", contentsScale); */

    if (!width) {
      width = canvas.width / (window.devicePixelRatio || 1);
      if (!width) {
        console.warn("Can't get canvas width");
        return;
      }
    }
    if (!height) {
      height = canvas.height / (window.devicePixelRatio || 1);
      if (!height) {
        console.warn("Can't get canvas height");
        return;
      }
    }

    width *= contentsScale;
    height *= contentsScale;

    if ((NIAPP_CONFIG.maxWidth && NIAPP_CONFIG.maxHeight) &&
        (width > NIAPP_CONFIG.maxWidth || height > NIAPP_CONFIG.maxHeight))
    {
      var maxRatio = NIAPP_CONFIG.maxHeight / NIAPP_CONFIG.maxWidth;
      var canvasRatio = height / width;
      var scale = (canvasRatio < maxRatio) ?
                  (NIAPP_CONFIG.maxWidth / width) :
                  (NIAPP_CONFIG.maxHeight / height);
      width *= scale;
      width = Math.floor(width+0.5);
      height *= scale;
      height = Math.floor(height+0.5);
      contentsScale = 1.0;
    }

    if (canvas) {
      if (NIAPP_CONFIG.lastCanvasWidth != width ||
          NIAPP_CONFIG.lastCanvasHeight != height)
      {
        niApp.Debug("niApp.SetCanvasSize - canvasSize: w: " + width + ", h: " + height);
        canvas.width = width;
        canvas.height = height;
        NIAPP_CONFIG.lastCanvasWidth = width;
        NIAPP_CONFIG.lastCanvasHeight = height;
      }
    }
    if (NIAPP_CONFIG.lastCapiWidth != width ||
        NIAPP_CONFIG.lastCapiHeight != height)
    {
      niApp.Debug("niApp.SetCanvasSize - capiSize: w: " + width + ", h: " + height);
      NIAPP_CAPI.WndNotifyResize(width,height,contentsScale);
      NIAPP_CONFIG.lastCapiWidth = width;
      NIAPP_CONFIG.lastCapiHeight = height;
    }
    NIAPP_CONFIG.width = width;
    NIAPP_CONFIG.height = height;
  },

  AddInputEvents: function AddInputEvents(aElement) {
    if (typeof(aElement) == "string") {
      aElement = document.getElementById(aElement);
    }

    var inputLib = NIAPP_INPUT();
    inputLib.addEvents(aElement);
    return inputLib;
  },

  AddResizeEvents: function AddResizeEvents(aElement) {
    if (typeof(aElement) == "string") {
      aElement = document.getElementById(aElement);
    }

    var doResizeTimeout;
    function delayResized(width,height) {
      if (NIAPP_CONFIG.watchResizeDelay) {
        if (doResizeTimeout) {
          clearTimeout(doResizeTimeout);
        }
        doResizeTimeout = setTimeout(function() {
          niApp.SetCanvasSize(aElement,width,height);
        }, NIAPP_CONFIG.watchResizeDelay);
      }
      else {
        niApp.SetCanvasSize(aElement,width,height);
      }
    }

    aElement.addEventListener("resize", function(){
      delayResized(aElement.clientWidth,aElement.clientHeight);
    });
    window.addEventListener("resize", function(){
      delayResized(aElement.clientWidth,aElement.clientHeight);
    });

    niApp.SetCanvasSize(aElement,aElement.clientWidth,aElement.clientHeight);
  },

  GetParamsAsObject: function GetParamsAsObject(query) {
    query = query.substring(query.indexOf('?') + 1);

    var re = /([^&=]+)=?([^&]*)/g;
    var decodeRE = /\+/g;

    var decode = function (str) {
      return decodeURIComponent(str.replace(decodeRE, " "));
    };

    var params = {}, e;
    while (e = re.exec(query)) {
      var k = decode(e[1]), v = decode(e[2]);
      if (k.substring(k.length - 2) === '[]') {
        k = k.substring(0, k.length - 2);
        (params[k] || (params[k] = [])).push(v);
      }
      else params[k] = v;
    }

    var assign = function (obj, keyPath, value) {
      var lastKeyIndex = keyPath.length - 1;
      for (var i = 0; i < lastKeyIndex; ++i) {
        var key = keyPath[i];
        if (!(key in obj))
          obj[key] = {}
        obj = obj[key];
      }
      obj[keyPath[lastKeyIndex]] = value;
    }

    for (var prop in params) {
      var structure = prop.split('[');
      if (structure.length > 1) {
        var levels = [];
        structure.forEach(function (item, i) {
          var key = item.replace(/[?[\]\\ ]/g, '');
          levels.push(key);
        });
        assign(params, levels, params[prop]);
        delete(params[prop]);
      }
    }
    return params;
  },

  startAR: function() {
    NIAPP_AR.start();
  },
};

Module["preRun"] = function() {
  console.log("I/preRun BEGIN: baseUrl: " + NIAPP_CONFIG.baseUrl);

  var params = niApp.GetParamsAsObject(location.search);
  var title = "niLang";
  var fsModules = NIAPP_CONFIG.fsModules;

  // Set the default title and fs modules based on the app name
  var appName = niPath_NoExt(niPath_GetFilename(location.pathname));
  if (appName) {
    if (appName.endsWith("_ra") || appName.endsWith("_da")) {
      appName = appName.substr(0,appName.length-3);
    }
    title = appName;
    if (appName.toLowerCase().startsWith("test")) {
      fsModules.push("test");
    }
    if (NIAPP_FSMODULES[appName]) {
      fsModules.push(appName);
    }
  }
  console.log("AppName: " + appName);

  // Create the virtual WORK & niLang folders
  var workDir = "/Work";
  FS.mkdir(workDir);

  // niLang
  var niLangDir = workDir + "/niLang";
  FS.mkdir(niLangDir);
  FS.mkdir(niLangDir + "/data");
  FS.mkdir(niLangDir + "/bin");
  FS.mkdir(niLangDir + "/bin/web-js");

  // Load FS modules
  for (var i = 0; i < fsModules.length; ++i) {
    var mname = fsModules[i];
    var registerFSModule = NIAPP_FSMODULES[mname];
    if (!registerFSModule) {
      throw new Error("No FileSystem for '" + mname + "'.");
    }
    registerFSModule();
  }

  // Set the current directory so that it looks like a regular executable
  FS.chdir(niLangDir + "/bin/web-js");

  // Process the build-in parameters
  var packagePath, packageUrl;
  if (params["OpenFile"]) {
    packagePath = '/Work/Package.vpk';
    packageUrl = params["OpenFile"]; // niPath_Join(NIAPP_CONFIG.baseUrl,'_Packages/Tesla-Template-Version-1.vpk');
    console.log("Package: " + packagePath + " from " + packageUrl);
    niFS_AddSingleFile(packagePath,packageUrl);
    if (!title) {
      document.title = niPath_GetFilename(packageUrl);
    }
  }
  if (title) {
    document.title = title;
  }

  Module.onRuntimeInitialized = function() {
    // Initialize the CAPI bridge
    NIAPP_CAPI.Initialize();

    // Initialize the canvas
    var canvas = Module['canvas'];
    console.log("... onRuntimeInitialized: canvas: ", canvas);
    niApp.AddInputEvents(canvas);
    niApp.AddResizeEvents(canvas);

    // Parse and set the parameters
    for (var pname in params) {
      var pval = params[pname];
      console.log("... Property: " + pname + " = " + pval);
      switch (pname) {
        case "OpenFile": {
          NIAPP_CAPI.SetProperty("OpenFile",packagePath);
          break;
        }
        default:
          NIAPP_CAPI.SetProperty(pname,pval);
          break;
      }
    }
  };

  console.log("I/preRun DONE");
};
