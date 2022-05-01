package ni.app;

import static ni.common.math.Vec.Vec4f;
import static ni.common.math.Rect.Rectf;

import java.io.FileNotFoundException;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;
import java.util.concurrent.ConcurrentLinkedQueue;

import ni.NiException;
import ni.common.lang3.Validate;
import ni.common.util.Log;
import ni.niLang.EOSWindowMessage;
import ni.niLang.EOSWindowShowFlags;
import ni.niLang.IConcurrent;
import ni.niLang.IConsole;
import ni.niLang.IHString;
import ni.niLang.ILang;
import ni.niLang.IMath;
import ni.niLang.IMessageHandler;
import ni.niLang.IMessageHandlerImpl;
import ni.niLang.IMessageQueue;
import ni.niLang.IOSWindow;
import ni.niLang.IProf;
import ni.niLang.IProfDraw;
import ni.niLang.IScriptingHost;
import ni.niLang.IScriptingHostImpl;
import ni.niLang.Lib;
import ni.niLang.util.IScriptingHostContextHandler;
import ni.niLang.util.Strings;
import ni.niUI.EFontFormatFlags;
import ni.niUI.EUIMessage;
import ni.niUI.IFont;
import ni.niUI.IGraphics;
import ni.niUI.IGraphicsContext;
import ni.niUI.IUIContext;
import ni.niUI.ICanvas;
import ni.niUI.IWidget;
import ni.niUI.IWidgetSink;
import ni.niUI.IWidgetSinkImpl;
import ni.niUI.widget.WidgetScriptingHostContextHandler;
import ni.types.IUnknown;
import ni.types.ULColor;
import ni.types.UUID;
import ni.types.Vec2f;
import ni.types.Vec4f;

public abstract class BaseAppWindow implements IMessageHandlerImpl, IWidgetSinkImpl {
	protected static ILang lang = Lib.getLang();
    protected static IConcurrent _concurrent = Lib.getConcurrent();

    protected final long _threadId;

	protected IMessageQueue _mq;
    protected IConsole _console;
    protected IMath _math;
    protected IOSWindow _wnd;
    protected IGraphics _graphics;
    protected IGraphicsContext _graphicsContext;
    protected IUIContext _uiContext;
    protected IWidget _rootWidget;
    protected IWidgetSink _widgetSink;

    protected AppEventHandler _eventHandler;
    protected ConcurrentLinkedQueue<Runnable> _queue = new ConcurrentLinkedQueue<Runnable>();

    public AppConfig _config;
    protected IProf _prof;
    protected IProfDraw _profDraw;
    protected IFont _profFont;

    protected boolean isDisposed = false;

    protected BaseAppWindow(final AppEventHandler eventHandler) {
        Validate.notNull(eventHandler);
        _threadId = _concurrent.getCurrentThreadID();
        _eventHandler = eventHandler;
    }

    /**
     * Construct the AppWindow should be called in the constructor of the
     * implementation class
     *
     * @throws NiException
     * @throws FileNotFoundException
     */
    protected final void construct() throws NiException, FileNotFoundException {
        // Init the main message queue
        _mq = _concurrent.createMessageQueue(_concurrent.getCurrentThreadID(), 0xFFFF);

        // Init profiler
        _prof = Lib.getLang().getProf();

        // Init config
        _config = createAppConfig();
        _eventHandler.onInitConfig(this, _config);

        // Init scripting host
        lang.addScriptingHost(Lib.hstr("java"), IScriptingHost.impl(new ScriptingHost()));

        // Init console
        _console = Lib.getConsole();

        // Init math
        _math = Lib.getMath();
        if (_math == null)
            throw new NiException("Can't create the Math object.");

        // Init graphics
        _graphics = IGraphics.query(Lib.createGlobalInstance("niUI.Graphics"));
        if (_graphics == null)
            throw new NiException("Can't create Graphics object.");

        // Init window and graphics context
        initWindowAndGraphicsContext();
        assert _wnd != null;
        assert _graphicsContext != null;
        lang.setGlobalInstance("iGraphicsContext", _graphicsContext);

        // Load fonts
        _graphics.registerSystemFonts();

        // Init UI context
        _uiContext = IUIContext.query(Lib.createInstance("niUI.UIContext",_graphicsContext,_wnd.getContentsScale()));
        lang.setGlobalInstance("iUIContext", _uiContext);

        registerScriptingHostContextHandler(WidgetScriptingHostContextHandler.CONTEXT_NAME, new WidgetScriptingHostContextHandler());

        _widgetSink = IWidgetSink.impl(this);
        _rootWidget = _uiContext.getRootWidget();

        // Init window
        _wnd.getMessageHandlers().addSink(IMessageHandler.impl(this));
        _wnd.setRefreshTimer(_config.windowRefreshTimer);
        _wnd.setShow(EOSWindowShowFlags.Show | _config.windowShowFlags);
        _wnd.setFocus();
        _wnd.activateWindow();
        lang.setGlobalInstance("iOSWindow", _wnd);

        // Call the startup method
        if (!_eventHandler.onStartup(this)) {
            throw new NiException("Application onStartup failed.");
        }

        // Sink attached is the last thing that we do, so that the SinkAttached message is sent when everything
        // has been initialized.
        _rootWidget.addSink(_widgetSink);
    }

    @Override
    protected void finalize() {
        if (!isDisposed) {
            Log.e("Window should have been disposed.");
            dispose();
        }
    }

    /**
     * Dispose the window object, called when the window is destroyed.
     * <p>
     * You shouldn't call this method to close the application window instead
     * use {@link #close} to do that.
     */
    protected void dispose() {
        if (!isDisposed) {
            _eventHandler.onShutdown(this);
            _eventHandler = null;
            if (_rootWidget != null) {
                _rootWidget.dispose();
                _rootWidget = null;
            }
            if (_widgetSink != null) {
                _widgetSink.invalidate();
                _widgetSink.dispose();
                _widgetSink = null;
            }
            if (_uiContext != null) {
                _uiContext.invalidate();
                _uiContext.dispose();
                _uiContext = null;
            }
            if (_graphicsContext != null) {
                _graphicsContext.invalidate();
                _graphicsContext.dispose();
                _graphicsContext = null;
            }
            if (_graphics != null) {
                _graphics.invalidate();
                _graphics.dispose();
                _graphics = null;
            }
            if (_wnd != null) {
                _wnd.invalidate();
                _wnd.dispose();
                _wnd = null;
            }
            if (_console != null) {
                _console.invalidate();
                _console.dispose();
                _console = null;
            }
            if (_math != null) {
                _math.invalidate();
                _math.dispose();
                _math = null;
            }
            isDisposed = true;
        }
    }

    /**
     * Request the OS Window to close the application.
     * <p>
     * This method is the one to use to cleanly 'exit' the application.
     */
    public void close() {
        _wnd.setRequestedClose(true);
    }

    /**
     * Create the default App config.
     * <p>
     * This is called from the constructor so you can't rely on your class's
     * members in there since they won't be initialized yet.
     */
    protected abstract AppConfig createAppConfig() throws NiException;

    /**
     * Create an OS dependent window.
     * <p>
     * This is called from the constructor so you can't rely on your class's
     * members in there since they won't be initialized yet.
     *
     * @return the window created
     */
    protected abstract void initWindowAndGraphicsContext() throws NiException;

    /**
     * Queue a runnable to be run on the main UI thread
     */
    public void queue(final Runnable runnable) {
        _queue.add(runnable);
    }

    /**
     * Should be called to update the window
     */
    protected boolean update() {
        // update the Java message queue
        {
            Runnable runnable = _queue.poll();
            while (runnable != null) {
                try {
                    runnable.run();
                }
                catch (final Throwable t) {
                    Log.e("Exception in queue", t);
                }
                runnable = _queue.poll();
            }
        }
        // update the window... dispatch queue input events, etc...
        _wnd.updateWindow(true);
        // update the system message queue
        while (_mq.pollAndDispatch()) {}

        return !_wnd.getRequestedClose();
    }

    private void doDraw() {
        if ((_uiContext == null) || (_graphicsContext == null))
            return;

        final double frameTime = lang.getFrameTime();

        // service all scripting hosts
        lang.serviceAllScriptingHosts(false);
        // update the main executor
        _concurrent.getExecutorMain().update(10);
        // update the console
        _console.popAndRunAllCommands();

        // update rendering
        if (_config.windowDrawWhenInactive || _wnd.getIsActive()) {
            _uiContext.resize(Rectf(0,0,_graphicsContext.getWidth(),_graphicsContext.getHeight()),
                              _wnd.getContentsScale());
            _uiContext.update((float)frameTime);
        }

		if (_config.clearBuffers > 0) {
			_graphicsContext.clearBuffers(_config.clearBuffers, _config.clearColor, 1.0f, 0);
		}
        _uiContext.draw();
        _graphicsContext.display(0, null);

        lang.updateFrameTime(lang.timerInSeconds());
        if (_prof != null) {
            _prof.update();
        }
    }

    /**
     * Should be called to draw the window's content
     */
    protected void draw() {
        if (_config.windowDrawWhenInactive || _wnd.getIsActive()) {
            doDraw();
        }
    }

    final public IConsole getConsole() {
        return _console;
    }

    final public AppConfig getConfig() {
        return _config;
    }

    final public IOSWindow getOSWindow() {
        return _wnd;
    }

    final public IGraphics getGraphics() {
        return _graphics;
    }

    final public IGraphicsContext getGraphicsContext() {
        return _graphicsContext;
    }

    final public IUIContext getUIContext() {
        return _uiContext;
    }

    final public AppEventHandler getEventHandler() {
        return _eventHandler;
    }

    final public IProf getProf() {
    	return _prof;
    }

    @Override
    public long getThreadID() {
        return _threadId;
    }

    @Override
    public void handleMessage(final int msgId, final Object varA, final Object varB) {
        try {
            if (_eventHandler.onWindowMessage(this, msgId, varA, varB))
                return;
        }
        catch (final Exception ex) {
            Log.e("Window Message Handler Exception : " + Strings.fromMessage(_wnd, msgId, varA, varB), ex);
            return;
        }

        if (msgId == EOSWindowMessage.Close) {
            Log.i("Default Close");
            _wnd.setRequestedClose(true);
            return;
        }
        else if (msgId == EOSWindowMessage.SwitchIn) {
            _wnd.setRefreshTimer(_config.windowRefreshTimer);
        }
        else if (msgId == EOSWindowMessage.SwitchOut) {
            _wnd.setRefreshTimer(-1);
        }
        else if (msgId == EOSWindowMessage.Paint) {
            if (!_config.windowDrawWhenInactive && !_wnd.getIsActive()) {
                doDraw();
            }
            return;
        }

        _uiContext.sendWindowMessage(msgId, varA, varB);
    }

    @Override
    public boolean onWidgetSink(final IWidget w, final int msg, final Object a, final Object b)
    {
        try {
            if (w == null || w.equals(_rootWidget)) {
                final boolean ret = _eventHandler.onRootMessage(this, w, msg, a, b);
                if (!ret) {
					if (msg == EUIMessage.ContextAfterDraw) {
                        if (_config.drawFps != 0) {
                        	final IWidget rootWidget = _uiContext.getRootWidget();
                            final ICanvas canvas = ICanvas.query(a);
                            final Vec4f rect = rootWidget.getAbsoluteRect();

                            if (_profFont == null) {
                            	_profFont = rootWidget.getFont().createFontInstance(null);
                            	_profFont.setSizeAndResolution(new Vec2f(), 16, _uiContext.getContentsScale());
                            	_profFont.setColor(~0);
                            }

							final IProf prof = Lib.getLang().getProf();
							if (prof != null && _config.drawFps == 1) {
                            	if (_profDraw == null) {
                            		_profDraw = _uiContext.createProfDraw(canvas, _profFont);
                            	}
                                prof.drawGraph(
                                    _profDraw,
                                    5, rect.getHeight(),
                                    2, 10);
                                prof.drawTable(
                                    _profDraw,
                                    5, 5,
                                    rect.getWidth()*0.4f, rect.getHeight(),
                                    3);
                            }

                        	{
                        		_profFont.setColor(~0);
	                            final float fh = _profFont.getHeight();
	                            final Vec4f posRect = Vec4f(5, rect.getHeight() - fh - 5, 0, 0);
	                            final String fpsText = "FPS:" +
	                                                   lang.getAverageFrameRate() +
	                                                   " (" +
	                                                   ((int)(lang.getFrameTime() * 1000.0)) +
	                                                   ")";
	                            canvas.blitText(_profFont, posRect, EFontFormatFlags.Border, fpsText);
	                            canvas.flush();
                        	}
                        }
                    }
                }
                return ret;
            }
        }
        catch (final Exception ex) {
            Log.e("Widget Message Handler Exception : " + Strings.fromMessage(w, msg, a, b), ex);
            if (msg == EUIMessage.SinkAttached)
                throw new RuntimeException(ex);
        }
        return false;
    }

    private static final Map<String, IScriptingHostContextHandler> scriptingHostContextHandlers =
    		new TreeMap<String, IScriptingHostContextHandler>();

    public boolean registerScriptingHostContextHandler(String contextName, IScriptingHostContextHandler handler) {
    	scriptingHostContextHandlers.put(contextName, handler);
    	return true;
    }

    private class ScriptingHost implements IScriptingHostImpl {

        @Override
        public boolean evalString(final IHString ahspContext, final String aaszCode) {
            return false;
        }

        @Override
        public boolean canEvalImpl(final IHString ahspContext, final IHString ahspCodeResource)
        {
        	final String className = ahspCodeResource.getChars();
        	return Strings.endsWith(className,	".java");
        }

        @Override
        public IUnknown evalImpl(final IHString ahspContext, final IHString ahspCodeResource, final UUID aIID)
        {
            String className = ahspCodeResource.getChars();
            if (!Strings.endsWith(className,	".java")) {
            	Log.e("Invalid className for JVM class: " + className);
            	return null;
            }
            className = Strings.beforeLast(className, ".java");

            String context = ahspContext.getChars();
            IScriptingHostContextHandler contextHandler = scriptingHostContextHandlers.get(context);
            if (contextHandler == null) {
            	Log.e("No context handler register for context '"+context+"' for class: " + className);
            	return null;
            }

            try {
            	final IUnknown o = contextHandler.evalImpl(className);
            	return o;
            }
            catch (final Throwable t) {
                Log.e("Can't create an instance of :" + className, t);
            }
            return null;
        }

        @Override
        public void service(final boolean abForceGC) {
        }
    }
}
