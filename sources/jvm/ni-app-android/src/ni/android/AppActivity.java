package ni.android;

import ni.android.gl.GLOptions;
import ni.android.util.Dialogs;
import ni.android.view.AppView;
import ni.android.view.AppViewGL;
import ni.app.AppConfig;
import ni.app.AppEventHandler;
import ni.app.BaseAppWindow;
import ni.niLang.EOSWindowMessage;
import ni.niUI.IWidget;
import ni.niUI.EClearBuffersFlags;
import android.app.Activity;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

public class AppActivity extends Activity implements AppEventHandler {

    private final int _viewLayoutResourceId;
    private final int _viewResourceId;
    protected AppViewGL _view;

    /**
     * Default constructor that will set a {@link AppView} as content view
     * (with {@code this.setContentView})
     */
    public AppActivity() {
        _viewLayoutResourceId = -1;
        _viewResourceId = -1;
    }

    /**
     * Constructor to use to specify a layout from an Android layout XML.
     *
     * @param aSelenaLayoutResourceId is the layout resource id, ex:
     *            {@code R.layout.main}
     * @param aSelenaViewResourceId is the id of the view in the layout, ex:
     *            {@code R.id.surfaceView1}
     */
    public AppActivity(final int aSelenaLayoutResourceId, final int aSelenaViewResourceId) {
        _viewLayoutResourceId = aSelenaLayoutResourceId;
        _viewResourceId = aSelenaViewResourceId;
    }

    /**
     * Called when the {@link EOSWindowMessage#Close} message is received by the
     * window in {@link #onWindowMessage}.
     * <p>
     * By default shows an OK/Cancel dialog to confirm that the user wants to
     * exit the activity.
     * <p>
     * To change this behavior you can override the method and call appWindow's
     * {@link BaseAppWindow#close} to close the window, ex:
     * {@code appWindow.close()}.
     *
     * @param appWindow {@link BaseAppWindow}
     */
    protected void tryClose(final BaseAppWindow appWindow) {
        Dialogs.alertOkCancel(this, "Do you want to exit '" + appWindow.getOSWindow().getTitle() + "' ?",
                new Runnable() {
                    @Override
                    public void run() {
                        appWindow.close();
                        android.os.Process.killProcess(android.os.Process.myPid());
                    }
                },
                null);
    }

    /**
     * Set the activity to FullScreen, must be called in onCreate BEFORE
     * super.onCreate().
     */
    public void setFeatureFullScreen() {
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
    }

    /**
     * Set the activity to keep the screen on, must be called in onCreate BEFORE
     * super.onCreate().
     */
    public void setFeatureKeepScreenOn() {
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    @Override
    public void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Setup view...
        if (_viewLayoutResourceId > 0) {
            setContentView(_viewLayoutResourceId);
            _view = (AppViewGL)findViewById(_viewResourceId);
        }
        else {
            _view = new AppView(this);
            setContentView(_view);
        }
        if (_view != null) {
            _view.startup(this);
        }
    }

    @Override
    public void onInitConfig(final BaseAppWindow appWindow, final AppConfig appConfig)
    {
    	if (GLOptions.TRANSLUCENT_WINDOW) {
    		appConfig.clearBuffers &= ~EClearBuffersFlags.Color;
    	}
    	if (!GLOptions.USE_GL_ES2) {
    		appConfig.renDriver = "GL1";
    	}
    	else {
    		appConfig.renDriver = "GL2";
    	}
    	if (GLOptions.USE_COLOR_BUFFER_32) {
    		appConfig.renBBFmt = "R8G8B8A8";
    	}
    	if (GLOptions.USE_DEPTH_BUFFER_24) {
    		appConfig.renDSFmt = "D24X8";
    	}
    }

    @Override
    public boolean onStartup(final BaseAppWindow appWindow) {
        return true;
    }

    @Override
    public void onShutdown(final BaseAppWindow appWindow) {
    }

    @Override
    public boolean onWindowMessage(final BaseAppWindow appWindow, final int msg, final Object varA, final Object varB) throws Exception
    {
        if (msg == EOSWindowMessage.Close) {
            tryClose(appWindow);
            return true;
        }
        return false;
    }

    @Override
    public boolean onRootMessage(final BaseAppWindow appWindow, final IWidget w, final int msg, final Object a, final Object b) throws Exception
    {
        return false;
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (_view != null) {
            _view.activityOnPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (_view != null) {
            _view.activityOnResume();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (_view != null) {
            _view.activityOnDestroy();
        }
    }
}
