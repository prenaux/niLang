package ni.app;

import ni.common.util.Log;
import ni.common.util.Strings;
import ni.niLang.ILang;
import ni.niLang.Lib;

public final class App {
    private App() {
    }

    static public void run(final AppEventHandler eventHandler) {
        try {
            final NativeAppWindow app = new NativeAppWindow(eventHandler);
            while (app.update()) {
                app.draw();
            }
            app.dispose();
            // Make sure all threads are terminated and the application exits
            System.exit(0);
        }
        catch (final Exception e) {
            final ILang sys = Lib.getLang();
            if (sys != null) {
                Log.e("Fatal Error", e);
                sys.fatalError(Strings.getStackTrace(e));
            }
        }
    }
}
