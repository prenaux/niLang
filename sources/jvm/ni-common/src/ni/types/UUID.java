package ni.types;

import ni.types.util.Strings;

public class UUID extends Object implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "UUID";
    }

    public static String __jniNativeType() {
        return "ni::tUUID";
    }

    public static String __jniNullRet() {
        return "ni::kuuidZero()";
    }

    public final long dataHi;
    public final long dataLo;

    /**
     * The NULL UUID
     */
    public static final UUID UUID_NULL = new UUID(0, 0);

    /**
     * Build from a hi and lo long
     */
    UUID(final long[] values) {
        this.dataHi = values[0];
        this.dataLo = values[1];
    }

    /**
     * Build from a hi and lo long
     */
    UUID(final long hi, final long lo) {
        this.dataHi = hi;
        this.dataLo = lo;
    }

    /**
     * Build from an interface definition UUID
     */
    public UUID(final int anData1, final int anData2, final int anData3,
            final int anData4, final int anData5, final int anData6, final int anData7,
            final int anData8, final int anData9, final int anData10, final int anData11) {
        dataHi = (anData1 & 0x00000000FFFFFFFFL) |
                 (((long)(anData2 & 0xFFFF)) << 32) |
                 (((long)(anData3 & 0xFFFF)) << 48);
        dataLo = (anData4) |
                 ((long)(anData5 & 0xFF) << 8) |
                 ((long)(anData6 & 0xFF) << 16) |
                 ((long)(anData7 & 0xFF) << 24) |
                 ((long)(anData8 & 0xFF) << 32) |
                 ((long)(anData9 & 0xFF) << 40) |
                 ((long)(anData10 & 0xFF) << 48) |
                 ((long)(anData11 & 0xFF) << 56);
    }

    /**
     * Parses the string representation "<tt>{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}</tt>"
     * or "<tt>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</tt>".
     * @param str the String to be parsed
     */
    public UUID(String str) {
        if (str.length() == 32 + 4) {
            str = '{' + str + '}';
        }

        if (str.length() != 32 + 6)
            throw new IllegalArgumentException("not a UUID: " + str);

        // {xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}
        //  (1)      (10) (15) (20) (25)
        dataHi = parse(str, 1, 8) | (parse(str, 10, 4) << 32) | (parse(str, 15, 4) << 48);
        dataLo = parse(str, 20, 2) |
                 (parse(str, 22, 2) << 8) |
                 (parse(str, 25, 2) << 16) |
                 (parse(str, 27, 2) << 24) |
                 (parse(str, 29, 2) << 32) |
                 (parse(str, 31, 2) << 40) |
                 (parse(str, 33, 2) << 48) |
                 (parse(str, 35, 2) << 56);
    }

    private long parse(final String s, final int idx, final int len) {
        return Long.parseLong(s.substring(idx, idx + len), 16);
    }

    /**
     * Returns true if the given object is a {@link UUID} object and has the same bit representation.
     * @param o the second object
     * @return true, if the given UUID object and this object have the same bit representation.
     */
    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof UUID))
            return false;

        final UUID guid = (UUID)o;

        if (dataHi != guid.dataHi)
            return false;
        if (dataLo != guid.dataLo)
            return false;

        return true;
    }

    @Override
    public int hashCode() {
        int result;
        result = (int)(dataHi ^ (dataHi >>> 32));
        result = 29 * result + (int)(dataLo ^ (dataLo >>> 32));
        return result;
    }

    /**
     * Returns the UUID in the "<tt>{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}</tt>" format.
     * @return the String representation of this UUID object.
     */
    @Override
    public String toString() {
        final StringBuffer buf = new StringBuffer(38);
        buf.append('{');
        Strings.toHex(buf, (dataHi & 0x00000000FFFFFFFFL), 8);
        buf.append('-');
        Strings.toHex(buf, (dataHi & 0x0000FFFF00000000L) >> 32, 4);
        buf.append('-');
        Strings.toHex(buf, (dataHi & 0xFFFF000000000000L) >> 48, 4);
        buf.append('-');
        Strings.toHex(buf, (dataLo & 0x00000000000000FFL), 2);
        Strings.toHex(buf, (dataLo & 0x000000000000FF00L) >> 8, 2);
        buf.append('-');
        Strings.toHex(buf, (dataLo & 0x0000000000FF0000L) >> 16, 2);
        Strings.toHex(buf, (dataLo & 0x00000000FF000000L) >> 24, 2);
        Strings.toHex(buf, (dataLo & 0x000000FF00000000L) >> 32, 2);
        Strings.toHex(buf, (dataLo & 0x0000FF0000000000L) >> 40, 2);
        Strings.toHex(buf, (dataLo & 0x00FF000000000000L) >> 48, 2);
        Strings.toHex(buf, (dataLo & 0xFF00000000000000L) >> 56, 2);
        buf.append('}');
        return buf.toString();
    }
};
