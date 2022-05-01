package ni.android.util;

import ni.common.util.Log;
import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;

public class ProgressRunnable extends AsyncTask<Runnable,Integer,Boolean> {
    final String message;
    final Context context;
    ProgressDialog progress;

    private ProgressRunnable(final Context aContext, final String aMessage) {
        context = aContext;
        message = aMessage;
    }

    @Override
    protected Boolean doInBackground(final Runnable... runnables) {
        for (final Runnable r : runnables) {
            try {
                r.run();
            }
            catch (final Exception e) {
                Log.e(e);
            }
        }
        return true;
    }

    @Override
    protected void onPreExecute() {
        progress = new ProgressDialog(context);
        progress.setMessage(message);
        progress.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        progress.setCancelable(false);
        progress.show();
    }

    @Override
    protected void onPostExecute(final Boolean result) {
        super.onPostExecute(result);
        try {
            progress.dismiss();
        }
        catch (final Exception e) {
            Log.e(e);
        }
    }

    static public void run(final String aMessage, final Context aContext, final Runnable aRunnable)
    {
        (new ProgressRunnable(aContext, aMessage)).execute(aRunnable);
    }
}
