package ni.common.util;

import java.io.IOException;
import java.io.InputStream;
import java.io.RandomAccessFile;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class FetchUrl {
    /////////////////////////////////////////////////////////
    private static final int[] DOGTAG_SIZE1 = { 0xDE, 0xAD, 0xBE, 0xEF };
    private static final int[] DOGTAG_SIZE2 = { 0xCC, 0xCC, 0xCC, 0xCC };
    private static final int[] DOGTAG_HEADER = {
            0xDE, 0xAD, 0xBE, 0xEF,
            0x49, 0x4C, 0x48, 0x48,
            0x49, 0x4C, 0x48, 0x48,
            0xDE, 0xAD, 0xBE, 0xEF,
    };
    private static final long OFFSET_LAST_MODIFIED = DOGTAG_HEADER.length;
    private static final long OFFSET_TOTAL_SIZE = OFFSET_LAST_MODIFIED + 16;
    private static final long OFFSET_CURRENT_SIZE1 = OFFSET_TOTAL_SIZE + 16;
    private static final long OFFSET_CURRENT_SIZE2 = OFFSET_CURRENT_SIZE1 + 16;
    private static final long HEADER_SIZE = OFFSET_CURRENT_SIZE2 + 16;

    public static final int FLAG_PREALLOC = 1;

    /////////////////////////////////////////////////////////
    private int mFlags = 0;
    private int mTimeoutMS = 0;

    //////////////////////////////////////////////////////////
    public FetchUrl(final int flags) {
        mFlags = flags;
    }

    public interface EventReceiver {
        public void onProgress(String url, long current, long total);

        public void onError(String url, int statusCode);

        public void onCompleted(String url, RandomAccessFile output, RandomAccessFile log);

        public void onTimeout(String url);

        public void onBeginReceivingBody(String url);

        public void onReceiveBody(String url, byte[] data, int length);
    }

    //////////////////////////////////////////////////////////
    private static boolean compareInBytes(final RandomAccessFile file, final long offset, final int[] data)
    {
        if (data == null) {
            return false;
        }
        try {
            final long oldOffset = file.getFilePointer();
            file.seek(offset);
            for (int i = 0; i < data.length; ++i) {
                final int b = file.readUnsignedByte();
                if (b != data[i]) {
                    file.seek(oldOffset);
                    return false;
                }
            }
            return true;
        }
        catch (final IOException e) {
            return false;
        }
    }

    private boolean writeBytes(final RandomAccessFile file, final int[] data) {
        if (file == null || data == null) {
            return false;
        }
        try {
            for (int i = 0; i < data.length; ++i) {
                file.writeByte(data[i]);
            }
            return true;
        }
        catch (final IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    private boolean isDogTag(final RandomAccessFile file, final long offset, final int[] dogTag)
    {
        try {
            final long oldOffset = file.getFilePointer();
            file.seek(offset);
            final boolean ret = (file.readUnsignedByte() == dogTag[0]
                                 && file.readUnsignedByte() == dogTag[1]
                                 && file.readUnsignedByte() == dogTag[2]
                                 && file.readUnsignedByte() == dogTag[3]);
            file.seek(oldOffset);
            return ret;
        }
        catch (final IOException e) {
            return false;
        }
    }

    private boolean writeSizeEx(final RandomAccessFile file, final long offset, final long size, final int[] dogTag)
    {
        if (file == null || offset < 0 || dogTag == null || dogTag.length != 4) {
            return false;
        }

        try {
            final long oldOffset = file.getFilePointer();
            file.seek(offset);

            if (!writeBytes(file, dogTag)) {
                file.seek(oldOffset);
                return false;
            }

            file.writeLong(size);

            if (!writeBytes(file, dogTag)) {
                file.seek(oldOffset);
                return false;
            }

            file.seek(oldOffset);
        }
        catch (final IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    private boolean writeSize(final RandomAccessFile file, final long offset, final long size)
    {
        return writeSizeEx(file, offset, size, DOGTAG_SIZE1);
    }

    private long readSize(final RandomAccessFile file, final long offset, final long defValue)
    {
        if (file == null || offset < 0) {
            return defValue;
        }
        try {
            final long oldOffset = file.getFilePointer();
            int[] dogTag = DOGTAG_SIZE1;
            if (!isDogTag(file, offset, dogTag)) {
                dogTag = DOGTAG_SIZE2;
                if (!isDogTag(file, offset, dogTag)) {
                    file.seek(oldOffset);
                    Log.w("Invalid size dog tag.");
                    return defValue;
                }
            }
            file.seek(offset + 4);
            final long size = file.readLong();
            if (!isDogTag(file, offset, dogTag)) {
                file.seek(oldOffset);
                Log.w("Invalid size dog tag.");
                return defValue;
            }
            return size;
        }
        catch (final IOException e) {
            e.printStackTrace();
            return defValue;
        }
    }

    private boolean initLogFile(final RandomAccessFile file) {
        if (file == null) {
            return false;
        }
        try {
            final long oldOffset = file.getFilePointer();
            file.setLength(HEADER_SIZE);
            // Header dog tag
            if (!writeBytes(file, DOGTAG_HEADER)) {
                file.seek(oldOffset);
                return false;
            }
            // Last modified time
            if (!writeSize(file, OFFSET_LAST_MODIFIED, 0)) {
                file.seek(oldOffset);
                return false;
            }
            // Total size
            if (!writeSize(file, OFFSET_TOTAL_SIZE, 0)) {
                file.seek(oldOffset);
                return false;
            }
            // Current size 1
            if (!writeSize(file, OFFSET_CURRENT_SIZE1, 0)) {
                file.seek(oldOffset);
                return false;
            }
            // Current size 2
            if (!writeSize(file, OFFSET_CURRENT_SIZE2, 0)) {
                file.seek(oldOffset);
                return false;
            }
            file.seek(oldOffset);
            return true;
        }
        catch (final IOException e) {
            return false;
        }
    }

    private boolean isLogFile(final RandomAccessFile file) {
        if (file == null) {
            return false;
        }
        try {
            if (file.length() != HEADER_SIZE) {
                Log.e("Invalid log file size:" + file.length());
                return false;
            }
            if (!compareInBytes(file, 0, DOGTAG_HEADER)) {
                Log.e("Invalid header dog tag");
                return false;
            }
            return true;
        }
        catch (final IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    //////////////////////////////////////////////////////////
    private long getTotalSize(final RandomAccessFile file) {
        return readSize(file, OFFSET_TOTAL_SIZE, 0);
    }

    private boolean setTotalSize(final RandomAccessFile file, final long size) {
        return writeSize(file, OFFSET_TOTAL_SIZE, size);
    }

    private long getCurrentSize(final RandomAccessFile file) {
        final long size1 = readSize(file, OFFSET_CURRENT_SIZE1, 0);
        final long size2 = readSize(file, OFFSET_CURRENT_SIZE2, 0);
        return size1 >= size2 ? size1 : size2;
    }

    private boolean setCurrentSize(final RandomAccessFile file, final int which, final long size)
    {
        if (file == null) {
            return false;
        }
        try {
            final long oldOffset = file.getFilePointer();
            if (which % 2 == 0) {
                file.seek(OFFSET_CURRENT_SIZE1);
                int[] dogTag = DOGTAG_SIZE1;
                if (file.readUnsignedByte() == dogTag[0]) {
                    dogTag = DOGTAG_SIZE2;
                }
                file.seek(oldOffset);
                return writeSizeEx(file, OFFSET_CURRENT_SIZE1, size, dogTag);
            }
            else {
                file.seek(OFFSET_CURRENT_SIZE2);
                int[] dogTag = DOGTAG_SIZE2;
                if (file.readUnsignedByte() == dogTag[0]) {
                    dogTag = DOGTAG_SIZE2;
                }
                file.seek(oldOffset);
                return writeSizeEx(file, OFFSET_CURRENT_SIZE2, size, dogTag);
            }
        }
        catch (final IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    private long getLastModified(final RandomAccessFile file) {
        return readSize(file, OFFSET_LAST_MODIFIED, 0);
    }

    private boolean setLastModified(final RandomAccessFile file, final long size) {
        return writeSize(file, OFFSET_LAST_MODIFIED, size);
    }

    //////////////////////////////////////////////////////////
    private String formateTime(final long time) {
        final SimpleDateFormat formatter = new SimpleDateFormat("E, dd MMM yyyy HH:mm:ss", Locale.US);
        formatter.setTimeZone(TimeZone.getTimeZone("GMT"));
        return formatter.format(new Date(time)) + " GMT";
    }

    private int[] parseContentRange(final String data) {
        final Pattern p1 = Pattern.compile("^bytes (\\-?\\d+)\\-(\\-?\\d+)\\/(\\-?\\d+)");
        final Matcher m1 = p1.matcher(data);
        if (m1.find()) {
            return new int[] {
                    Integer.parseInt(m1.group(1)),
                    Integer.parseInt(m1.group(3)),
            };
        }

        final Pattern p2 = Pattern.compile("^bytes (\\-?\\d+|\\*)\\/(\\-?\\d+|\\*)");
        final Matcher m2 = p2.matcher(data);
        if (m2.find()) {
            return new int[] {
                    Integer.parseInt(m1.group(1)),
                    Integer.parseInt(m1.group(2)),
            };
        }

        return new int[] { 0, 0 };
    }

    //////////////////////////////////////////////////////////
    public void setFlags(final int flags) {
        mFlags = flags;
    }

    public int getFlags() {
        return mFlags;
    }

    public boolean fetchUrl(final String url, final String filePath) {
        return fetchUrlEx(url, filePath, null, null);
    }

    public boolean fetchUrl(final String url, final String filePath, final String logFilePath)
    {
        return fetchUrlEx(url, filePath, logFilePath, null);
    }

    public boolean fetchUrlEx(final String url, final String filePath, final String logFilePath, final EventReceiver eventReceiver)
    {
        if (url == null || url.length() == 0) {
            return false;
        }

        HttpURLConnection conn = null;
        try {
            RandomAccessFile outputFile = null;
            if (filePath != null) {
                outputFile = new RandomAccessFile(filePath, "rw");
            }
            RandomAccessFile logFile = null;
            if (logFilePath != null) {
                logFile = new RandomAccessFile(logFilePath, "rw");
            }

            conn = (HttpURLConnection)(new URL(url)).openConnection();
            HttpURLConnection.setFollowRedirects(true);
            conn.setUseCaches(false);

            String lastModified = formateTime(0);
            long currentSize = 0;
            long totalSize = 0;
            // Read log file and set HTTP request headers
            if (logFile != null) {
                Log.d("Log file size:" + logFile.length());
                if (isLogFile(logFile)) {
                    Log.d("Log file found.");

                    lastModified = formateTime(getLastModified(logFile));
                    Log.d("last modified:" + lastModified);

                    currentSize = getCurrentSize(logFile);
                    if (outputFile != null && currentSize > outputFile.length()) {
                        currentSize = 0;
                    }

                    totalSize = getTotalSize(logFile);
                    if (totalSize == 0
                        || outputFile == null
                        || totalSize > outputFile.length()
                        || currentSize > totalSize) {
                        totalSize = -1;
                        currentSize = 0;
                    }
                    Log.d("total size in log file:" + totalSize);
                }
                else {
                    Log.d("Not a valid log file.");
                    if (!initLogFile(logFile)) {
                        Log.w("Can't initialize log file.");
                    }
                }
            }

            // Set HTTP request headers
            if (currentSize > 0) {
                if (currentSize == totalSize) {
                    conn.addRequestProperty("If-Modified-Since", lastModified);
                }
                else {
                    final StringBuilder range = new StringBuilder();
                    range.append("bytes=")
                            .append(currentSize)
                            .append("-")
                            .append(totalSize);
                    conn.addRequestProperty("Range", range.toString());
                    conn.addRequestProperty("If-Range", lastModified);
                }
            }

            conn.connect();

            final int statusCode = conn.getResponseCode();
            if (statusCode == 200 || statusCode == 206) {
                // Read http response headers and write log file
                final long newLastModified = conn.getLastModified();
                if (newLastModified > 0) {
                    if (!setLastModified(logFile, newLastModified)) {
                        Log.w("Can't write last modified time to log file.");
                    }
                }

                // Read Content-Range from response headers
                totalSize = conn.getContentLength();
                final String contentRange = conn.getHeaderField("Content-Range");
                if (contentRange != null && contentRange.length() > 0) {
                    final int[] ret = parseContentRange(contentRange);
                    currentSize = ret[0];
                    if (ret[1] > 0) {
                        totalSize = ret[1];
                    }
                }

                // Write current and total size into log file.
                int which = 0;
                if (currentSize < 0) {
                    currentSize = 0;
                }
                if (!setCurrentSize(logFile, which, currentSize)) {
                    Log.w("Can't write current size into log file.");
                }
                if (outputFile != null)
                    outputFile.seek(currentSize);

                if (totalSize < 0) {
                    totalSize = 0;
                }
                if (!setTotalSize(logFile, totalSize)) {
                    Log.w("Can't write total size into log file.");
                }
                // Resize the file.
                if (outputFile != null && ((mFlags & FLAG_PREALLOC) == FLAG_PREALLOC || outputFile.length() > totalSize)) {
                    outputFile.setLength(totalSize);
                }
                if (eventReceiver != null)
                    eventReceiver.onBeginReceivingBody(url);

                final InputStream input = conn.getInputStream();
                final byte[] buffer = new byte[512];
                int bufSize = 0;
                while ((bufSize = input.read(buffer)) > 0) {
                    if (outputFile != null) {
                        outputFile.write(buffer, 0, bufSize);
                    }
                    currentSize += bufSize;
                    if (logFile != null) {
                        if (!setCurrentSize(logFile, which, currentSize)) {
                            Log.w("Can't write current size to log file.");
                        }
                    }
                    if (eventReceiver != null) {
                        // Synchronize the data to the sd card is very slow.
                        // outputFile.getFD().sync();
                        eventReceiver.onProgress(url, currentSize, totalSize);
                        eventReceiver.onReceiveBody(url, buffer, bufSize);
                    }
                    ++which;
                }

                if (eventReceiver != null) {
                    eventReceiver.onCompleted(url, outputFile, logFile);
                }
            }
            else if (statusCode == 304) {
                Log.i(url + " is not modified.");
                if (eventReceiver != null) {
                    eventReceiver.onCompleted(url, outputFile, logFile);
                }
            }
            else {
                Log.i("Status code:" + statusCode + ", can't fetch " + url);
                if (eventReceiver != null) {
                    eventReceiver.onError(url, statusCode);
                }

                if (conn != null)
                    conn.disconnect();
                return false;
            }

            if (conn != null)
                conn.disconnect();
            return true;
        }
        catch (final java.net.SocketTimeoutException e) {
            if (eventReceiver != null) {
                if (eventReceiver != null) {
                    eventReceiver.onTimeout(url);
                }
            }
        }
        catch (final Exception e) {
            e.printStackTrace();
            // Log.e(e.getMessage());
            if (eventReceiver != null) {
                eventReceiver.onError(url, 0);
            }
        }
        if (conn != null)
            conn.disconnect();
        return false;
    }

    public void setTimeout(final int timeoutMS) {
        mTimeoutMS = timeoutMS;
    }

    public int getTimeout() {
        return mTimeoutMS;
    }
}
