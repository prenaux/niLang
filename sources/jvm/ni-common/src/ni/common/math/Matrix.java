package ni.common.math;

import ni.types.Matrixf;
import ni.types.Vec3f;
import ni.types.Vec4f;

public class Matrix extends Vec {
    protected Matrix() {
    }

    public static final Matrixf MATRIXF_ZERO = Matrixf().setZero();
    public static final Matrixf MATRIXF_IDENTITY = Matrixf().setIdentity();

    public static Matrixf Matrixf() {
        return new Matrixf();
    }

    public static Matrixf Matrixf(final float e11, final float e12, final float e13, final float e14,
            final float e21, final float e22, final float e23, final float e24,
            final float e31, final float e32, final float e33, final float e34,
            final float e41, final float e42, final float e43, final float e44)
    {
        return (new Matrixf()).set(e11, e12, e13, e14, e21, e22, e23, e24, e31, e32, e33, e34, e41, e42, e43, e44);
    }

    /**
     * Add two matrices together and place the result in a third matrix.
     * 
     * @param Out The destination matrix, or null if a new one is to be created
     * @param left The left source matrix
     * @param right The right source matrix
     * @return the destination matrix
     */
    public static Matrixf add(Matrixf Out, final Matrixf left, final Matrixf right) {
        if (Out == null) {
            Out = new Matrixf();
        }

        Out._11 = left._11 + right._11;
        Out._12 = left._12 + right._12;
        Out._13 = left._13 + right._13;
        Out._14 = left._14 + right._14;
        Out._21 = left._21 + right._21;
        Out._22 = left._22 + right._22;
        Out._23 = left._23 + right._23;
        Out._24 = left._24 + right._24;
        Out._31 = left._31 + right._31;
        Out._32 = left._32 + right._32;
        Out._33 = left._33 + right._33;
        Out._34 = left._34 + right._34;
        Out._41 = left._41 + right._41;
        Out._42 = left._42 + right._42;
        Out._43 = left._43 + right._43;
        Out._44 = left._44 + right._44;
        return Out;
    }

    /**
     * Subtract the right matrix from the left and place the result in a third
     * matrix.
     * 
     * @param Out The destination matrix, or null if a new one is to be created
     * @param left The left source matrix
     * @param right The right source matrix
     * @return the destination matrix
     */
    public static Matrixf sub(Matrixf Out, final Matrixf left, final Matrixf right) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._11 = left._11 - right._11;
        Out._12 = left._12 - right._12;
        Out._13 = left._13 - right._13;
        Out._14 = left._14 - right._14;
        Out._21 = left._21 - right._21;
        Out._22 = left._22 - right._22;
        Out._23 = left._23 - right._23;
        Out._24 = left._24 - right._24;
        Out._31 = left._31 - right._31;
        Out._32 = left._32 - right._32;
        Out._33 = left._33 - right._33;
        Out._34 = left._34 - right._34;
        Out._41 = left._41 - right._41;
        Out._42 = left._42 - right._42;
        Out._43 = left._43 - right._43;
        Out._44 = left._44 - right._44;
        return Out;
    }

    /**
     * Performs a lerp between two matrices member by member.
     * 
     * @param Out The destination matrix, or null if a new one is to be created
     * @param left The left source matrix
     * @param right The right source matrix
     * @return the destination matrix
     */
    public static Matrixf lerp(Matrixf Out, final Matrixf left, final Matrixf right, final float t)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._11 = lerp(left._11, right._11, t);
        Out._12 = lerp(left._12, right._12, t);
        Out._13 = lerp(left._13, right._13, t);
        Out._14 = lerp(left._14, right._14, t);
        Out._21 = lerp(left._21, right._21, t);
        Out._22 = lerp(left._22, right._22, t);
        Out._23 = lerp(left._23, right._23, t);
        Out._24 = lerp(left._24, right._24, t);
        Out._31 = lerp(left._31, right._31, t);
        Out._32 = lerp(left._32, right._32, t);
        Out._33 = lerp(left._33, right._33, t);
        Out._34 = lerp(left._34, right._34, t);
        Out._41 = lerp(left._41, right._41, t);
        Out._42 = lerp(left._42, right._42, t);
        Out._43 = lerp(left._43, right._43, t);
        Out._44 = lerp(left._44, right._44, t);
        return Out;
    }

    /**
     * Multiply the right matrix by the left and place the result in a third
     * matrix.
     * 
     * @param Out The destination matrix, or null if a new one is to be created
     * @param M1 The left source matrix
     * @param M2 The right source matrix
     * @return the destination matrix
     */
    public static Matrixf mul(Matrixf Out, final Matrixf M1, final Matrixf M2) {
        if (Out == null) {
            Out = new Matrixf();
        }

        final float _11 =
                M1._11 * M2._11 + M1._21 * M2._12 + M1._31 * M2._13 + M1._41 * M2._14;
        final float _12 =
                M1._12 * M2._11 + M1._22 * M2._12 + M1._32 * M2._13 + M1._42 * M2._14;
        final float _13 =
                M1._13 * M2._11 + M1._23 * M2._12 + M1._33 * M2._13 + M1._43 * M2._14;
        final float _14 =
                M1._14 * M2._11 + M1._24 * M2._12 + M1._34 * M2._13 + M1._44 * M2._14;
        final float _21 =
                M1._11 * M2._21 + M1._21 * M2._22 + M1._31 * M2._23 + M1._41 * M2._24;
        final float _22 =
                M1._12 * M2._21 + M1._22 * M2._22 + M1._32 * M2._23 + M1._42 * M2._24;
        final float _23 =
                M1._13 * M2._21 + M1._23 * M2._22 + M1._33 * M2._23 + M1._43 * M2._24;
        final float _24 =
                M1._14 * M2._21 + M1._24 * M2._22 + M1._34 * M2._23 + M1._44 * M2._24;
        final float _31 =
                M1._11 * M2._31 + M1._21 * M2._32 + M1._31 * M2._33 + M1._41 * M2._34;
        final float _32 =
                M1._12 * M2._31 + M1._22 * M2._32 + M1._32 * M2._33 + M1._42 * M2._34;
        final float _33 =
                M1._13 * M2._31 + M1._23 * M2._32 + M1._33 * M2._33 + M1._43 * M2._34;
        final float _34 =
                M1._14 * M2._31 + M1._24 * M2._32 + M1._34 * M2._33 + M1._44 * M2._34;
        final float _41 =
                M1._11 * M2._41 + M1._21 * M2._42 + M1._31 * M2._43 + M1._41 * M2._44;
        final float _42 =
                M1._12 * M2._41 + M1._22 * M2._42 + M1._32 * M2._43 + M1._42 * M2._44;
        final float _43 =
                M1._13 * M2._41 + M1._23 * M2._42 + M1._33 * M2._43 + M1._43 * M2._44;
        final float _44 =
                M1._14 * M2._41 + M1._24 * M2._42 + M1._34 * M2._43 + M1._44 * M2._44;

        Out._11 = _11;
        Out._12 = _12;
        Out._13 = _13;
        Out._14 = _14;
        Out._21 = _21;
        Out._22 = _22;
        Out._23 = _23;
        Out._24 = _24;
        Out._31 = _31;
        Out._32 = _32;
        Out._33 = _33;
        Out._34 = _34;
        Out._41 = _41;
        Out._42 = _42;
        Out._43 = _43;
        Out._44 = _44;

        return Out;
    }

    /**
     * Transpose the source matrix and place the result in the destination
     * matrix
     * 
     * @param Out The destination matrix or null if a new matrix is to be
     *            created
     * @param src The source matrix
     * @return the transposed matrix
     */
    public static Matrixf transpose(Matrixf Out, final Matrixf src) {
        if (Out == null) {
            Out = new Matrixf();
        }
        final float _11 = src._11;
        final float _12 = src._21;
        final float _13 = src._31;
        final float _14 = src._41;
        final float _21 = src._12;
        final float _22 = src._22;
        final float _23 = src._32;
        final float _24 = src._42;
        final float _31 = src._13;
        final float _32 = src._23;
        final float _33 = src._33;
        final float _34 = src._43;
        final float _41 = src._14;
        final float _42 = src._24;
        final float _43 = src._34;
        final float _44 = src._44;
        Out._11 = _11;
        Out._12 = _12;
        Out._13 = _13;
        Out._14 = _14;
        Out._21 = _21;
        Out._22 = _22;
        Out._23 = _23;
        Out._24 = _24;
        Out._31 = _31;
        Out._32 = _32;
        Out._33 = _33;
        Out._34 = _34;
        Out._41 = _41;
        Out._42 = _42;
        Out._43 = _43;
        Out._44 = _44;
        return Out;
    }

    /**
     * @return the determinant of the matrix
     */
    public static float det(final Matrixf M) {
        float f =
                M._11
                        * ((M._22 * M._33 * M._44 + M._23 * M._34 * M._42 + M._24 * M._32 * M._43)
                           - M._24 * M._33 * M._42
                           - M._22 * M._34 * M._43
                           - M._23 * M._32 * M._44);
        f -= M._12
             * ((M._21 * M._33 * M._44 + M._23 * M._34 * M._41 + M._24 * M._31 * M._43)
                - M._24 * M._33 * M._41
                - M._21 * M._34 * M._43
                - M._23 * M._31 * M._44);
        f += M._13
             * ((M._21 * M._32 * M._44 + M._22 * M._34 * M._41 + M._24 * M._31 * M._42)
                - M._24 * M._32 * M._41
                - M._21 * M._34 * M._42
                - M._22 * M._31 * M._44);
        f -= M._14
             * ((M._21 * M._32 * M._43 + M._22 * M._33 * M._41 + M._23 * M._31 * M._42)
                - M._23 * M._32 * M._41
                - M._21 * M._33 * M._42
                - M._22 * M._31 * M._43);
        return f;
    }

    /**
     * Invert the source matrix and put the result in the destination
     * 
     * @param Out The destination matrix, or null if a new matrix is to be
     *            created
     * @param src The source matrix
     * @return The inverted matrix if successful, null otherwise
     */
    public static Matrixf invert(Matrixf Out, final Matrixf src) {
        final float determinant = det(src);
        if (determinant != 0) {
            if (Out == null) {
                Out = new Matrixf();
            }
            final float determinant_inv = 1f / determinant;

            // first row
            final float t00 = det3x3(
                    src._22, src._23, src._24, src._32, src._33, src._34, src._42,
                    src._43, src._44);
            final float t01 = -det3x3(
                    src._21, src._23, src._24, src._31, src._33, src._34, src._41,
                    src._43, src._44);
            final float t02 = det3x3(
                    src._21, src._22, src._24, src._31, src._32, src._34, src._41,
                    src._42, src._44);
            final float t03 = -det3x3(
                    src._21, src._22, src._23, src._31, src._32, src._33, src._41,
                    src._42, src._43);
            // second row
            final float t10 = -det3x3(
                    src._12, src._13, src._14, src._32, src._33, src._34, src._42,
                    src._43, src._44);
            final float t11 = det3x3(
                    src._11, src._13, src._14, src._31, src._33, src._34, src._41,
                    src._43, src._44);
            final float t12 = -det3x3(
                    src._11, src._12, src._14, src._31, src._32, src._34, src._41,
                    src._42, src._44);
            final float t13 = det3x3(
                    src._11, src._12, src._13, src._31, src._32, src._33, src._41,
                    src._42, src._43);
            // third row
            final float t20 = det3x3(
                    src._12, src._13, src._14, src._22, src._23, src._24, src._42,
                    src._43, src._44);
            final float t21 = -det3x3(
                    src._11, src._13, src._14, src._21, src._23, src._24, src._41,
                    src._43, src._44);
            final float t22 = det3x3(
                    src._11, src._12, src._14, src._21, src._22, src._24, src._41,
                    src._42, src._44);
            final float t23 = -det3x3(
                    src._11, src._12, src._13, src._21, src._22, src._23, src._41,
                    src._42, src._43);
            // fourth row
            final float t30 = -det3x3(
                    src._12, src._13, src._14, src._22, src._23, src._24, src._32,
                    src._33, src._34);
            final float t31 = det3x3(
                    src._11, src._13, src._14, src._21, src._23, src._24, src._31,
                    src._33, src._34);
            final float t32 = -det3x3(
                    src._11, src._12, src._14, src._21, src._22, src._24, src._31,
                    src._32, src._34);
            final float t33 = det3x3(
                    src._11, src._12, src._13, src._21, src._22, src._23, src._31,
                    src._32, src._33);

            // transpose and divide by the determinant
            Out._11 = t00 * determinant_inv;
            Out._22 = t11 * determinant_inv;
            Out._33 = t22 * determinant_inv;
            Out._44 = t33 * determinant_inv;
            Out._12 = t10 * determinant_inv;
            Out._21 = t01 * determinant_inv;
            Out._31 = t02 * determinant_inv;
            Out._13 = t20 * determinant_inv;
            Out._23 = t21 * determinant_inv;
            Out._32 = t12 * determinant_inv;
            Out._14 = t30 * determinant_inv;
            Out._41 = t03 * determinant_inv;
            Out._24 = t31 * determinant_inv;
            Out._42 = t13 * determinant_inv;
            Out._43 = t23 * determinant_inv;
            Out._34 = t32 * determinant_inv;
            return Out;
        }
        else {
            return null;
        }
    }

    /**
     * Invert the specified matrix in place.
     * 
     * @return The inverted matrix if successful, null otherwise
     */
    public static Matrixf invert(final Matrixf src) {
        return invert(src, src);
    }

    /**
     * Negate this matrix and place the result in a destination matrix.
     * 
     * @param Out The destination matrix, or null if a new matrix is to be
     *            created
     * @param src The source matrix
     * @return The negated matrix
     */
    public static Matrixf negate(Matrixf Out, final Matrixf src) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._11 = -src._11;
        Out._12 = -src._12;
        Out._13 = -src._13;
        Out._14 = -src._14;
        Out._21 = -src._21;
        Out._22 = -src._22;
        Out._23 = -src._23;
        Out._24 = -src._24;
        Out._31 = -src._31;
        Out._32 = -src._32;
        Out._33 = -src._33;
        Out._34 = -src._34;
        Out._41 = -src._41;
        Out._42 = -src._42;
        Out._43 = -src._43;
        Out._44 = -src._44;
        return Out;
    }

    public static Matrixf lookAtRH(Matrixf Out, final Vec3f Eye, final Vec3f At, final Vec3f Up)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        final Vec3f nAxis = Vec.normalize(Vec.negate(Vec.sub(null, At, Eye)));
        final Vec3f vAxis = Vec.sub(null, Up, nAxis);
        Vec.mul(vAxis, vAxis, Vec.dot(Up, nAxis));

        if (Vec.lengthSq(vAxis) < EPSILON5) {
            vAxis.x = -nAxis.y * nAxis.x;
            vAxis.y = 1.0f - nAxis.y * nAxis.y;
            vAxis.z = -nAxis.y * nAxis.z;
            if (Vec.lengthSq(vAxis) < EPSILON5) {
                vAxis.x = -nAxis.z * nAxis.x;
                vAxis.y = -nAxis.z * nAxis.y;
                vAxis.z = 1.0f - nAxis.z * nAxis.z;
            }
        }

        Vec.normalize(vAxis);
        final Vec3f uAxis = Vec.cross(null, vAxis, nAxis);

        Out._11 = uAxis.x;
        Out._12 = vAxis.x;
        Out._13 = nAxis.x;
        Out._14 = 0.0f;
        Out._21 = uAxis.y;
        Out._22 = vAxis.y;
        Out._23 = nAxis.y;
        Out._24 = 0.0f;
        Out._31 = uAxis.z;
        Out._32 = vAxis.z;
        Out._33 = nAxis.z;
        Out._34 = 0.0f;

        Out._41 = -dot(Eye, uAxis);
        Out._42 = -dot(Eye, vAxis);
        Out._43 = -dot(Eye, nAxis);
        Out._44 = 1.0f;

        return Out;
    }

    public static Matrixf lookAtLH(Matrixf Out, final Vec3f Eye, final Vec3f At, final Vec3f Up)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        final Vec3f nAxis = Vec.normalize(Vec.sub(null, At, Eye));
        final Vec3f vAxis = Vec.sub(null, Up, nAxis);
        Vec.mul(vAxis, vAxis, dot(Up, nAxis));
        if (lengthSq(vAxis) < EPSILON5) {
            vAxis.x = -nAxis.y * nAxis.x;
            vAxis.y = 1.0f - nAxis.y * nAxis.y;
            vAxis.z = -nAxis.y * nAxis.z;
            if (lengthSq(vAxis) < EPSILON5) {
                vAxis.x = -nAxis.z * nAxis.x;
                vAxis.y = -nAxis.z * nAxis.y;
                vAxis.z = 1.0f - nAxis.z * nAxis.z;
            }
        }

        Vec.normalize(vAxis);
        final Vec3f uAxis = Vec.cross(null, vAxis, nAxis);

        Out._11 = uAxis.x;
        Out._12 = vAxis.x;
        Out._13 = nAxis.x;
        Out._14 = 0.0f;
        Out._21 = uAxis.y;
        Out._22 = vAxis.y;
        Out._23 = nAxis.y;
        Out._24 = 0.0f;
        Out._31 = uAxis.z;
        Out._32 = vAxis.z;
        Out._33 = nAxis.z;
        Out._34 = 0.0f;

        Out._41 = -dot(Eye, uAxis);
        Out._42 = -dot(Eye, vAxis);
        Out._43 = -dot(Eye, nAxis);
        Out._44 = 1.0f;

        return Out;
    }

    public static Matrixf orthoRH(Matrixf Out, final float w, final float h, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        assert Math.abs(w) > EPSILON5;
        assert Math.abs(h) > EPSILON5;

        float nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = 1.0f / nfRatio;

        Out._11 = 2.0f / w;
        Out._22 = 2.0f / h;
        Out._33 = -nfRatio;
        Out._43 = -nfRatio * zn;
        Out._44 = 1.0f;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._34 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;

        return Out;
    }

    public static Matrixf orthoLH(Matrixf Out, final float w, final float h, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        assert Math.abs(w) > EPSILON5;
        assert Math.abs(h) > EPSILON5;

        float nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = 1.0f / nfRatio;

        Out._11 = 2.0f / w;
        Out._22 = 2.0f / h;
        Out._33 = nfRatio;
        Out._43 = -nfRatio * zn;
        Out._44 = 1.0f;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._34 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;

        return Out;
    }

    public static Matrixf orthoOffCenterRH(Matrixf Out, final float l, final float r, final float t, final float b, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio, width, height;

        width = r - l;
        height = b - t;

        assert Math.abs(width) > EPSILON5;
        assert Math.abs(height) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = 1.0f / nfRatio;

        Out._11 = 2.0f / width;
        Out._22 = 2.0f / height;
        Out._33 = -nfRatio;
        Out._41 = -(l + r) / width;
        Out._42 = -(t + b) / height;
        Out._43 = -nfRatio * zn;
        Out._44 = 1.0f;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._34 = 0.0f;

        return Out;
    }

    public static Matrixf orthoOffCenterLH(Matrixf Out, final float l, final float r, final float t, final float b, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio, width, height;

        width = r - l;
        height = b - t;

        assert Math.abs(width) > EPSILON5;
        assert Math.abs(height) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = 1.0f / nfRatio;

        Out._11 = 2.0f / width;
        Out._22 = 2.0f / height;
        Out._33 = nfRatio;
        Out._41 = -(l + r) / width;
        Out._42 = -(t + b) / height;
        Out._43 = -nfRatio * zn;
        Out._44 = 1.0f;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._34 = 0.0f;

        return Out;
    }

    public static Matrixf perspectiveRH(Matrixf Out, final float w, final float h, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio;

        assert Math.abs(w) > EPSILON5;
        assert Math.abs(h) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        Out._11 = (2.0f * zn) / w;
        Out._22 = (2.0f * zn) / h;
        Out._33 = -nfRatio;
        Out._34 = -1.0f;
        Out._43 = -nfRatio * zn;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;

        return Out;
    }

    public static Matrixf perspectiveLH(Matrixf Out, final float w, final float h, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio;

        assert Math.abs(w) > EPSILON5;
        assert Math.abs(h) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        Out._11 = (2.0f * zn) / w;
        Out._22 = (2.0f * zn) / h;
        Out._33 = nfRatio;
        Out._34 = 1.0f;
        Out._43 = -nfRatio * zn;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;

        return Out;
    }

    public static Matrixf perspectiveFovRH(Matrixf Out, final float fovy, final float aspect, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        double nfRatio, cotFovY;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        cotFovY = Math.sin(fovy * 0.5f);
        assert cotFovY != 0;
        cotFovY = Math.cos(fovy * 0.5f) / cotFovY;

        Out._11 = (float)(cotFovY / aspect);
        Out._22 = (float)cotFovY;
        Out._33 = (float)-nfRatio;
        Out._34 = -1.0f;
        Out._43 = (float)(-nfRatio * zn);

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;

        return Out;
    }

    public static Matrixf perspectiveFovLH(Matrixf Out, final float fovy, final float aspect, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        double nfRatio, cotFovY;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        cotFovY = Math.sin(fovy * 0.5f);
        assert cotFovY != 0;
        cotFovY = Math.cos(fovy * 0.5f) / cotFovY;

        Out._11 = (float)(cotFovY / aspect);
        Out._22 = (float)cotFovY;
        Out._33 = (float)nfRatio;
        Out._34 = 1.0f;
        Out._43 = (float)(-nfRatio * zn);

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;
        return Out;
    }

    public static Matrixf perspectiveOffCenterRH(Matrixf Out, final float l, final float r, final float t, final float b, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio, width, height;

        width = r - l;
        height = b - t;

        assert Math.abs(width) > EPSILON5;
        assert Math.abs(height) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        Out._11 = (2.0f * zn) / width;
        Out._22 = (2.0f * zn) / height;
        Out._31 = (l + r) / width;
        Out._32 = (t + b) / height;
        Out._33 = -nfRatio;
        Out._34 = -1.0f;
        Out._43 = -nfRatio * zn;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;

        return Out;
    }

    public static Matrixf perspectiveOffCenterLH(Matrixf Out, final float l, final float r, final float t, final float b, final float zn, final float zf)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        float nfRatio, width, height;

        width = r - l;
        height = b - t;

        assert Math.abs(width) > EPSILON5;
        assert Math.abs(height) > EPSILON5;

        nfRatio = zf - zn;
        assert nfRatio != 0;
        nfRatio = zf / nfRatio;

        Out._11 = (2.0f * zn) / width;
        Out._22 = (2.0f * zn) / height;
        Out._31 = -(l + r) / width;
        Out._32 = -(t + b) / height;
        Out._33 = nfRatio;
        Out._34 = 1.0f;
        Out._43 = -nfRatio * zn;

        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._44 = 0.0f;

        return Out;
    }

    public static Matrixf reflect(Matrixf Out, final Vec4f Plane) {
        if (Out == null) {
            Out = new Matrixf();
        }

        float a = Plane.x;
        float b = Plane.y;
        float c = Plane.z;
        float d = Plane.w;

        float mag = a * a + b * b + c * c;
        if (Math.abs(1.0f - mag) > EPSILON5) {
            mag = (float)Math.sqrt(mag);
            assert mag != 0;
            mag = 1.0f / mag;
            a *= mag;
            b *= mag;
            c *= mag;
            d *= mag;
        }

        final float tx = a * d;
        final float ty = b * d;
        final float tz = c * d;

        Out._11 = -(2.0f * a * a - 1.0f);
        Out._21 = -(2.0f * a * b);
        Out._31 = -(2.0f * a * c);

        Out._12 = -(2.0f * b * a);
        Out._22 = -(2.0f * b * b - 1.0f);
        Out._32 = -(2.0f * b * c);

        Out._13 = -(2.0f * c * a);
        Out._23 = -(2.0f * c * b);
        Out._33 = -(2.0f * c * c - 1.0f);

        Out._41 = 2.0f * (Out._11 * tx + Out._21 * ty + Out._31 * tz);
        Out._42 = 2.0f * (Out._12 * tx + Out._22 * ty + Out._32 * tz);
        Out._43 = 2.0f * (Out._13 * tx + Out._23 * ty + Out._33 * tz);

        Out._14 = 0.0f;
        Out._24 = 0.0f;
        Out._34 = 0.0f;
        Out._44 = 1.0f;
        return Out;
    }

    public static Matrixf mtxRotation(Matrixf Out, final Vec3f V, final double radians)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        final float sqLen = lengthSq(V);
        if (fuzzyEqual(sqLen, 0.0f, EPSILON5)) {
            Out.setIdentity();
            return Out;
        }

        // normalize
        final double len = Math.sqrt(sqLen);
        final double ilen = 1.0 / len;
        final double x = V.x * ilen;
        final double y = V.y * ilen;
        final double z = V.z * ilen;

        // negate the angle to rotate clockwise (the algo rotates counter-clockwise)
        final double fSin = Math.sin(radians);
        final double fCos = Math.cos(radians);
        final double fInvCos = 1.0f - fCos;

        Out._11 = (float)(fInvCos * x * x + fCos);
        Out._12 = (float)(fInvCos * x * y - z * fSin);
        Out._13 = (float)(fInvCos * x * z + y * fSin);
        Out._14 = 0.0f;

        Out._21 = (float)(fInvCos * x * y + z * fSin);
        Out._22 = (float)(fInvCos * y * y + fCos);
        Out._23 = (float)(fInvCos * y * z - x * fSin);
        Out._24 = 0.0f;

        Out._31 = (float)(fInvCos * x * z - y * fSin);
        Out._32 = (float)(fInvCos * y * z + x * fSin);
        Out._33 = (float)(fInvCos * z * z + fCos);
        Out._34 = 0.0f;

        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._43 = 0.0f;
        Out._44 = 1.0f;

        return Out;
    }

    public static Matrixf mtxRotation(Matrixf Out, final double yaw, final double pitch, final double roll)
    {
        if (Out == null) {
            Out = new Matrixf();
        }

        double sinY, cosY, sinP, cosP, sinR, cosR;
        double ux, uy, uz, vx, vy, vz, nx, ny, nz;

        sinY = (Math.sin(yaw));
        cosY = (Math.cos(yaw));
        sinP = (Math.sin(pitch));
        cosP = (Math.cos(pitch));
        sinR = (Math.sin(roll));
        cosR = (Math.cos(roll));

        ux = cosY * cosR + sinY * sinP * sinR;
        uy = sinR * cosP;
        uz = cosY * sinP * sinR - sinY * cosR;

        vx = sinY * sinP * cosR - cosY * sinR;
        vy = cosR * cosP;
        vz = sinR * sinY + cosR * cosY * sinP;

        nx = cosP * sinY;
        ny = -sinP;
        nz = cosP * cosY;

        Out._11 = (float)ux;
        Out._12 = (float)uy;
        Out._13 = (float)uz;
        Out._14 = 0.0f;
        Out._21 = (float)vx;
        Out._22 = (float)vy;
        Out._23 = (float)vz;
        Out._24 = 0.0f;
        Out._31 = (float)nx;
        Out._32 = (float)ny;
        Out._33 = (float)nz;
        Out._34 = 0.0f;
        Out._41 = 0.0f;
        Out._42 = 0.0f;
        Out._43 = 0.0f;
        Out._44 = 1.0f;

        return Out;
    }

    public static Matrixf scaling(Matrixf Out, final float sx, final float sy, final float sz)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out.setIdentity();
        Out._11 = sx;
        Out._22 = sy;
        Out._33 = sz;
        return Out;
    }

    /**
     * Computes a scaling matrix which garantees the matrix's handness is
     * preserved.
     */
    public static Matrixf scalingH(Matrixf Out, final float sx, final float sy, final float sz, final float sh)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out.setIdentity();
        Out._11 = sx * sh;
        Out._22 = sy * sh;
        Out._33 = sz * sh;
        return Out;
    }

    /**
     * Computes a scaling matrix which garantees the matrix's handness is
     * preserved.
     */
    public static Matrixf scalingH(final Matrixf Out, final Vec4f s) {
        return scalingH(Out, s.x, s.y, s.z, s.w);
    }

    public static Matrixf scale(Matrixf Out, final Matrixf M, final float x, final float y, final float z)
    {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }

        // Right
        Out._11 = M._11 * x;
        Out._21 = M._21 * x;
        Out._31 = M._31 * x;

        // Up
        Out._12 = M._12 * y;
        Out._22 = M._22 * y;
        Out._32 = M._32 * y;

        // Fwd
        Out._13 = M._13 * z;
        Out._23 = M._23 * z;
        Out._33 = M._33 * z;

        return Out;
    }

    public static Matrixf scale(final Matrixf Out, final Matrixf M, final Vec3f s) {
        return scale(Out, M, s.x, s.y, s.z);
    }

    public static Matrixf translation(Matrixf Out, final float x, final float y, final float z)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out.setIdentity();
        Out._41 = x;
        Out._42 = y;
        Out._43 = z;
        return Out;
    }

    public static Matrixf translation(final Matrixf Out, final Vec3f t) {
        return translation(Out, t.x, t.y, t.z);
    }

    public static Matrixf transpose3x3(Matrixf Out, final Matrixf M) {
        if (Out == M) {
            // swap components
            float t;
            t = Out._12;
            Out._12 = Out._21;
            Out._21 = t;
            t = Out._13;
            Out._13 = Out._31;
            Out._31 = t;
            t = Out._23;
            Out._23 = Out._32;
            Out._32 = t;
        }
        else {
            if (Out == null) {
                Out = new Matrixf();
            }
            Out._11 = M._11;
            Out._21 = M._12;
            Out._31 = M._13;
            Out._12 = M._21;
            Out._22 = M._22;
            Out._32 = M._23;
            Out._13 = M._31;
            Out._23 = M._32;
            Out._33 = M._33;
        }
        return Out;
    }

    public static Matrixf textureOffset2(Matrixf Out, final float fBias, final int nTexW, final int nTexH, final int nDepthBits)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        final float fOffsetX = 0.5f + (0.5f / nTexW);
        final float fOffsetY = 0.5f + (0.5f / nTexH);
        final float range = (0xFFFFFFFF >> (32 - nDepthBits));
        Out._11 = 0.5f;
        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._22 = -0.5f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._33 = range;
        Out._34 = 0.0f;
        Out._41 = fOffsetX;
        Out._42 = fOffsetY;
        Out._43 = fBias * range;
        Out._44 = 1.0f;
        return Out;
    }

    public static Matrixf textureOffset(Matrixf Out, final float fBias, final int nTexW, final int nTexH)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        final float fOffsetX = 0.5f + (0.5f / nTexW);
        final float fOffsetY = 0.5f + (0.5f / nTexH);
        Out._11 = 0.5f;
        Out._12 = 0.0f;
        Out._13 = 0.0f;
        Out._14 = 0.0f;
        Out._21 = 0.0f;
        Out._22 = -0.5f;
        Out._23 = 0.0f;
        Out._24 = 0.0f;
        Out._31 = 0.0f;
        Out._32 = 0.0f;
        Out._33 = 1.0f;
        Out._34 = 0.0f;
        Out._41 = fOffsetX;
        Out._42 = fOffsetY;
        Out._43 = fBias;
        Out._44 = 1.0f;
        return Out;
    }

    public static Matrixf setForward(Matrixf Out, final Matrixf M, final Vec3f V) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._13 = V.x;
        Out._23 = V.y;
        Out._33 = V.z;
        return Out;
    }

    public static Matrixf setForward(final Matrixf M, final Vec3f V) {
        return setForward(M, M, V);
    }

    public static Matrixf setUp(Matrixf Out, final Matrixf M, final Vec3f V) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._12 = V.x;
        Out._22 = V.y;
        Out._32 = V.z;
        return Out;
    }

    public static Matrixf setUp(final Matrixf M, final Vec3f V) {
        return setUp(M, M, V);
    }

    public static Matrixf setRight(Matrixf Out, final Matrixf M, final Vec3f V) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._11 = V.x;
        Out._21 = V.y;
        Out._31 = V.z;
        return Out;
    }

    public static Matrixf setRight(final Matrixf M, final Vec3f V) {
        return setRight(M, M, V);
    }

    public static Matrixf setTranslation(Matrixf Out, final Matrixf M, final Vec3f V) {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._41 = V.x;
        Out._42 = V.y;
        Out._43 = V.z;
        return Out;
    }

    public static Matrixf setTranslation(final Matrixf M, final Vec3f V) {
        return setTranslation(M, M, V);
    }

    public static Vec3f getForward(Vec3f Out, final Matrixf M) {
        if (Out == null) {
            Out = Vec.Vec3f();
        }
        Out.x = M._13;
        Out.y = M._23;
        Out.z = M._33;
        return Out;
    }

    public static Vec3f getUp(Vec3f Out, final Matrixf M) {
        if (Out == null) {
            Out = Vec.Vec3f();
        }
        Out.x = M._12;
        Out.y = M._22;
        Out.z = M._32;
        return Out;
    }

    public static Vec3f getRight(Vec3f Out, final Matrixf M) {
        if (Out == null) {
            Out = Vec.Vec3f();
        }
        Out.x = M._11;
        Out.y = M._21;
        Out.z = M._31;
        return Out;
    }

    public static Vec3f getTranslation(Vec3f Out, final Matrixf M) {
        if (Out == null) {
            Out = Vec.Vec3f();
        }
        Out.x = M._41;
        Out.y = M._42;
        Out.z = M._43;
        return Out;
    }

    public static Matrixf setRotation(Matrixf Out, final Matrixf M) {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }
        Out._11 = M._11;
        Out._12 = M._12;
        Out._13 = M._13;
        Out._21 = M._21;
        Out._22 = M._22;
        Out._23 = M._23;
        Out._31 = M._31;
        Out._32 = M._32;
        Out._33 = M._33;
        return Out;
    }

    Vec3f getProjectedTranslation(Vec3f Out, final Matrixf M, final float Mdet) {
        if (Out == null) {
            Out = Vec.Vec3f();
        }

        if (Mdet == 0) {
            Out.x = Out.y = Out.z = 0;
            return Out;
        }

        float a1, b1, c1;
        float a2, b2, c2;
        float a3, b3, c3;
        float a4, b4, c4;
        final float det = 1.0f / Mdet;

        a1 = M._11;
        b1 = M._12;
        c1 = M._13;
        a2 = M._21;
        b2 = M._22;
        c2 = M._23;
        a3 = M._31;
        b3 = M._32;
        c3 = M._33;
        a4 = M._41;
        b4 = M._42;
        c4 = M._43;

        Out.x = -det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4) * det;
        Out.y = det3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4) * det;
        Out.z = -det3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4) * det;

        return Out;
    }

    Vec3f getProjectedTranslation(final Vec3f Out, final Matrixf M) {
        return getProjectedTranslation(Out, M, det(M));
    }

    public static Matrixf setProjectedTranslation(Matrixf Out, final Matrixf M, final Vec3f avT)
    {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }
        final Vec3f X = getRight(null, M);
        final Vec3f Y = getUp(null, M);
        final Vec3f Z = getForward(null, M);
        final Vec3f Trans = Vec.Vec3f(
                -dot(X, avT),
                -dot(Y, avT),
                -dot(Z, avT));
        return setTranslation(Out, Trans);
    }

    public static Matrixf setProjectedTranslation(final Matrixf M, final Vec3f avT) {
        return setProjectedTranslation(M, M, avT);
    }

    public static Matrixf setNotRotation(Matrixf Out, final Matrixf M) {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }
        Out._14 = M._14;
        Out._24 = M._24;
        Out._34 = M._34;
        Out._41 = M._41;
        Out._42 = M._42;
        Out._43 = M._43;
        Out._44 = M._44;
        return Out;
    }

    public static Matrixf rotate(Matrixf Out, final Matrixf A, final Matrixf B) {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }

        float a1, a2, a3;

        // 1st row * 1st column
        a1 = A._11;
        a2 = A._12;
        a3 = A._13;
        final float r11 = a1 * B._11 + a2 * B._21 + a3 * B._31;
        final float r12 = a1 * B._12 + a2 * B._22 + a3 * B._32;
        final float r13 = a1 * B._13 + a2 * B._23 + a3 * B._33;

        // 2nd row * 2nd column
        a1 = A._21;
        a2 = A._22;
        a3 = A._23;
        final float r21 = a1 * B._11 + a2 * B._21 + a3 * B._31;
        final float r22 = a1 * B._12 + a2 * B._22 + a3 * B._32;
        final float r23 = a1 * B._13 + a2 * B._23 + a3 * B._33;

        // 3rd row * 3rd column
        a1 = A._31;
        a2 = A._32;
        a3 = A._33;
        final float r31 = a1 * B._11 + a2 * B._21 + a3 * B._31;
        final float r32 = a1 * B._12 + a2 * B._22 + a3 * B._32;
        final float r33 = a1 * B._13 + a2 * B._23 + a3 * B._33;

        Out._11 = r11;
        Out._12 = r12;
        Out._13 = r13;
        Out._21 = r21;
        Out._22 = r22;
        Out._23 = r23;
        Out._31 = r31;
        Out._32 = r32;
        Out._33 = r33;
        return Out;
    }

    Vec3f getLocalScale(Vec3f aS, final Matrixf In) {
        if (aS == null) {
            aS = Vec.Vec3f();
        }
        aS.x = (float)Math.sqrt(In._11 * In._11 + In._12 * In._12 + In._13 * In._13);
        aS.y = (float)Math.sqrt(In._21 * In._21 + In._22 * In._22 + In._23 * In._23);
        aS.z = (float)Math.sqrt(In._31 * In._31 + In._32 * In._32 + In._33 * In._33);
        return aS;
    }

    Vec3f getLocalScaleSq(Vec3f aS, final Matrixf In) {
        if (aS == null) {
            aS = Vec.Vec3f();
        }
        aS.x = (In._11 * In._11 + In._12 * In._12 + In._13 * In._13);
        aS.y = (In._21 * In._21 + In._22 * In._22 + In._23 * In._23);
        aS.z = (In._31 * In._31 + In._32 * In._32 + In._33 * In._33);
        return aS;
    }

    public static Matrixf viewport(Matrixf Out, final Vec4f aVP, final float aMinZ, final float aMaxZ)
    {
        if (Out == null) {
            Out = MATRIXF_IDENTITY.clone();
        }
        final float w2 = (aVP.getRight() - aVP.getLeft()) / 2.0f;
        final float h2 = (aVP.getBottom() - aVP.getTop()) / 2.0f;
        Out._11 = w2;
        Out._22 = -h2;
        Out._33 = aMaxZ - aMinZ;
        Out._41 = aVP.getLeft() + w2;
        Out._42 = aVP.getTop() + h2;
        Out._43 = aMinZ;
        return Out;
    }

    public static Matrixf adjustViewport(final Matrixf Out, final Vec4f aContextVP, final Vec4f aVirtualVP, final float afMinZ, final float afMaxZ)
    {
        if (aContextVP.equals(aVirtualVP)) {
            if (Out == null) {
                return MATRIXF_IDENTITY.clone();
            }
            else {
                Out.setIdentity();
                return Out;
            }
        }
        else {
            final Matrixf mtxInvCtxVP = invert(viewport(null, aContextVP, afMinZ, afMaxZ));
            final Matrixf mtxVirtVP = viewport(null, aVirtualVP, afMinZ, afMaxZ);
            return mul(Out, mtxVirtVP, mtxInvCtxVP);
        }
    }

    public static final float getFovV(final Matrixf M) {
        final float h = M._22; // get the view space height
        return 2.0f * (float)Math.atan(1.0f / h);
    }

    public static boolean isNormal(final Matrixf M, final float epsilon) {
        final Vec3f t = Vec.Vec3f();
        if (!Vec.isNormal(getRight(t, M), epsilon))
            return false;
        if (!Vec.isNormal(getUp(t, M), epsilon))
            return false;
        if (!Vec.isNormal(getForward(t, M), epsilon))
            return false;
        return true;
    }

    public static boolean isNormal(final Matrixf M) {
        return isNormal(M, EPSILON5);
    }

    public static boolean isOrthogonal(final Matrixf M, final float epsilon) {
        final Vec3f r = Vec.normalize(getRight(null, M));
        final Vec3f u = Vec.normalize(getUp(null, M));
        final Vec3f f = Vec.normalize(getForward(null, M));
        return ((Math.abs(dot(r, u)) <= epsilon) &&
                (Math.abs(dot(r, f)) <= epsilon) && (Math.abs(dot(f, u)) <= epsilon));
    }

    public static boolean isOrthogonal(final Matrixf M) {
        return isOrthogonal(M, EPSILON5);
    }

    public static boolean isOrthoNormal(final Matrixf M, final float epsilon) {
        final Vec3f r = Vec.normalize(getRight(null, M));
        if (!Vec.isNormal(r))
            return false;
        final Vec3f u = Vec.normalize(getUp(null, M));
        if (!Vec.isNormal(u))
            return false;
        final Vec3f f = Vec.normalize(getForward(null, M));
        if (!Vec.isNormal(f))
            return false;
        return ((Math.abs(dot(r, u)) <= epsilon) &&
                (Math.abs(dot(r, f)) <= epsilon) && (Math.abs(dot(f, u)) <= epsilon));
    }

    public static boolean isOrthoNormal(final Matrixf M) {
        return isOrthoNormal(M, EPSILON5);
    }

    public static Matrixf toCoordinateSystem(Matrixf Out,
            final Vec3f avRight,
            final Vec3f avUp,
            final Vec3f avFwd,
            final Vec3f avOrg)
    {
        if (Out == null) {
            Out = new Matrixf();
        }
        Out._11 = avRight.x;
        Out._21 = avUp.x;
        Out._31 = avFwd.x;
        Out._41 = avOrg.x;
        Out._12 = avRight.y;
        Out._22 = avUp.y;
        Out._32 = avFwd.y;
        Out._42 = avOrg.y;
        Out._13 = avRight.z;
        Out._23 = avUp.z;
        Out._33 = avFwd.z;
        Out._43 = avOrg.z;
        Out._14 = 0.0f;
        Out._24 = 0.0f;
        Out._34 = 0.0f;
        Out._44 = 1.0f;
        return Out;
    }

    public static Matrixf orthogonalize(Matrixf Out, final Matrixf In, final float epsilon)
    {
        if (Out == null) {
            Out = In.clone();
        }

        final Vec3f t = Vec.Vec3f();
        final Vec3f X = getRight(null, In);
        Vec3f Y, Z;

        {
            final float xl = lengthSq(X);
            if (xl <= epsilon)
                return null;

            Y = getUp(null, In);
            Vec.sub(Y, Y, Vec.mul(t, X, dot(X, Y) / xl));

            Z = getForward(null, In);
            Vec.sub(Z, Z, Vec.mul(t, X, dot(X, Z) / xl));
        }

        {
            final float yl = lengthSq(Y);
            if (yl <= epsilon)
                return null;
            Vec.sub(Z, Z, Vec.mul(t, Y, dot(Y, Z) / yl));
        }

        setRight(Out, X);
        setUp(Out, Y);
        return setForward(Out, Z);
    }

    public static Matrixf orthogonalize(final Matrixf Out, final Matrixf In) {
        return orthogonalize(Out, In, EPSILON5);
    }

    public static Matrixf normalize(Matrixf Out, final Matrixf In) {
        if (Out == null) {
            Out = In.clone();
        }
        final Vec3f t = Vec.Vec3f();
        setRight(Out, Vec.normalize(getRight(t, In)));
        setUp(Out, Vec.normalize(getUp(t, In)));
        setForward(Out, Vec.normalize(getForward(t, In)));
        return Out;
    }

    public static Matrixf normalize(final Matrixf In) {
        return normalize(In, In);
    }

    public static Matrixf orthoNormalize(Matrixf Out, final Matrixf In) {
        if (Out == null) {
            Out = In.clone();
        }
        if (orthogonalize(Out, In) == null)
            return null;
        return normalize(Out);
    }
}
