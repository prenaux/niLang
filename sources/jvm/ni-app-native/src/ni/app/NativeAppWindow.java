package ni.app;

import java.io.IOException;

import ni.NiException;
import ni.niLang.EOSWindowStyleFlags;
import ni.niLang.Lib;
import ni.types.Vec4i;

class NativeAppWindow extends BaseAppWindow {
    NativeAppWindow(final AppEventHandler eventHandler) throws NiException, IOException {
        super(eventHandler);
        construct();
    }

    @Override
    protected AppConfig createAppConfig() {
        return new AppConfig();
    }

    @Override
    protected void initWindowAndGraphicsContext() throws NiException {
        //// Create the window ////
        Vec4i r = _config.windowRect;
        if (r == null) {
            r = lang.getMonitorRect(0);
            r.setWidth((int)(r.getWidth() * 0.75));
            r.setHeight((int)(r.getHeight() * 0.75));
            if ((r.getWidth() < 10) || (r.getHeight() < 10)) {
                r.set(0, 0, 1024, 768);
            }
        }

        _wnd = lang.createWindow(
                null,
                _config.windowTitle,
                r,
                0,
                EOSWindowStyleFlags.Regular);
        if (_wnd == null)
            throw new NiException("Can't create the application's window.");

        if (_config.windowCenter) {
            _wnd.centerWindow();
        }
        if (_config.windowFullScreen >= 0) {
            _wnd.setFullScreen(_config.windowFullScreen);
        }

        //// Create the graphics context ////
        if (!_graphics.initializeDriver(Lib.hstr(_config.renDriver)))
            throw new NiException("Can't initialize graphics driver.");

        _graphicsContext = _graphics.createContextForWindow(
                _wnd,
                _config.renBBFmt,
                _config.renDSFmt,
                _config.renSwapInterval,
                _config.renBBFlags);
        if (_graphicsContext == null)
            throw new NiException("Can't create graphics context.");
    }
}
