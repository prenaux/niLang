package ni.common.util;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Collection;
import java.util.Formatter;
import java.util.Iterator;
import java.util.Locale;

import ni.types.MessageID;

import ni.common.lang3.StringUtils;

public class Strings extends StringUtils {
    protected Strings() {
    }

    public final static String join(final Collection<String> collection, final String delimiter)
    {
        final StringBuffer buffer = new StringBuffer();
        final Iterator<String> iter = collection.iterator();
        while (iter.hasNext()) {
            buffer.append(iter.next());
            if (iter.hasNext()) {
                buffer.append(delimiter);
            }
        }
        return buffer.toString();
    }

    public final static boolean isTrue(final String val) {
        final String v = val.toLowerCase();
        return "1".equals(v) || "on".equals(v) || "yes".equals(v) || "true".equals(v);
    }

    public final static String getFilename(final String val) {
        return val.substring(val.lastIndexOf("/") + 1, val.length());
    }

    public final static String getDirectory(final String val) {
        return val.substring(0, val.lastIndexOf("/"));
    }

    static final public String getStackTrace(final Throwable t) {
        final StringWriter sw = new StringWriter();
        final PrintWriter pw = new PrintWriter(sw);
        t.printStackTrace(pw);
        return sw.toString(); // stack trace as a string
    }

    static final public String fromMessage(final Object receiver, final int msg, final Object a, final Object b)
    {
        return "[-> " + ((receiver == null) ? "null" : receiver) + "]{" +
               MessageID.toString(msg) + "," + a + "," + b + "}";
    }

    static final public String fromMessage(final int msg, final Object a, final Object b)
    {
        return "{" + MessageID.toString(msg) + "," + a + "," + b + "}";
    }

    final public static String bytesToString(final byte[] b) {
        final StringBuilder sb = new StringBuilder();
        final Formatter fmt = new Formatter(sb, Locale.US);
        for (int i = 0; i < b.length; ++i) {
            fmt.format("%02X", b[i]);
        }
        return fmt.toString();
    }

    final public static String intToHex(final int value) {
        final StringBuilder sb = new StringBuilder();
        final Formatter fmt = new Formatter(sb, Locale.US);
        fmt.format("%02X", value);
        return fmt.toString();
    }

    final public static String encodeURI(final String scheme, final String host, final String path)
    {
        try {
            final URI uri = new URI(scheme, host, path, null);
            return uri.toASCIIString();
        }
        catch (final URISyntaxException e) {
            return "";
        }
    }

    final public static String getURIHost(final String uri) {
        try {
            final URI o = new URI(uri);
            return o.getHost();
        }
        catch (final URISyntaxException e) {
            return "";
        }
    }

    final public static String getURIScheme(final String uri) {
        try {
            final URI o = new URI(uri);
            return o.getScheme();
        }
        catch (final URISyntaxException e) {
            return "";
        }
    }

    final public static String getURIPath(final String uri) {
        try {
            final URI o = new URI(uri);
            return o.getPath();
        }
        catch (final URISyntaxException e) {
            return "";
        }
    }
    
    final public static String after(final String str, final String toFind) {
    	return substringAfter(str, toFind);
    }
    
    final public static String afterLast(final String str, final String toFind) {
    	return substringAfterLast(str, toFind);
    }
    
    final public static String before(final String str, final String toFind) {
    	return substringBefore(str, toFind);
    }
    
    final public static String beforeLast(final String str, final String toFind) {
    	return substringBeforeLast(str, toFind);
    }
}
