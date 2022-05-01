package ni.types;

import ni.types.util.Hash;

/**
 * Holds a R8G8B8 Color.
 */
public class Color4ub implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Color4ub";
    }

    public static String __jniNativeType() {
        return "ni::sColor4ub";
    }

    public static String __jniNullRet() {
        return "ni::sColor4ub::Zero()";
    }

    public byte x, y, z, w;

    public Color4ub set(final int x, final int y, final int z) {
        this.x = (byte)(x & 0xFF);
        this.y = (byte)(y & 0xFF);
        this.z = (byte)(z & 0xFF);
        return this;
    }

    public Color4ub set(final int x, final int y, final int z, final int w) {
        this.x = (byte)(x & 0xFF);
        this.y = (byte)(y & 0xFF);
        this.z = (byte)(z & 0xFF);
        this.w = (byte)(w & 0xFF);
        return this;
    }

    /**
     * Load from another Color3ub
     * 
     * @param src The source vector
     * @return this
     */
    public Color4ub set(final Color3ub src) {
        x = src.x;
        y = src.y;
        z = src.z;
        return this;
    }

    /**
     * Load from another Color4ub
     * 
     * @param src The source vector
     * @return this
     */
    public Color4ub set(final Color4ub src) {
        x = src.x;
        y = src.y;
        z = src.z;
        w = src.w;
        return this;
    }

    @Override
    public String toString() {
        final StringBuilder sb = new StringBuilder(64);
        sb.append("Color4ub[");
        sb.append(x);
        sb.append(", ");
        sb.append(y);
        sb.append(", ");
        sb.append(z);
        sb.append(", ");
        sb.append(w);
        sb.append(']');
        return sb.toString();
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Color4ub))
            return false;
        final Color4ub rhs = (Color4ub)o;
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y, z, w);
    }
}
