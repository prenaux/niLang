package ni.android.util;

import ni.common.util.FetchUrl;
import ni.common.util.Log;
import android.app.ProgressDialog;
import android.os.AsyncTask;

public class ProgressDownloadSmallFile extends AsyncTask<String,Integer,Boolean> {
    final int dlId;
    final String dlMessage;
    final String dlSrc;
    final String dlDest;
    final IDownloadContext parent;
    final boolean useLog;
    final FetchUrl fetch = new FetchUrl(0);
    ProgressDialog progress;

    private ProgressDownloadSmallFile(final IDownloadContext aParent, final String aDlMessage, final int aDlId, final String aDlSrc, final String aDlDest, final boolean aUseLog) {
        parent = aParent;
        dlMessage = aDlMessage;
        dlId = aDlId;
        dlSrc = aDlSrc;
        dlDest = aDlDest;
        useLog = aUseLog;
    }

    @Override
    protected Boolean doInBackground(final String... urls) {
        try {
            if (!fetch.fetchUrl(dlSrc, dlDest, useLog ? (dlDest + ".dllog") : null)) {
                return false;
            }
        }
        catch (final Exception e) {
            Log.e(e);
            return false;
        }

        return true;
    }

    @Override
    protected void onPreExecute() {
        progress = new ProgressDialog(parent.getContext());
        progress.setMessage(dlMessage);
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
        try {
            parent.onDownloadCompleted(result, dlId, dlSrc, dlDest);
        }
        catch (final Exception e) {
            Log.e(e);
        }
    }

    public static void run(final IDownloadContext aParent, final String aDlMessage, final int aDlId, final String aDlSrc, final String aDlDest, final boolean aUseLog)
    {
        final ProgressDownloadSmallFile theTask = new ProgressDownloadSmallFile(
                aParent,
                aDlMessage,
                aDlId,
                aDlSrc,
                aDlDest,
                aUseLog);
        theTask.execute();
    }
}
