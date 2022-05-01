package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 3-tuple int vector.
 */
public class Vec3i implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Vec3i";
    }

    public static String __jniNativeType() {
        return "ni::sVec3i";
    }

    public static String __jniNullRet() {
        return "ni::sVec3i::Zero()";
    }

    public int x, y, z;

    public Vec3i() {
    }

    public Vec3i setAt(final int i, final int v) {
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
        default:
            throw new IndexOutOfBoundsException();
        }
    }

    public Vec3i set(final float x) {
        this.x = (int)x;
        return this;
    }

    public Vec3i set(final float x, final float y) {
        this.x = (int)x;
        this.y = (int)y;
        return this;
    }

    public Vec3i set(final float x, final float y, final float z) {
        this.x = (int)x;
        this.y = (int)y;
        this.z = (int)z;
        return this;
    }

    public Vec3i set(final Vec2f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        return this;
    }

    public Vec3i set(final Vec3f v) {
        this.x = (int)v.x;
        this.y = (int)v.y;
        this.z = (int)v.z;
        return this;
    }

    public Vec3i set(final int x) {
        this.x = x;
        return this;
    }

    public Vec3i set(final int x, final int y) {
        this.x = x;
        this.y = y;
        return this;
    }

    public Vec3i set(final int x, final int y, final int z) {
        this.x = x;
        this.y = y;
        this.z = z;
        return this;
    }

    public Vec3i set(final Vec2i v) {
        this.x = v.x;
        this.y = v.y;
        return this;
    }

    public Vec3i set(final Vec3i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        return this;
    }

    @Override
    public String toString() {
        return "Vec3(" + x + "," + y + "," + z + ")";
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Vec3i))
            return false;
        final Vec3i rhs = (Vec3i)o;
        return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
    }

    @Override
    public int hashCode() {
        return Hash.hashVec(x, y, z);
    }

    @Override
    public Vec3i clone() {
        return (new Vec3i()).set(this);
    }

    /**
     * Copy the object into this vector.
     * @throws ClassCastException if the object is not of a compatible type 
     */
    public Vec3i set(final Object o) throws ClassCastException {
        if (o instanceof Vec3f) {
            return this.set((Vec3f)o);
        }
        if (o instanceof Vec2f) {
            return this.set((Vec2f)o);
        }
        if (o instanceof Vec2i) {
            return this.set((Vec2i)o);
        }
        return this.set((Vec3i)o);
    }
}
