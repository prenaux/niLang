package ni.android.util;

import android.content.Context;

public interface IDownloadContext {
    public Context getContext();

    public void onDownloadCompleted(boolean succeeded, int id, String src, String dest);
}
