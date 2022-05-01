package ni.android.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.lang.reflect.Method;

import ni.common.util.Files;
import ni.common.util.Hash;
import ni.common.util.Log;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;

public class FileUtils extends Files {

    private FileUtils() {
        // Utility class.
    }

    static public boolean externalStorageMounted() {
        final String state = Environment.getExternalStorageState();
        return Environment.MEDIA_MOUNTED.equals(state)
               || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state);
    }

    public static File getExternalDownload() {
        try {
            final Class<?> c = Class.forName("android.os.Environment");
            final Method m = c.getDeclaredMethod("getExternalStoragePublicDirectory", String.class);
            final String download = c.getDeclaredField("DIRECTORY_DOWNLOADS").get(null).toString();
            return (File)m.invoke(null, download);
        }
        catch (final Exception e) {
            return new File(Environment.getExternalStorageDirectory(), "Download");
        }
    }

    public static String readFromAssetsFile(final Context context, final String name) throws IOException
    {
        final AssetManager am = context.getAssets();
        final BufferedReader reader = new BufferedReader(new InputStreamReader(am.open(name)));
        String line;
        final StringBuilder builder = new StringBuilder();
        while ((line = reader.readLine()) != null) {
            builder.append(line);
        }
        reader.close();
        return builder.toString();
    }

    public static File getHomeDir(final Context context) {
        return context.getDir("home", Context.MODE_PRIVATE);
    }

    public static File getDownloadsDir(final Context context) {
        return context.getDir("downloads", Context.MODE_PRIVATE);
    }

    public static File getTempDir(final Context context) {
        return context.getDir("temp", Context.MODE_PRIVATE);
    }

    public static File getCacheDir(final Context context) {
        File cacheDir;
        // Find the dir to save cached images
        if (android.os.Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED))
            cacheDir = new File(android.os.Environment.getExternalStorageDirectory(), "niApp/cache");
        else
            cacheDir = context.getCacheDir();
        if (!cacheDir.exists())
            cacheDir.mkdirs();
        return cacheDir;
    }

    public static File getCacheFile(final File cacheDir, final String urlOrAbsPath) {
        return new File(cacheDir, Hash.MD5(urlOrAbsPath));
    }

    public static File getCacheFile(final Context context, final String urlOrAbsPath) {
        return new File(getCacheDir(context), Hash.MD5(urlOrAbsPath));
    }

    static public boolean copyFromAssets(final AssetManager assets, final String astrName, final String astrSource, final String astrTarget)
    {
        final String strSrcFile = astrSource + astrName;
        final String strTargetFile = astrTarget + astrName;
        try {
            InputStream in = assets.open(strSrcFile);
            final File outFile = new File(strTargetFile);
            if (!outFile.exists()) {
                outFile.createNewFile();

                OutputStream out = new FileOutputStream(outFile);
                final byte[] buf = new byte[1024];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
                out.close();
                out = null;
            }
            in.close();
            in = null;
        }
        catch (final Exception e) {
            Log.d(e.getMessage());
            return false;
        }
        return true;
    }

    static public boolean copyFromAssetsDir(final AssetManager assets, final String astrSource, final String astrTarget)
    {
        try {
            final String[] list = assets.list(astrSource);
            final File fpDir = new File(astrTarget);
            if (!fpDir.exists())
                fpDir.mkdir();

            for (final String src : list) {
                final int nLastIndex = src.lastIndexOf("/");
                final String strFile = src.substring(nLastIndex + 1, src.length());
                if (!copyFromAssets(assets, strFile, astrSource + "/", astrTarget))
                    return false;
            }
        }
        catch (final Exception e) {
            Log.d(e.getMessage());
        }
        return true;
    }
}
