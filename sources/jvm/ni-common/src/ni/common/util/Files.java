package ni.common.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;

public class Files {
    protected Files() {
    }

    public final static int chmod(final File path, final int mode) throws Exception {
        final Class<?> fileUtils = Class.forName("android.os.FileUtils");
        final Method setPermissions = fileUtils.getMethod(
                "setPermissions",
                String.class,
                int.class,
                int.class,
                int.class);
        return (Integer)setPermissions.invoke(
                null,
                path.getAbsolutePath(),
                mode,
                -1,
                -1);
    }

    public final static boolean recursiveChmod(final File root, final int mode) throws Exception
    {
        boolean success = chmod(root, mode) == 0;
        for (final File path : root.listFiles()) {
            if (path.isDirectory()) {
                success = recursiveChmod(path, mode);
            }
            success &= (chmod(path, mode) == 0);
        }
        return success;
    }

    public final static boolean delete(final File path) {
        boolean result = true;
        if (path.exists()) {
            if (path.isDirectory()) {
                for (final File child : path.listFiles()) {
                    result &= delete(child);
                }
                result &= path.delete(); // Delete empty directory.
            }
            if (path.isFile()) {
                result &= path.delete();
            }
            if (!result) {
                Log.e("Delete failed;");
            }
            return result;
        }
        else {
            Log.e("File does not exist.");
            return false;
        }
    }

    public final static File copyFromStream(final String name, final InputStream input) {
        if ((name == null) || (name.length() == 0)) {
            Log.e("No script name specified.");
            return null;
        }
        final File file = new File(name);
        if (!makeDirectories(file.getParentFile(), 0755))
            return null;
        try {
            final OutputStream output = new FileOutputStream(file);
            Streams.copy(input, output);
        }
        catch (final Exception e) {
            Log.e(e);
            return null;
        }
        return file;
    }

    public final static boolean makeDirectories(final File directory, final int mode) {
        File parent = directory;
        while ((parent.getParentFile() != null) && !parent.exists()) {
            parent = parent.getParentFile();
        }
        if (!directory.exists()) {
            Log.v("Creating directory: " + directory.getName());
            if (!directory.mkdirs()) {
                Log.e("Failed to create directory.");
                return false;
            }
        }
        try {
            recursiveChmod(parent, mode);
        }
        catch (final Exception e) {
            Log.e(e);
            return false;
        }
        return true;
    }

    public final static boolean rename(final File file, final String name) {
        return file.renameTo(new File(file.getParent(), name));
    }

    public final static String readToString(final String filePath) {
        try {
            final File fp = new File(filePath);
            return readToString(fp);
        }
        catch (final IOException e) {
        }
        return "";
    }

    public final static String readToString(final File file) throws IOException {
        if ((file == null) || !file.exists())
            return null;
        final FileReader reader = new FileReader(file);
        final StringBuilder out = new StringBuilder();
        final char[] buffer = new char[1024 * 4];
        int numRead = 0;
        while ((numRead = reader.read(buffer)) > -1) {
            out.append(String.valueOf(buffer, 0, numRead));
        }
        reader.close();
        return out.toString();
    }
}
