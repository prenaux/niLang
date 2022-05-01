package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 2-tuple int vector.
 */
public class Vec2i implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Vec2i";
    }

    public static String __jniNativeType() {
        return "ni::sVec2i";
    }

    public static String __jniNullRet() {
        return "ni::sVec2i::Zero()";
    }

    public int x, y;

    public Vec2i() {
    }

    public Vec2i setAt(final int i, final int v) {
        switch (i) {
        case 0:
            this.x = v;
            break;
        case 1:
            this.y = v;
            break;
        default:
            throw new IndexOutOfBoundsException();
        }
        return this;
    }

    public int getAt(final int i) {
        switch (i) {
        case 0:
            return this.x;
        case 1:
            return this.y;
        default:
            throw new IndexOutOfBoundsException();
        }
    }

    public Vec2i set(final float x) {
        this.x = (int)x;
        return this;
    }

    public Vec2i set(final float x, final float y) {
        this.x = (int)x;
        this.y = (int)y;
        return this;
    }

    public Vec2i set(final Vec2f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        return this;
    }

    public Vec2i set(final int x) {
        this.x = x;
        return this;
    }

    public Vec2i set(final int x, final int y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec2i set(final Vec2i v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    @Override
    public String toString() {
        return "Vec2(" + x + "," + y + ")";
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Vec2i))
            return false;
        final Vec2i rhs = (Vec2i)o;
        return (x == rhs.x) && (y == rhs.y);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y);
    }

    @Override
    public Vec2f clone() {
        return (new Vec2f()).set(this);
    }

    /**
     * Copy the object into this vector.
     * @throws ClassCastException if the object is not of a compatible type 
     */
    public Vec2i set(final Object o) throws ClassCastException {
        if (o instanceof Vec2f) {
            return this.set((Vec2f)o);
        }
        return this.set((Vec2i)o);
    }
}
