package ni.types.util;

public class Hash {
    /**
     * A simple hash function for one dimensional integer mathematical vector.
     */
    public final static int hashVec(final int x) {
        final int f = (x * 1) & 0x7fffffff; // avoid problems with +-0
        return (f >> 22) ^ (f >> 12) ^ (f);
    }

    /**
     * A simple hash function for one dimensional float mathematical vector.
     */
    public final static int hashVec(final float x) {
        final int iX = Float.floatToIntBits(x);
        return hashVec(iX);
    }

    /**
     * A simple hash function for two dimensional integer mathematical vector.
     */
    public final static int hashVec(final int x, final int y) {
        final int f = (x * 1 + y * 11) & 0x7fffffff; // avoid problems with +-0
        return (f >> 22) ^ (f >> 12) ^ (f);
    }

    /**
     * A simple hash function for two dimensional float mathematical vector.
     */
    public final static int hashVec(final float x, final float y) {
        final int iX = Float.floatToIntBits(x);
        final int iY = Float.floatToIntBits(y);
        return hashVec(iX, iY);
    }

    /**
     * A simple hash function for three dimensional integer mathematical vector.
     */
    public final static int hashVec(final int x, final int y, final int z) {
        final int f = (x * 1 + y * 11 + z * 17) & 0x7fffffff; // avoid problems with +-0
        return (f >> 22) ^ (f >> 12) ^ (f);
    }

    /**
     * A simple hash function for three dimensional float mathematical vector.
     */
    public final static int hashVec(final float x, final float y, final float z) {
        final int iX = Float.floatToIntBits(x);
        final int iY = Float.floatToIntBits(y);
        final int iZ = Float.floatToIntBits(z);
        return hashVec(iX, iY, iZ);
    }

    /**
     * A simple hash function for four dimensional integer mathematical vector.
     */
    public final static int hashVec(final int x, final int y, final int z, final int w) {
        final int f = (x * 1 + y * 11 + z * 17 + w * 19) & 0x7fffffff; // avoid problems with +-0
        return (f >> 22) ^ (f >> 12) ^ (f);
    }

    /**
     * A simple hash function for four dimensional float mathematical vector.
     */
    public final static int hashVec(final float x, final float y, final float z, final float w)
    {
        final int iX = Float.floatToIntBits(x);
        final int iY = Float.floatToIntBits(y);
        final int iZ = Float.floatToIntBits(z);
        final int iW = Float.floatToIntBits(w);
        return hashVec(iX, iY, iZ, iW);
    }

    /**
     * A simple hash function for a 2x2 float mathematical matrix.
     * <p>
     * Equivalent to four dimensional float vector hash.
     */
    public final static int hashMatrix(
            final float _11, final float _12,
            final float _21, final float _22)
    {
        return hashVec(_11, _12, _21, _22);
    }

    /**
     * A simple hash function for a 3x3 float mathematical matrix
     */
    public final static int hashMatrix(
            final float _11, final float _12, final float _13,
            final float _21, final float _22, final float _23,
            final float _31, final float _32, final float _33)
    {
        final int r_1 = hashVec(_11, _12, _13);
        final int r_2 = hashVec(_21, _22, _23);
        final int r_3 = hashVec(_31, _32, _33);
        return hashVec(r_1, r_2, r_3);
    }

    /**
     * A simple hash function for a 4x4 float mathematical matrix
     */
    public final static int hashMatrix(
            final float _11, final float _12, final float _13, final float _14,
            final float _21, final float _22, final float _23, final float _24,
            final float _31, final float _32, final float _33, final float _34,
            final float _41, final float _42, final float _43, final float _44)
    {
        final int r_1 = hashVec(_11, _12, _13, _14);
        final int r_2 = hashVec(_21, _22, _23, _24);
        final int r_3 = hashVec(_31, _32, _33, _34);
        final int r_4 = hashVec(_41, _42, _43, _44);
        return hashVec(r_1, r_2, r_3, r_4);
    }
}
