package ni.types.util;

public class Strings {
    protected Strings() {
        // Utility class.
    }

    private static final char[] digits = "0123456789ABCDEF".toCharArray();

    public final static void toHex(final StringBuffer buf, final long n, final int len) {
        for (int i = len - 1; i >= 0; i--) {
            buf.append(digits[((int)(n >> (i * 4))) & 0xF]);
        }
    }

    public final static void toHex(final char[] chars, final int charsStartPos, final long n, final int len)
    {
        int ci = charsStartPos;
        for (int i = len - 1; i >= 0; i--) {
            chars[ci++] = digits[((int)(n >> (i * 4))) & 0xF];
        }
    }

    public final static String toHexString(final long n, final int len) {
        final StringBuffer sb = new StringBuffer(32);
        toHex(sb, n, len);
        return sb.toString();
    }
}
