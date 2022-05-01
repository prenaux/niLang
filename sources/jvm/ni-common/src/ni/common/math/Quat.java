package ni.common.math;

import ni.types.Matrixf;
import ni.types.Vec3f;
import ni.types.Vec4f;

public class Quat extends Vec {
    protected Quat() {
    }

    //===========================================================================
    //
    // Vec4f
    //
    //===========================================================================
    public static final Vec4f QUAT_ZERO = Quatf(0, 0, 0, 0);
    public static final Vec4f QUAT_ONE = Quatf(1, 1, 1, 1);
    public static final Vec4f QUAT_MINUS_ONE = Quatf(-1, -1, -1, -1);
    public static final Vec4f QUAT_IDENTITY = Quatf(0, 0, 0, 1);

    public static Vec4f Quatf() {
        return new Vec4f();
    }

    public static Vec4f Quatf(final int x, final int y, final int z, final int w) {
        return (new Vec4f()).set(x, y, z, w);
    }

    public static Vec4f Quatf(final float x, final float y, final float z, final float w)
    {
        return (new Vec4f()).set(x, y, z, w);
    }

    public static Vec4f Quatf(final Object o) {
        return (new Vec4f()).set(o);
    }

    public static Vec4f quatConjugate(final Vec4f Out, final Vec4f Q) {
        return negate(Out, Q);
    }

    public static Vec4f quatExp(Vec4f Out, final Vec4f Q) {
        if (Out == null) {
            Out = new Vec4f();
        }
        final double theta = Math.sqrt(Q.x * Q.x + Q.y * Q.y + Q.z * Q.z);
        final double sinTheta = Math.sin(theta);
        if (Math.abs(sinTheta) >= EPSILON5) {
            final float coeff = (float)(sinTheta / theta);
            Out.x = coeff * Q.x;
            Out.y = coeff * Q.y;
            Out.z = coeff * Q.z;
        }
        else {
            Out.x = Q.x;
            Out.y = Q.y;
            Out.z = Q.z;
        }
        Out.w = (float)Math.cos(theta);
        return Out;
    }

    public static Vec4f quatLn(Vec4f Out, final Vec4f Q) {
        if (Out == null) {
            Out = new Vec4f();
        }
        if (Math.abs(Q.w) < 1.0f) {
            final double theta = Math.acos(Q.w);
            final double sinTheta = Math.sin(theta);
            if (Math.abs(sinTheta) >= EPSILON5) {
                final double coeff = theta / sinTheta;
                Out.x = (float)(coeff * Q.x);
                Out.y = (float)(coeff * Q.y);
                Out.z = (float)(coeff * Q.z);
                Out.w = 0.0f;
                return Out;
            }
        }
        Out.x = Q.x;
        Out.y = Q.y;
        Out.z = Q.z;
        Out.w = 0.0f;
        return Out;
    }

    public static Vec4f quatInverse(Vec4f Out, final Vec4f Q) {
        if (Out == null) {
            Out = new Vec4f();
        }
        float denom = Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w;
        assert denom != 0;
        denom = 1.0f / denom;
        Out.x = -Q.x * denom;
        Out.y = -Q.y * denom;
        Out.z = -Q.z * denom;
        Out.w = Q.w * denom;
        return Out;
    }

    public static Vec4f quatMul(Vec4f Out, final Vec4f Q1, final Vec4f Q2) {
        if (Out == null) {
            Out = new Vec4f();
        }
        float q1x, q1y, q1z, q1w;
        float q2x, q2y, q2z, q2w;
        q1x = Q1.x;
        q1y = Q1.y;
        q1z = Q1.z;
        q1w = Q1.w;
        q2x = Q2.x;
        q2y = Q2.y;
        q2z = Q2.z;
        q2w = Q2.w;
        Out.x = q2w * q1x + q2x * q1w + q2y * q1z - q2z * q1y;
        Out.y = q2w * q1y + q2y * q1w + q2z * q1x - q2x * q1z;
        Out.z = q2w * q1z + q2z * q1w + q2x * q1y - q2y * q1x;
        Out.w = q2w * q1w - q2x * q1x - q2y * q1y - q2z * q1z;
        return Out;
    }

    public static Vec4f quatRotation(Vec4f Out, final Vec3f Axis, final double radian)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        final double sinHalfAngle = Math.sin(0.5 * radian);
        Out.x = (float)(sinHalfAngle * Axis.x);
        Out.y = (float)(sinHalfAngle * Axis.y);
        Out.z = (float)(sinHalfAngle * Axis.z);
        Out.w = (float)Math.cos(0.5f * radian);
        return Out;
    }

    private final static int next[] = { 1, 2, 0 };

    public static Vec4f quatRotation(final Vec4f Out, final Matrixf M) {
        float trace, root;
        int i, j, k;
        trace = M._11 + M._22 + M._33;
        if (trace > 0.0f) {
            root = (float)Math.sqrt(trace + 1.0f);
            Out.w = 0.5f * root;
            root = 0.5f / root;

            Out.x = root * (M._23 - M._32);
            Out.y = root * (M._31 - M._13);
            Out.z = root * (M._12 - M._21);
        }
        else {
            i = 0;
            if (M._22 > M._11)
                i = 1;
            if (M._33 > M.getAt(i, i))
                i = 2;

            j = next[i];
            k = next[j];

            root = (float)Math.sqrt(M.getAt(i, i) - M.getAt(j, j) - M.getAt(k, k) + 1.0f);

            Out.setAt(i, 0.5f * root);
            root = 0.5f / root;
            Out.setAt(j, root * (M.getAt(i, j) + M.getAt(j, i)));
            Out.setAt(k, root * (M.getAt(i, k) + M.getAt(k, i)));
            Out.w = root * (M.getAt(j, k) - M.getAt(k, j));
        }
        return Out;
    }

    public static Vec4f quatRotation(Vec4f Out, final double yaw, final double pitch, final double roll)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        final double sinY = Math.sin(0.5f * yaw);
        final double cosY = Math.cos(0.5f * yaw);
        final double sinP = Math.sin(0.5f * pitch);
        final double cosP = Math.cos(0.5f * pitch);
        final double sinR = Math.sin(0.5f * roll);
        final double cosR = Math.cos(0.5f * roll);
        Out.x = (float)(cosR * sinP * cosY + sinR * cosP * sinY);
        Out.y = (float)(cosR * cosP * sinY - sinR * sinP * cosY);
        Out.z = (float)(sinR * cosP * cosY - cosR * sinP * sinY);
        Out.w = (float)(cosR * cosP * cosY + sinR * sinP * sinY);
        return Out;
    }

    public static Vec4f quatRotation(final Vec4f Out, final Vec3f from, final Vec3f to)
    {
        final Vec3f crossFT = Vec.cross(null, from, to);
        Vec.normalize(crossFT, crossFT);
        return quatRotation(Out, crossFT, Math.cos(Vec.dot(from, to)));
    }

    public static final int SLERP_SHORT = 0;
    public static final int SLERP_LONG = 1;
    public static final int SLERP_CW = 2;
    public static final int SLERP_CCW = 3;

    public static Vec4f quatSlerp(final Vec4f Out, final Vec4f Q1, final Vec4f Q2, final float aT, final int mode)
    {
        float cosTheta, invT;
        float T = aT;
        float q1x, q1y, q1z, q1w;
        float q2x, q2y, q2z, q2w;

        q1x = Q1.x;
        q1y = Q1.y;
        q1z = Q1.z;
        q1w = Q1.w;
        q2x = Q2.x;
        q2y = Q2.y;
        q2z = Q2.z;
        q2w = Q2.w;

        cosTheta = dot(Q1, Q2);
        if (((mode == SLERP_SHORT) && (cosTheta < 0.0)) ||
            ((mode == SLERP_LONG) && (cosTheta > 0.0)) ||
            ((mode == SLERP_CW) && (q1w > q2w)) ||
            ((mode == SLERP_CCW) && (q1w < q2w))) {
            cosTheta = -cosTheta;
            q2x = -q2x;
            q2y = -q2y;
            q2z = -q2z;
            q2w = -q2w;
        }

        invT = 1.0f - aT;
        if (1.0f - cosTheta > EPSILON5) {
            final double theta = Math.acos(cosTheta);
            final double sinTheta = Math.sin(theta);
            invT = (float)(Math.sin(theta * invT) / sinTheta);
            T = (float)(Math.sin(theta * aT) / sinTheta);
        }

        Out.x = invT * q1x + T * q2x;
        Out.y = invT * q1y + T * q2y;
        Out.z = invT * q1z + T * q2z;
        Out.w = invT * q1w + T * q2w;
        return Out;
    }

    public static Vec4f quatSlerp(final Vec4f Out, final Vec4f Q1, final Vec4f Q2, final float aT)
    {
        return quatSlerp(Out, Q1, Q2, aT, SLERP_SHORT);
    }

    public static Vec4f quatSquad(Vec4f Out, final Vec4f Q1, final Vec4f Q2, final Vec4f Q3, final Vec4f Q4, final float t)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        final Vec4f u = quatSlerp(null, Q1, Q4, t);
        final Vec4f v = quatSlerp(null, Q2, Q3, t);
        return quatSlerp(Out, u, v, 2.0f * t * (1.0f - t));
    }

    /**
     * Converts a quaternion to an AxisAngle.
     * @return An Axis angle that represents the same rotation as the
     *         quaternion, the axis is returned in xyz and the angle
     *         in w.
     */
    public static Vec4f quatToAxisAngle(Vec4f Out, final Vec4f Q) {
        if (Out == null) {
            Out = new Vec4f();
        }

        final float squareLength = Q.x * Q.x + Q.y * Q.y + Q.z * Q.z;
        if (squareLength > EPSILON5) {
            final float invLength = 1.0f / (float)Math.sqrt(squareLength);
            Out.x = Q.x * invLength;
            Out.y = Q.y * invLength;
            Out.z = Q.z * invLength;
            Out.w = 2.0f * (float)Math.acos(Q.w);
            return Out;
        }

        Out.x = 1.0f;
        Out.y = 0.0f;
        Out.z = 0.0f;
        Out.w = 0.0f;
        return Out;
    }

    /**
     * Generates a random rotation quaternion.
     * <p>
     * From: "Uniform Random Rotations", Ken Shoemake, Graphics Gems III, pg. 124-132
     */
    public static Vec4f quatRandom(Vec4f Out) {
        if (Out == null) {
            Out = new Vec4f();
        }
        final double x0 = Math.random();
        final double r1 = Math.sqrt(1.0 - x0);
        final double r2 = Math.sqrt(x0);
        final double t1 = Math.PI * 2.0 * Math.random();
        final double t2 = Math.PI * 2.0 * Math.random();
        final double c1 = Math.cos(t1);
        final double s1 = Math.sin(t1);
        final double c2 = Math.cos(t2);
        final double s2 = Math.sin(t2);
        Out.x = (float)(s1 * r1);
        Out.y = (float)(c1 * r1);
        Out.z = (float)(s2 * r2);
        Out.w = (float)(c2 * r2);
        return Out;
    }
}
