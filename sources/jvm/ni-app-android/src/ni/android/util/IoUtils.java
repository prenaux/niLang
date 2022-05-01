package ni.android.util;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import ni.common.util.Log;

public class IoUtils {
    private static final int BUFFER_SIZE = 1024 * 8;

    private IoUtils() {
        // Utility class.
    }

    public static int copy(final InputStream input, final OutputStream output) throws Exception, IOException
    {
        final byte[] buffer = new byte[BUFFER_SIZE];

        final BufferedInputStream in = new BufferedInputStream(input, BUFFER_SIZE);
        final BufferedOutputStream out = new BufferedOutputStream(output, BUFFER_SIZE);
        int count = 0, n = 0;
        try {
            while ((n = in.read(buffer, 0, BUFFER_SIZE)) != -1) {
                out.write(buffer, 0, n);
                count += n;
            }
            out.flush();
        }
        finally {
            try {
                out.close();
            }
            catch (final IOException e) {
                Log.e(e.getMessage(), e);
            }
            try {
                in.close();
            }
            catch (final IOException e) {
                Log.e(e.getMessage(), e);
            }
        }
        return count;
    }

    public static String streamToString(final InputStream input) throws IOException, Exception
    {
        final ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        if (copy(input, buffer) > 0) {
            return buffer.toString();
        }
        else {
            return "";
        }
    }
}
