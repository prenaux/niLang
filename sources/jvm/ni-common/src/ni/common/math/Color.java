package ni.common.math;

import ni.types.Color3ub;
import ni.types.Color4ub;

public class Color extends Vec {
    protected Color() {
    }

    public static Color3ub Col3ub() {
        return new Color3ub();
    }

    public static Color3ub Col3ub(final Color3ub src) {
        return (new Color3ub()).set(src);
    }

    public static Color3ub Col3ub(final int x, final int y, final int z) {
        return (new Color3ub()).set(x, y, z);
    }

    public static Color4ub Col4ub() {
        return new Color4ub();
    }

    public static Color4ub Col4ub(final Color3ub src) {
        return (new Color4ub()).set(src);
    }

    public static Color4ub Col4ub(final Color4ub src) {
        return (new Color4ub()).set(src);
    }

    public static Color4ub Col4ub(final int x, final int y, final int z) {
        return (new Color4ub()).set(x, y, z);
    }

    public static Color4ub Col4ub(final int x, final int y, final int z, final int w) {
        return (new Color4ub()).set(x, y, z, w);
    }

    /**
     * Add a vector to another vector and place the result in a destination
     * vector.
     * @param dest The destination vector, or null if a new vector is to be created
     * @param left The LHS vector
     * @param right The RHS vector
     * @return the sum of left and right in dest
     */
    public static Color3ub add(final Color3ub dest, final Color3ub left, final Color3ub right)
    {
        if (dest == null)
            return Col3ub(
                    left.x + right.x, left.y + right.y, left.z + right.z);
        else {
            dest.set(
                    left.x + right.x, left.y + right.y, left.z + right.z);
            return dest;
        }
    }

    /**
     * Subtract a vector from another vector and place the result in a destination
     * vector.
     * @param dest The destination vector, or null if a new vector is to be created
     * @param left The LHS vector
     * @param right The RHS vector
     * @return left minus right in dest
     */
    public static Color3ub sub(final Color3ub dest, final Color3ub left, final Color3ub right)
    {
        if (dest == null)
            return Col3ub(
                    left.x - right.x, left.y - right.y, left.z - right.z);
        else {
            dest.set(
                    left.x - right.x, left.y - right.y, left.z - right.z);
            return dest;
        }
    }

    /**
     * Add a vector to another vector and place the result in a destination
     * vector.
     * @param dest The destination vector, or null if a new vector is to be created
     * @param left The LHS vector
     * @param right The RHS vector
     * @return the sum of left and right in dest
     */
    public static Color4ub add(final Color4ub dest, final Color4ub left, final Color4ub right)
    {
        if (dest == null)
            return Col4ub(
                    left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
        else {
            dest.set(
                    left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
            return dest;
        }
    }

    /**
     * Subtract a vector from another vector and place the result in a destination
     * vector.
     * @param dest The destination vector, or null if a new vector is to be created
     * @param left The LHS vector
     * @param right The RHS vector     * 
     * @return left minus right in dest
     */
    public static Color4ub sub(final Color4ub dest, final Color4ub left, final Color4ub right)
    {
        if (dest == null)
            return Col4ub(
                    left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
        else {
            dest.set(
                    left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
            return dest;
        }
    }
}
