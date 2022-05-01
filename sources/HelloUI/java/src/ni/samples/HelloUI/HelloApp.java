package ni.samples.HelloUI;

import ni.app.AppConfig;
import ni.app.AppEventHandler;
import ni.app.BaseAppWindow;
import ni.common.util.Log;
import ni.niUI.IWidget;
import ni.niUI.IWidgetSink;
import ni.niUI.widget.WidgetSink;

public class HelloApp extends WidgetSink implements AppEventHandler {
    @Override
    public void onInitConfig(final BaseAppWindow appWnd, final AppConfig cfg) {
        cfg.windowTitle = "HelloUI - JVM - Desktop";
        cfg.drawFps = 1;
    }

    @Override
    public boolean onWindowMessage(final BaseAppWindow appWindow, final int msg, final Object varA, final Object varB) {
        return false;
    }

    @Override
    public boolean onRootMessage(final BaseAppWindow appWindow, final IWidget w, final int msg, final Object a, final Object b) throws Exception
    {
        return false;
    }

    @Override
    public boolean onStartup(final BaseAppWindow appWnd) {
    	Log.i("Hello App onStartup");
    	appWnd.getUIContext().getRootWidget().addSink(IWidgetSink.impl(new HelloWidget()));
        return true;
    }

    @Override
    public void onShutdown(final BaseAppWindow appWnd) {
    	Log.i("Hello App onShutdown");
    }

	public static void main(final String[] args) {
		ni.app.App.run(new HelloApp());
	}
}
