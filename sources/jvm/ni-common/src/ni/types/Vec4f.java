package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 4-tuple vector.
 */
public class Vec4f implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Vec4f";
    }

    public static String __jniNativeType() {
        return "ni::sVec4f";
    }

    public static String __jniNullRet() {
        return "ni::sVec4f::Zero()";
    }

    public float x, y, z, w;

    public Vec4f() {
    }

    public Vec4f setAt(final int i, final float v) {
        switch (i) {
        case 0:
            this.x = v;
            break;
        case 1:
            this.y = v;
            break;
        case 2:
            this.z = v;
            break;
        case 3:
            this.w = v;
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
        case 2:
            return this.z;
        case 3:
            return this.w;
        default:
            throw new IndexOutOfBoundsException();
        }
    }

    public Vec4f set(final float x) {
        this.x = x;
        return this;
    }

    public Vec4f set(final float x, final float y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec4f set(final float x, final float y, final float z) {
        this.x = x;
        this.y = y;
        this.z = z;
        return this;
    }

    public Vec4f set(final float x, final float y, final float z, final float w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
        return this;
    }

    public Vec4f set(final Vec2f v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    public Vec4f set(final Vec3f v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        return this;
    }

    public Vec4f set(final Vec4f v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
        return this;
    }

    public Vec4f set(final int x) {
        this.x = x;
        return this;
    }

    public Vec4f set(final int x, final int y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec4f set(final int x, final int y, final int z) {
        this.x = x;
        this.y = y;
        this.z = z;
        return this;
    }

    public Vec4f set(final int x, final int y, final int z, final int w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
        return this;
    }

    public Vec4f set(final Vec2i v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    public Vec4f set(final Vec3i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        return this;
    }

    public Vec4f set(final Vec4i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
        return this;
    }

    /**
     * Set a rectangle's width
     */
    public Vec4f setWidth(final float w) {
        this.z = this.x + w;
        return this;
    }

    /**
     * Get a rectangle's width
     */
    public float getWidth() {
        return this.z - this.x;
    }

    /**
     * Set a rectangle's height
     */
    public Vec4f setHeight(final float h) {
        this.w = this.y + h;
        return this;
    }

    /**
     * Get a rectangle's height
     */
    public float getHeight() {
        return this.w - this.y;
    }

    /**
     * Set a rectangle's size
     */
    public Vec4f setSize(final float w, final float h) {
        this.z = this.x + w;
        this.w = this.y + h;
        return this;
    }

    /**
     * Set a rectangle's size
     */
    public Vec4f setSize(final Vec2f size) {
        this.z = this.x + size.x;
        this.w = this.y + size.y;
        return this;
    }

    /**
     * Get a rectangle's size
     */
    public Vec2f getSize() {
        return (new Vec2f()).set(this.z - this.x, this.w - this.y);
    }

    public float getLeft() {
        return x;
    }

    public Vec4f setLeft(final float v) {
        this.x = v;
        return this;
    }

    public float getTop() {
        return y;
    }

    public Vec4f setTop(final float v) {
        this.y = v;
        return this;
    }

    public float getRight() {
        return z;
    }

    public Vec4f setRight(final float v) {
        this.z = v;
        return this;
    }

    public float getBottom() {
        return w;
    }

    public Vec4f setBottom(final float v) {
        this.w = v;
        return this;
    }

    public float get11() {
        return x;
    }

    public Vec4f set11(final float v) {
        this.x = v;
        return this;
    }

    public float get12() {
        return y;
    }

    public Vec4f set12(final float v) {
        this.y = v;
        return this;
    }

    public float get21() {
        return z;
    }

    public Vec4f set21(final float v) {
        this.z = v;
        return this;
    }

    public float get22() {
        return w;
    }

    public Vec4f set22(final float v) {
        this.w = v;
        return this;
    }

    @Override
    public String toString() {
        return "Vec4(" + x + "," + y + "," + z + "," + w + ")";
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Vec4f))
            return false;
        final Vec4f rhs = (Vec4f)o;
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y, z, w);
    }

    @Override
    public Vec4f clone() {
        return (new Vec4f()).set(this);
    }

    /**
     * Copy the object into this vector.
     * @throws ClassCastException if the object is not of a compatible type
     */
    public Vec4f set(final Object o) throws ClassCastException {
        if (o instanceof Vec4i) {
            return this.set((Vec4i)o);
        }
        if (o instanceof Vec3i) {
            return this.set((Vec3i)o);
        }
        if (o instanceof Vec2i) {
            return this.set((Vec2i)o);
        }
        if (o instanceof Vec3f) {
            return this.set((Vec3f)o);
        }
        if (o instanceof Vec2f) {
            return this.set((Vec2f)o);
        }
        return this.set((Vec4f)o);
    }
}
