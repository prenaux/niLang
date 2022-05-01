package ni.types;

public class ULColor {
    private ULColor() {
    }

    public static int build(final byte r, final byte g, final byte b, final byte a) {
        return (((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)));
    }

    public static int build(final byte r, final byte g, final byte b) {
        return (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)));
    }

    public static int build(final int r, final int g, final int b, final int a) {
        return (((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)));
    }

    public static int build(final int r, final int g, final int b) {
        return (((r & 0xFF) << 16) | ((g & 0xFF) << 8) | ((b & 0xFF)));
    }

    public static int build(final Vec3i v) {
        return build(v.x, v.y, v.z);
    }

    public static int build(final Vec4i v) {
        return build(v.x, v.y, v.z, v.w);
    }

    public static int build(final Color3ub v) {
        return build(v.x, v.y, v.z);
    }

    public static int build(final Color4ub v) {
        return build(v.x, v.y, v.z, v.w);
    }

    public static int buildf(final float r, final float g, final float b, final float a) {
        return ((((int)(a * 255.0f) & 0xFF) << 24) |
                (((int)(r * 255.0f) & 0xFF) << 16) |
                (((int)(g * 255.0f) & 0xFF) << 8) | (((int)(b * 255.0f) & 0xFF)));
    }

    public static int buildf(final float r, final float g, final float b) {
        return ((((int)(r * 255.0f) & 0xFF) << 16) |
                (((int)(g * 255.0f) & 0xFF) << 8) | (((int)(b * 255.0f) & 0xFF)));
    }

    public static int buildf(final Vec3f v) {
        return buildf(v.x, v.y, v.z);
    }

    public static int buildf(final Vec4f v) {
        return buildf(v.x, v.y, v.z, v.w);
    }

    public static int getA(final int color) {
        return (color >> 24) & 0xFF;
    }

    public static int getR(final int color) {
        return (color >> 16) & 0xFF;
    }

    public static int getG(final int color) {
        return (color >> 8) & 0xFF;
    }

    public static int getB(final int color) {
        return (color) & 0xFF;
    }

    public static float getAf(final int color) {
        return (((color >> 24) & 0xFF)) / 255.0f;
    }

    public static float getRf(final int color) {
        return (((color >> 16) & 0xFF)) / 255.0f;
    }

    public static float getGf(final int color) {
        return (((color >> 8) & 0xFF)) / 255.0f;
    }

    public static float getBf(final int color) {
        return (((color) & 0xFF)) / 255.0f;
    }
};
