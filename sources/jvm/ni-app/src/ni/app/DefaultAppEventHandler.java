package ni.app;

import ni.niUI.IWidget;

public class DefaultAppEventHandler implements AppEventHandler {

    @Override
    public void onInitConfig(final BaseAppWindow appWindow, final AppConfig appConfig) {
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
        return false;
    }

    @Override
    public boolean onRootMessage(final BaseAppWindow appWindow, final IWidget w, final int msg, final Object a, final Object b) throws Exception
    {
        return false;
    }
}
