package ni.app;

import ni.niUI.EUIMessage;
import ni.niUI.IWidget;

public interface AppEventHandler {

    /**
     * Called after the system has been initialized to get the application's
     * configuration.
     * <p>
     * This is the place to set the initial configuration of the application
     * nothing has been initialized yet ; no valid window nor UI context are
     * available yet.
     */
    public void onInitConfig(final BaseAppWindow appWindow, final AppConfig appConfig);

    /**
     * Called after the OS window has been created and the basic object have
     * been initialized.
     * <p>
     * This is a good place to put custom assets loading and initial
     * initialization, at this point all objects and UI contexts are available.
     * 
     * @return true if startup succeeded, false if it failed and the application
     *         cannot start
     */
    public boolean onStartup(final BaseAppWindow appWindow);

    /**
     * Called before the window is destroyed/closed/shutdown.
     * <p>
     * The method called before the application terminates. Mobile platforms
     * don't always call this method, for example the method wont be called if
     * the application is forcefully discarded from memory (SIGKILL, etc...)
     * which is quite common on platforms such as Android and iOS.
     */
    public void onShutdown(final BaseAppWindow appWindow);

    /**
     * Called when the OS window receives a message.
     * 
     * @return true to notify that the message has been handled and shouldn't be
     *         propagated.
     */
    public boolean onWindowMessage(final BaseAppWindow appWindow, final int msg, final Object varA, final Object varB) throws Exception;

    /**
     * Called when the root widget receives a message.
     * <p>
     * Generally used in response to {@link EUIMessage.ContextAfterDraw} to draw
     * debug overlays (such as a FPS counter) after everything else as been
     * drawn.
     * 
     * @return true to notify that the message has been handled and shouldn't be
     *         propagated.
     */
    public boolean onRootMessage(final BaseAppWindow appWindow, final IWidget w, final int msg, final Object a, final Object b) throws Exception;
}
