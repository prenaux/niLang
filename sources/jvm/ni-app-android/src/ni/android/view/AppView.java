package ni.android.view;

// import javax.microedition.khronos.egl.EGL10;

import ni.common.util.Log;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class AppView
        extends AppViewGL
        implements SensorEventListener
{
    // SensorManager, access to the accelerometer
    protected SensorManager mSensorManager = null;
    // other options: SensorManager.SENSOR_DELAY_FASTEST,
    // SensorManager.SENSOR_DELAY_NORMAL and
    // SensorManager.SENSOR_DELAY_UI
    protected int mSensorDelay = SensorManager.SENSOR_DELAY_GAME;

    public AppView(final Context context) {
        super(context);
        initialize(context);
    }

    public AppView(final Context context, final AttributeSet attrs) {
        super(context, attrs);
        initialize(context);
    }

    private void initialize(final Context context) {
        mSensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
    }

    @Override
    public void activityOnPause() {
        super.activityOnPause();
        Log.v("**** onPause");
        if (mSensorManager != null) {
            mSensorManager.unregisterListener(
                    this,
                    mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
        }
    }

    @Override
    public void activityOnResume() {
        super.activityOnResume();
        Log.v("**** onResume");
        if (mSensorManager != null) {
            mSensorManager.registerListener(
                    this,
                    mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                    mSensorDelay);
        }
    }

    //================================================================
    //
    // Sensor
    //
    //================================================================
    @Override
    public void onAccuracyChanged(final Sensor sensor, final int accuracy) {
    }

    @Override
    public void onSensorChanged(final SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            if (_appWnd != null) {
                _appWnd.inputAccelerometer(event.values);
            }
        }
    }

    //================================================================
    //
    // Soft Keyboard Input
    //
    //================================================================    
    @Override
    public InputConnection onCreateInputConnection(final EditorInfo outAttrs) {
        Log.d("onCreateInputConnection");

        outAttrs.actionLabel = null;
        outAttrs.label = "Test text";
        outAttrs.inputType = InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        outAttrs.imeOptions = EditorInfo.IME_ACTION_DONE |
                              EditorInfo.IME_FLAG_NO_EXTRACT_UI;

        return new MyInputConnection(this, true);
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return false;
    }

    public boolean icCommitText(final String input) {
        Log.d("icCommitText: " + input);
        if (_appWnd != null) {
            _appWnd.inputString(input);
        }
        return true;
    }

    public boolean icKeyEvent(final KeyEvent event) {
        // Send down/up in ACTION_DOWN because I'm pretty sure the up event won't
        // be send in some cases...
        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            Log.d("icKeyEvent: " + event.getKeyCode());
            final int keyCode = event.getKeyCode();
            // down event
            sendKeyEvent(keyCode, true);
            // up event
            sendKeyEvent(keyCode, false);
        }
        return true;
    }

    /*
     * MyInputConnection BaseInputConnection configured to be editable
     */
    class MyInputConnection extends BaseInputConnection {
        public final static String TAG = "MyInputConnection";

        public AppView mApp = null;

        public MyInputConnection(final AppView targetView, final boolean fullEditor) {
            super(targetView, fullEditor);
            mApp = targetView;
        }

        @Override
        public boolean commitText(final CharSequence text, final int newCursorPosition) {
            mApp.icCommitText(text.toString());
            return true;
        }

        @Override
        public boolean sendKeyEvent(final KeyEvent event) {
            Log.d("Event: " + event.toString());
            mApp.icKeyEvent(event);
            if ((event.getAction() == KeyEvent.ACTION_DOWN) && (event.getUnicodeChar() != 0)) {
                final String chars = new String(Character.toChars(event.getUnicodeChar()));
                if (chars != null) {
                    mApp.icCommitText(chars);
                }
            }
            return true;
        }
    }
}
