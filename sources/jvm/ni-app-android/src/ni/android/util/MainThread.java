package ni.android.util;

import java.util.concurrent.Callable;

import ni.common.util.Log;
import android.content.Context;
import android.os.Handler;

public class MainThread {

    private MainThread() {
        // Utility class.
    }

    /**
     * Executed in the main thread, returns the result of an execution. Anything
     * that runs here should finish quickly to avoid hanging the UI thread.
     */
    public static <T> T run(final Context context, final Callable<T> task) {
        final FutureResult<T> result = new FutureResult<T>();
        final Handler handler = new Handler(context.getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                try {
                    result.set(task.call());
                }
                catch (final Exception e) {
                    Log.e(e);
                    result.set(null);
                }
            }
        });
        try {
            return result.get();
        }
        catch (final InterruptedException e) {
            Log.e(e);
        }
        return null;
    }

    public static void run(final Context context, final Runnable task) {
        final Handler handler = new Handler(context.getMainLooper());
        handler.post(new Runnable() {
            @Override
            public void run() {
                task.run();
            }
        });
    }
}
