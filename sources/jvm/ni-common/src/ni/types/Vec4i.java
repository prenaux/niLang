package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 4-tuple vector.
 */
public class Vec4i implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Vec4i";
    }

    public static String __jniNativeType() {
        return "ni::sVec4i";
    }

    public static String __jniNullRet() {
        return "ni::sVec4i::Zero()";
    }

    public int x, y, z, w;

    public Vec4i setAt(final int i, final int v) {
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

    public int getAt(final int i) {
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

    public Vec4i set(final float x) {
        this.x = (int)x;
        return this;
    }

    public Vec4i set(final float x, final float y) {
        this.x = (int)x;
        this.y = (int)y;
        return this;
    }

    public Vec4i set(final float x, final float y, final float z) {
        this.x = (int)x;
        this.y = (int)y;
        this.z = (int)z;
        return this;
    }

    public Vec4i set(final float x, final float y, final float z, final float w) {
        this.x = (int)x;
        this.y = (int)y;
        this.z = (int)z;
        this.w = (int)w;
        return this;
    }

    public Vec4i set(final Vec2f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        return this;
    }

    public Vec4i set(final Vec3f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        this.z = (int)v.z;
        return this;
    }

    public Vec4i set(final Vec4f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        this.z = (int)v.z;
        this.w = (int)v.w;
        return this;
    }

    public Vec4i set(final int x) {
        this.x = x;
        return this;
    }

    public Vec4i set(final int x, final int y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec4i set(final int x, final int y, final int z) {
        this.x = x;
        this.y = y;
        this.z = z;
        return this;
    }

    public Vec4i set(final int x, final int y, final int z, final int w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
        return this;
    }

    public Vec4i set(final Vec2i v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    public Vec4i set(final Vec3i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        return this;
    }

    public Vec4i set(final Vec4i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
        return this;
    }

    /**
     * Set a rectangle's width
     */
    public Vec4i setWidth(final int w) {
        this.z = this.x + w;
        return this;
    }

    /**
     * Get a rectangle's width
     */
    public int getWidth() {
        return this.z - this.x;
    }

    /**
     * Set a rectangle's height
     */
    public Vec4i setHeight(final int h) {
        this.w = this.y + h;
        return this;
    }

    /**
     * Get a rectangle's height
     */
    public int getHeight() {
        return this.w - this.y;
    }

    /**
     * Set a rectangle's size
     */
    public Vec4i setSize(final int w, final int h) {
        this.z = this.x + w;
        this.w = this.y + h;
        return this;
    }

    /**
     * Set a rectangle's size
     */
    public Vec4i setSize(final Vec2i size) {
        this.z = this.x + size.x;
        this.w = this.y + size.y;
        return this;
    }

    /**
     * Get a rectangle's size
     */
    public Vec2i getSize() {
        return (new Vec2i()).set(this.z - this.x, this.w - this.y);
    }

    public int getLeft() {
        return x;
    }

    public Vec4i setLeft(final int v) {
        this.x = v;
        return this;
    }

    public int getTop() {
        return y;
    }

    public Vec4i setTop(final int v) {
        this.y = v;
        return this;
    }

    public int getRight() {
        return z;
    }

    public Vec4i setRight(final int v) {
        this.z = v;
        return this;
    }

    public int getBottom() {
        return w;
    }

    public Vec4i setBottom(final int v) {
        this.w = v;
        return this;
    }

    public int get11() {
        return x;
    }

    public Vec4i set11(final int v) {
        this.x = v;
        return this;
    }

    public int get12() {
        return y;
    }

    public Vec4i set12(final int v) {
        this.y = v;
        return this;
    }

    public int get21() {
        return z;
    }

    public Vec4i set21(final int v) {
        this.z = v;
        return this;
    }

    public int get22() {
        return w;
    }

    public Vec4i set22(final int v) {
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
        if (!(o instanceof Vec4i))
            return false;
        final Vec4i rhs = (Vec4i)o;
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y, z, w);
    }

    @Override
    public Vec4i clone() {
        return (new Vec4i()).set(this);
    }

    /**
     * Copy the object into this vector.
     * @throws ClassCastException if the object is not of a compatible type 
     */
    public Vec4i set(final Object o) throws ClassCastException {
        if (o instanceof Vec4f) {
            return this.set((Vec4f)o);
        }
        if (o instanceof Vec3f) {
            return this.set((Vec3f)o);
        }
        if (o instanceof Vec2f) {
            return this.set((Vec2f)o);
        }
        if (o instanceof Vec3i) {
            return this.set((Vec3i)o);
        }
        if (o instanceof Vec2i) {
            return this.set((Vec2i)o);
        }
        return this.set((Vec4i)o);
    }
}
