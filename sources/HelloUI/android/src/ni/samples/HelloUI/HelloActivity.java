package ni.samples.HelloUI;

import ni.android.AppActivity;
import ni.app.AppConfig;
import ni.app.BaseAppWindow;
import ni.niUI.IWidgetSink;
import android.os.Bundle;

public class HelloActivity extends AppActivity {
    @Override
    public void onCreate(final Bundle savedInstanceState) {
        setFeatureFullScreen();
        setFeatureKeepScreenOn();
        super.onCreate(savedInstanceState);
    }

    @Override
    public void onInitConfig(final BaseAppWindow appWnd, final AppConfig cfg) {
    	cfg.drawFps = 1;
    	super.onInitConfig(appWnd, cfg);
    }

    @Override
    public boolean onStartup(final BaseAppWindow appWnd) {
        appWnd.getUIContext().getRootWidget().addSink(IWidgetSink.impl(new HelloWidget()));
        return true;
	}
}
