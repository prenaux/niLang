package ni.types;

public class TwoShort {
    private TwoShort() {
    }

    public static int build(final int a, final int b) {
        return (a & 0xFFFF) | ((b & 0xFFFF) << 16);
    }

    public static int getA(final int v) {
        return (v & 0xFFFF);
    }

    public static int getB(final int v) {
        return ((v >> 16) & 0xFFFF);
    }
};
