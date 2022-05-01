package ni.types;

public class FourCC {
    private FourCC() {
    }

    public static int build(final byte a, final byte b, final byte c, final byte d) {
        return (((d & 0xFF) << 24) | ((c & 0xFF) << 16) | ((b & 0xFF) << 8) | ((a & 0xFF)));
    }

    public static int build(final int a, final int b, final int c, final int d) {
        return (((d & 0xFF) << 24) | ((c & 0xFF) << 16) | ((b & 0xFF) << 8) | ((a & 0xFF)));
    }

    public static int getD(final int color) {
        return (color >> 24) & 0xFF;
    }

    public static int getC(final int color) {
        return (color >> 16) & 0xFF;
    }

    public static int getB(final int color) {
        return (color >> 8) & 0xFF;
    }

    public static int getA(final int color) {
        return (color) & 0xFF;
    }
};
