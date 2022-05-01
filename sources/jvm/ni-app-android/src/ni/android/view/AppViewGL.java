package ni.android.view;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import ni.android.gl.GLOptions;
import ni.android.util.Views;
import ni.app.AppEventHandler;
import ni.common.util.Log;
import ni.niLang.EOSWindowMessage;
import ni.niLang.EOSWindowSwitchReason;

public class AppViewGL extends GLSurfaceView implements GLSurfaceView.Renderer {

  protected AppEventHandler _eventHandler = null;
  protected AppWindow _appWnd = null;

  public AppViewGL(Context context) { super(context); }

  public AppViewGL(Context context, final AttributeSet attrs) {
    super(context,attrs);
  }

  public void startup(final AppEventHandler aEventHandler)
  {
    setEGLContextClientVersion(2);
    _eventHandler = aEventHandler;
    setFocusable(true);
    setFocusableInTouchMode(true);
    setRenderer(this);
  }

  @Override
  public void invalidate() {
  }

  public void pushConsoleCommand(final String astrCommand) {
    if (_appWnd == null) {
      Log.w("Trying to push console command without a valid window :" + astrCommand);
      return;
    }
    _appWnd.getConsole().pushCommand(astrCommand);
  }

  public boolean onKeyDown(final int keyCode, final KeyEvent event) {
    return sendKeyEvent(keyCode, true);
  }

  public boolean onKeyUp(final int keyCode, final KeyEvent event) {
    return sendKeyEvent(keyCode, false);
  }

  protected boolean sendKeyEvent(final int keyCode, final boolean isDown) {
    if (_appWnd != null) {
      return _appWnd.inputKey(keyCode, isDown);
    }
    return false;
  }

  public boolean dispatchTouchEvent(final MotionEvent event) {
    if (_appWnd != null) {
      _appWnd.inputTouch(event);
    }
    return true;
  }

  /**
   * Should be called by the View's owner to notify that the activity has been
   * paused.
   * <p>
   * Usually that would be in the Activity's onPause override.
   */
  public void activityOnPause() {
    Log.v("**** onPause");
    if (_appWnd != null) {
      _appWnd.postEventMessage(EOSWindowMessage.SwitchOut, EOSWindowSwitchReason.Deactivated);
    }
  }

  /**
   * Should be called by the View's owner to notify that the activity has
   * resumed.
   * <p>
   * Usually that would be in the Activity's onResume override.
   */
  public void activityOnResume() {
    if (_appWnd != null) {
      _appWnd.postEventMessage(EOSWindowMessage.SwitchIn, EOSWindowSwitchReason.Activated);
    }
    // yey, garantees that Android won't drop the frame rate in some cases...
    this.requestLayout();
  }

  /**
   * Should be called by the View's owner to notify that the activity is going
   * to be destroyed.
   * <p>
   * Usually that would be in the Activity's onDestroy override.
   */
  public void activityOnDestroy() {
    Log.v("**** onDestroy");
    if (_appWnd != null) {
      _appWnd.dispose();
      _appWnd = null;
    }
  }

  @Override
  public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
    Log.d("Surface Created");
  }

  @Override
  public void onSurfaceChanged(GL10 gl10, int width, int height) {
    synchronized (this) {
      if (_appWnd != null) {
        _appWnd.resize(width, height);
      }
    }
  }

  @Override
  public void onDrawFrame(GL10 gl10) {
    synchronized (this) {
      if (_appWnd == null) {
        if (_eventHandler != null) {
          try {
            /*
             * Created here because we need to have a valid OpenGL context to start the Driver
             */
            int w,h;
            if (GLOptions.FIXED_SIZE.x != 0) {
              w = GLOptions.FIXED_SIZE.x;
              h = GLOptions.FIXED_SIZE.y;
            }
            else {
              w = Views.getHolderWidth(this);
              h = Views.getHolderHeight(this);
            }
            _appWnd = new AppWindow(getContext(),_eventHandler,w,h);
            if (GLOptions.FIXED_SIZE.x != 0) {
              AppWindow._touchScaleX = (float)w / (float)Views.getHolderWidth(this);
              AppWindow._touchScaleY = (float)h / (float)Views.getHolderHeight(this);
            }
          }
          catch (final Exception e) {
            // don't attempt to start anymore...
            _eventHandler = null;
            Log.e(e);
            return;
          }
        }
        else {
          return;
        }
      }
      if (_appWnd != null) {
        _appWnd.draw(gl10);
      }
    }
  }
}
