package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 2-tuple float vector.
 */
public class Vec2f implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Vec2f";
    }

    public static String __jniNativeType() {
        return "ni::sVec2f";
    }

    public static String __jniNullRet() {
        return "ni::sVec2f::Zero()";
    }

    public float x, y;

    public Vec2f() {
    }

    public Vec2f setAt(final int i, final float v) {
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

    public float getAt(final int i) {
        switch (i) {
        case 0:
            return this.x;
        case 1:
            return this.y;
        default:
            throw new IndexOutOfBoundsException();
        }
    }

    public Vec2f set(final float x) {
        this.x = x;
        return this;
    }

    public Vec2f set(final float x, final float y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec2f set(final Vec2f v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    public Vec2f set(final int x) {
        this.x = x;
        return this;
    }

    public Vec2f set(final int x, final int y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec2f set(final Vec2i v) {
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
        if (!(o instanceof Vec2f))
            return false;
        final Vec2f rhs = (Vec2f)o;
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
    public Vec2f set(final Object o) throws ClassCastException {
        if (o instanceof Vec2i) {
            return this.set((Vec2i)o);
        }
        return this.set((Vec2f)o);
    }
}
