package ni.types;

import ni.types.util.Strings;

public class MessageID {
    private MessageID() {
    }

    public static int charToBase(final int c) {
        return (((c) == '_') ? 1 :
                (((c) >= 'a') && ((c) <= 'z')) ? (((c) - 'a') + 2) :
                        (((c) >= 'A') && ((c) <= 'Z')) ? (((c) - 'A') + 28) :
                                (((c) >= '0') && ((c) <= '9')) ? (((c) - '0') + 54) : 0);
    }

    public static int baseToChar(final int base) {
        return (((base) == 1) ? '_' :
                (((base) >= 2) && ((base) < 28)) ? (((base) - 2) + 'a') :
                        (((base) >= 28) && ((base) < 54)) ? (((base) - 28) + 'A') :
                                (((base) >= 54) && ((base) < 64))
                                        ? (((base) - 54) + '0')
                                        : '-');
    }

    public static int packBased(final int a, final int b, final int c, final int d, final int theByte)
    {
        return (theByte) | (d << 8) | (c << 14) | (b << 20) | (a << 26);
    }

    public static int build(final int a, final int b, final int c, final int d, final int theByte)
    {
        return packBased(
                charToBase(a), charToBase(b), charToBase(c), charToBase(d), theByte);
    }

    public static int buildLocal(final int theByte)
    {
        return packBased(0, 0, 0, 0, theByte);
    }

    public static int getBaseA(final int msgID) {
        return (((msgID) >> 26) & 0x3F);
    }

    public static int getBaseB(final int msgID) {
        return (((msgID) >> 20) & 0x3F);
    }

    public static int getBaseC(final int msgID) {
        return (((msgID) >> 14) & 0x3F);
    }

    public static int getBaseD(final int msgID) {
        return (((msgID) >> 8) & 0x3F);
    }

    public static int getCharA(final int msgID) {
        return baseToChar(((msgID) >> 26) & 0x3F);
    }

    public static int getCharB(final int msgID) {
        return baseToChar(((msgID) >> 20) & 0x3F);
    }

    public static int getCharC(final int msgID) {
        return baseToChar(((msgID) >> 14) & 0x3F);
    }

    public static int getCharD(final int msgID) {
        return baseToChar(((msgID) >> 8) & 0x3F);
    }

    public static int getByte(final int msgID) {
        return (((msgID)) & 0xFF);
    }

    public static int maskA(final int msgID) {
        return packBased(getBaseA(msgID), 0, 0, 0, 0);
    }

    public static int maskAB(final int msgID) {
        return packBased(getBaseA(msgID), getBaseB(msgID), 0, 0, 0);
    }

    public static int maskABC(final int msgID) {
        return packBased(getBaseA(msgID), getBaseB(msgID), getBaseC(msgID), 0, 0);
    }

    public static int maskABCD(final int msgID) {
        return packBased(
                getBaseA(msgID), getBaseB(msgID), getBaseC(msgID), getBaseD(msgID), 0);
    }

    /**
     * Create a message id from the specified char sequence and explicit byte.
     */
    public static int parse(final CharSequence cs, final int theByte) {
        if (cs == null)
            return 0;
        final int len = cs.length();
        if (len < 4)
            return 0;
        return build(cs.charAt(0), cs.charAt(1), cs.charAt(2), cs.charAt(3), theByte);
    }

    /**
     * Create a message id from the specified char sequence. 
     * Format is either ABCD.B or ABCD00.
     */
    public static int parse(final CharSequence cs) {
        if (cs == null)
            return 0;
        final int len = cs.length();
        if (len < 6)
            return 0;
        final char[] buf = { cs.charAt(4), cs.charAt(5) };
        if (buf[0] == '.')
            // letter byte...
            return build(cs.charAt(0), cs.charAt(1), cs.charAt(2), cs.charAt(3),
                    cs.charAt(5));
        else
            // hexa byte
            return build(cs.charAt(0), cs.charAt(1), cs.charAt(2), cs.charAt(3),
                    Integer.parseInt(new String(buf), 16));
    }

    /**
     * Convert a msg id to char array (should be able to hold 6 characters).
     */
    public static char[] toChars(final char[] chars, final int msgID) {
        if (chars.length < 6)
            return null;
        chars[0] = (char)getCharA(msgID);
        chars[1] = (char)getCharB(msgID);
        chars[2] = (char)getCharC(msgID);
        chars[3] = (char)getCharD(msgID);
        final int theByte = getByte(msgID);
        if ((theByte > 32) && (theByte <= 255)) {
            chars[4] = '.';
            chars[5] = (char)theByte;
        }
        else {
            Strings.toHex(chars, 4, theByte, 2);
        }
        return chars;
    }

    /**
     * Convert a message id to a new string.
     */
    public static String toString(final int msgID) {
        final char[] chars = new char[6];
        return new String(toChars(chars, msgID));
    }

    /**
     * Filter format : INCLUDE!EXCLUDE*. INCLUDE letters are matched,
     * if any of them does match returns TRUE. Then EXCLUDE letters
     * are matched, if any of them does match returns FALSE. The
     * default return value if a '!' is found is TRUE, otherwise it is
     * FALSE. This means that the string "!" will match ANY character
     * (since the default return value becomes TRUE).
     */
    public static boolean filterChar(final char d, final CharSequence cs) {
        if (cs == null)
            return false;
        final int len = cs.length();
        boolean r = false;
        int p = 0;
        // INCLUDES
        while (p < len) {
            final char c = cs.charAt(p);
            if (c == '!') {
                r = true;
                ++p; // skip the '!'
                break;
            }
            else if (c == d)
                return true;
            ++p;
        }
        // EXCLUDES
        while (p < len) {
            final char c = cs.charAt(p);
            if (c == d)
                return false;
            ++p;
        }
        return r;
    }

    /**
     * Filter the pattern using the D character of the specified msgID.
     */
    public static boolean filterD(final int msgID, final CharSequence cs) {
        return filterChar((char)getCharD(msgID), cs);
    }
};
