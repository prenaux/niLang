package ni.android.util;

import java.io.RandomAccessFile;

import ni.common.util.FetchUrl;
import ni.common.util.Log;
import android.app.ProgressDialog;
import android.os.AsyncTask;

public class ProgressDownloadLargeFile extends AsyncTask<String,Integer,Boolean> implements FetchUrl.EventReceiver {
    final int dlId;
    final String dlMessage;
    final String dlSrc;
    final String dlDest;
    final IDownloadContext parent;
    final boolean useLog;
    final FetchUrl fetch = new FetchUrl(0);
    ProgressDialog progress;

    private ProgressDownloadLargeFile(final IDownloadContext aParent, final String aDlMessage, final int aDlId, final String aDlSrc, final String aDlDest, final boolean aUseLog) {
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
            if (!fetch.fetchUrlEx(dlSrc, dlDest, useLog ? (dlDest + ".dllog") : null, this)) {
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
        progress.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        progress.setMax(10000);
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
        final ProgressDownloadLargeFile theTask = new ProgressDownloadLargeFile(
                aParent,
                aDlMessage,
                aDlId,
                aDlSrc,
                aDlDest,
                aUseLog);
        theTask.execute();
    }

    @Override
    public void onProgress(final String url, final long current, final long total) {
        // Log.d("onProgress: " + url + ", " + current + ", " + total);
        if (total > 0) {
            progress.setMax((int)total);
            progress.setProgress((int)current);
        }
    }

    @Override
    public void onError(final String url, final int statusCode) {
        // Log.d("onError: " + url + ", " + statusCode);
    }

    @Override
    public void onCompleted(final String url, final RandomAccessFile output, final RandomAccessFile log)
    {
        // Log.d("onCompleted: " + url + ", " + output);
    }

    @Override
    public void onTimeout(final String url) {
        // Log.d("onTimeout: " + url);
    }

    @Override
    public void onBeginReceivingBody(final String url) {
        // Log.d("onBeginReceivingBody: " + url);
    }

    @Override
    public void onReceiveBody(final String url, final byte[] data, final int length) {
        // Log.d("onReceiveBody: " + url + ", " + length);
    }
}
