package ni.android.view;

import static ni.common.math.Rect.Recti;
import static ni.common.math.Vec.Vec3f;

import java.io.FileNotFoundException;

import javax.microedition.khronos.opengles.GL;

import ni.NiException;
import ni.android.util.Keys;
import ni.app.AppConfig;
import ni.app.AppEventHandler;
import ni.common.util.Log;
import ni.niLang.EKey;
import ni.niLang.EOSWindowMessage;
import ni.niLang.ICollection;
import ni.niLang.IOSWindow;
import ni.niLang.IOSWindowGeneric;
import ni.niLang.Lib;
import ni.niLang.util.Msg;
import ni.niUI.ETextureFlags;
import ni.types.Vec3f;
import ni.types.Vec4i;
import ni.common.math.Vec;
import android.app.Activity;
import android.content.Context;
import android.view.MotionEvent;
import android.util.DisplayMetrics;

public class AppWindow extends ni.app.BaseAppWindow {
	public static float _touchScaleX = 1.0f;
	public static float _touchScaleY = 1.0f;

    private final Vec3f _motionParam = Vec3f();
    private final Vec3f _motionParamDelta = Vec3f();
    private final Vec3f _prevMotionParam[] = { Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f(),
											   Vec3f(), Vec3f(), Vec3f(), Vec3f(), Vec3f() };
    protected final Vec4i _wndRect;
    protected final Context _context;
    protected ICollection _wndMT;
    protected IOSWindowGeneric _wndGeneric;

    public AppWindow(final Context activity, final AppEventHandler eventHandler, final int initialWidth, final int initialHeight) throws FileNotFoundException, NiException {
        super(eventHandler);
        _wndRect = Recti(0, 0, initialWidth, initialHeight);
        _context = activity;
        construct();
    }

    public Context getContext() {
        return _context;
    }

    @Override
    protected AppConfig createAppConfig() throws NiException {
        final AppConfig cfg = new AppConfig();
        if (_context instanceof Activity) {
            cfg.windowTitle = ((Activity)_context).getTitle().toString();
        }
        return cfg;
    }

    @Override
    protected void initWindowAndGraphicsContext() throws NiException
    {
        // Create the generic window object
        _wnd = IOSWindow.query(lang.createInstance("niLang.OSWindowGeneric", null, null));
        if (_wnd == null)
            throw new NiException("Can't create the application's window.");
        _wnd.setTitle(_config.windowTitle);
        _wndMT = _wnd.getMessageHandlers();
        _wnd.setRect(_wndRect);

        _wndGeneric = IOSWindowGeneric.query(_wnd);
        _wndGeneric.setContentsScale(_context.getResources().getDisplayMetrics().density);

        //// Create the graphics context ////
        if (!_graphics.initializeDriver(Lib.hstr(_config.renDriver)))
            throw new NiException("Can't initialize graphics driver.");

        _graphicsContext = _graphics.createContextForWindow(_wnd,
				_config.renBBFmt, _config.renDSFmt,
                _config.renSwapInterval,
                // ETextureFlags.Virtual asks the driver to not create the
                // context by itself and use the one we already have...
                _config.renBBFlags | ETextureFlags.Virtual);
        if (_graphicsContext == null)
            throw new NiException("Can't create graphics context.");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void dispose() {
        super.dispose();
    }

    public void draw(final GL gl) {
        if (!update()) {
            if (_context instanceof Activity) {
                ((Activity)_context).finish();
            }
            return;
        }
        draw();
    }

    public void resize(final int width, final int height) {
        _wndRect.setWidth(width);
        _wndRect.setHeight(height);
        _wnd.setRect(_wndRect);
        postEventMessage(EOSWindowMessage.Size);
    }

    public boolean inputKey(final int keyCode, final boolean isDown) {
        if (keyCode < Keys.mVKMap.length) {
            final int keyToSend = Keys.mVKMap[keyCode];
            if (keyToSend != EKey.Unknown) {
            	postInputMessage(
                        isDown ? EOSWindowMessage.KeyDown : EOSWindowMessage.KeyUp,
                        keyToSend);
                return true;
            }
        }
        return false;
    }

    public void inputString(final String str) {
        _wndGeneric.genericInputString(str);
    }

    // this shit isn't defined properly and is different between Android 2.1 & 2.2... Android API Happiness++
    public static final int ACTION_POINTER_INDEX_MASK = 0xff00;
    // this shit isn't defined properly and is different between Android 2.1 & 2.2... Android API Happiness++
    public static final int ACTION_POINTER_INDEX_SHIFT = 8;

    public void inputTouch(final MotionEvent aMotionEvent) {
        final int numEvents = aMotionEvent.getPointerCount();
        final int action = aMotionEvent.getAction();
        final int actionMasked = (action & MotionEvent.ACTION_MASK);
        switch (actionMasked) {
        case MotionEvent.ACTION_DOWN: {
            for (int i = 0; i < numEvents; ++i) {
                final int pid = aMotionEvent.getPointerId(i);
                _motionParam.x = aMotionEvent.getX(i) * _touchScaleX;
                _motionParam.y = aMotionEvent.getY(i) * _touchScaleY;
                _motionParam.z = aMotionEvent.getPressure(i);
				if (pid < _prevMotionParam.length) {
					_prevMotionParam[pid].set(_motionParam);
				}
                Log.i("# FINGER [" + pid + "] ACTION_DOWN (of " + numEvents + "): " + _motionParam +
                        "(touchScale: " + _touchScaleX + ", " + _touchScaleY + ")");
                postInputMessage(EOSWindowMessage.FingerDown, pid, _motionParam);
            }
            break;
        }
        case MotionEvent.ACTION_CANCEL:
        case MotionEvent.ACTION_UP: {
            for (int i = 0; i < numEvents; ++i) {
                final int pid = aMotionEvent.getPointerId(i);
                _motionParam.x = aMotionEvent.getX(i) * _touchScaleX;
                _motionParam.y = aMotionEvent.getY(i) * _touchScaleY;
                _motionParam.z = aMotionEvent.getPressure(i);
                Log.i("# FINGER [" + pid + "] " +
                        ((actionMasked == MotionEvent.ACTION_UP) ? "ACTION_UP" : "ACTION_CANCEL") +
                        " (of " + numEvents + "): " + _motionParam +
						(((action & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_CANCEL) ? "(ACTION_CANCEL)" : ""));
                postInputMessage(EOSWindowMessage.FingerUp, pid, _motionParam);
            }
            break;
        }
        case MotionEvent.ACTION_MOVE: {
            for (int i = 0; i < numEvents; ++i) {
                final int pid = aMotionEvent.getPointerId(i);
                _motionParam.x = aMotionEvent.getX(i) * _touchScaleX;
                _motionParam.y = aMotionEvent.getY(i) * _touchScaleY;
                _motionParam.z = aMotionEvent.getPressure(i);
                postInputMessage(EOSWindowMessage.FingerMove, pid, _motionParam);
				if (pid < _prevMotionParam.length) {
					Vec.sub(_motionParamDelta, _motionParam, _prevMotionParam[pid]);
					if (Vec.lengthSq(_motionParamDelta) > Vec.EPSILON5) {
						_prevMotionParam[pid].set(_motionParam);
						postInputMessage(EOSWindowMessage.FingerRelativeMove, pid, _motionParamDelta);
					}
				}
                Log.i("# FINGER [" + pid + "] ACTION_MOVE (of " + numEvents + "): " + _motionParam);
            }
            break;
        }
        case MotionEvent.ACTION_POINTER_DOWN: {
            final int pointerIndex = (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT;
            final int pid = aMotionEvent.getPointerId(pointerIndex);
            _motionParam.x = aMotionEvent.getX(pointerIndex) * _touchScaleX;
            _motionParam.y = aMotionEvent.getY(pointerIndex) * _touchScaleY;
            _motionParam.z = aMotionEvent.getPressure(pointerIndex);
			if (pid < _prevMotionParam.length) {
				_prevMotionParam[pid].set(_motionParam);
			}
            Log.i("# FINGER [" + pid + "] ACTION_POINTER_DOWN: " + _motionParam);
            postInputMessage(EOSWindowMessage.FingerDown, pid, _motionParam);
            break;
        }
        case MotionEvent.ACTION_POINTER_UP: {
            final int pointerIndex = (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT;
            final int pid = aMotionEvent.getPointerId(pointerIndex);
            _motionParam.x = aMotionEvent.getX(pointerIndex) * _touchScaleX;
            _motionParam.y = aMotionEvent.getY(pointerIndex) * _touchScaleY;
            _motionParam.z = aMotionEvent.getPressure(pointerIndex);
            Log.i("# FINGER [" + pid + "] ACTION_POINTER_UP: " + _motionParam);
            postInputMessage(EOSWindowMessage.FingerUp, pid, _motionParam);
            break;
        }
        }
    }

    public void inputAccelerometer(final float[] values) {
        // TODO: Implement Accelerometer
        /*
        // swap x/y because we assume 'landscape' by default
        // scale factor is to give us a value which is similar to the PC test devices
        _motionParam.x = values[1] * 0.2f;
        _motionParam.y = -values[0] * 0.2f;
        _motionParam.z = values[2] * 0.2f;
        _wndGeneric.genericMotionEvent(0, EMotionEvent.Accelerometer, _motionParam);
        */
    }

    public void postEventMessage(final int msg) {
        Msg.send(_wndMT, msg, null, null);
    }

    public void postEventMessage(final int msg, final Object a) {
        Msg.send(_wndMT, msg, a, null);
    }

    public void postEventMessage(final int msg, final Object a, final Object b) {
        Msg.send(_wndMT, msg, a, b);
    }

    public void postInputMessage(final int msg) {
    	if (_wndGeneric != null) {
    		_wndGeneric.genericSendMessage(msg, null, null);
    	}
    }

    public void postInputMessage(final int msg, final Object a) {
    	if (_wndGeneric != null) {
    		_wndGeneric.genericSendMessage(msg, a, null);
    	}
    }

    public void postInputMessage(final int msg, final Object a, final Object b) {
    	if (_wndGeneric != null) {
    		_wndGeneric.genericSendMessage(msg, a, b);
    	}
    }
}
