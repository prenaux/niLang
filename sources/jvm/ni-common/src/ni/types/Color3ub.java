package ni.types;

import ni.types.util.Hash;

/**
 * Holds a R8G8B8 Color.
 */
public class Color3ub implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Color3ub";
    }

    public static String __jniNativeType() {
        return "ni::sColor3ub";
    }

    public static String __jniNullRet() {
        return "ni::sColor3ub::Zero()";
    }

    public byte x, y, z;

    public Color3ub set(final int x, final int y, final int z) {
        this.x = (byte)(x & 0xFF);
        this.y = (byte)(y & 0xFF);
        this.z = (byte)(z & 0xFF);
        return this;
    }

    public Color3ub set(final Color3ub src) {
        x = src.x;
        y = src.y;
        z = src.z;
        return this;
    }

    @Override
    public String toString() {
        final StringBuilder sb = new StringBuilder(64);
        sb.append("Color3ub[");
        sb.append(x);
        sb.append(", ");
        sb.append(y);
        sb.append(", ");
        sb.append(z);
        sb.append(']');
        return sb.toString();
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Color3ub))
            return false;
        final Color3ub rhs = (Color3ub)o;
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y, z);
    }
}
