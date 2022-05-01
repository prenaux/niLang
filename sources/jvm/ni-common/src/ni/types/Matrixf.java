package ni.types;

import ni.types.util.Hash;

/**
 * Holds a 4x4 float matrix.
 */
public class Matrixf implements ExtendedPrimitive {
    public static String __jniTypeName() {
        return "Matrixf";
    }

    public static String __jniNativeType() {
        return "ni::sMatrixf";
    }

    public static String __jniNullRet() {
        return "ni::sMatrixf::Identity()";
    }

    public float
            _11, _12, _13, _14,
            _21, _22, _23, _24,
            _31, _32, _33, _34,
            _41, _42, _43, _44;

    public Matrixf setAt(final int i, final float v) {
        switch (i) {
        case 0:
            this._11 = v;
            break;
        case 1:
            this._12 = v;
            break;
        case 2:
            this._13 = v;
            break;
        case 3:
            this._14 = v;
            break;
        case 4:
            this._21 = v;
            break;
        case 5:
            this._22 = v;
            break;
        case 6:
            this._23 = v;
            break;
        case 7:
            this._24 = v;
            break;
        case 8:
            this._31 = v;
            break;
        case 9:
            this._32 = v;
            break;
        case 10:
            this._33 = v;
            break;
        case 11:
            this._34 = v;
            break;
        case 12:
            this._41 = v;
            break;
        case 13:
            this._42 = v;
            break;
        case 14:
            this._43 = v;
            break;
        case 15:
            this._44 = v;
            break;
        default:
            throw new IndexOutOfBoundsException();
        }
        return this;
    }

    public Matrixf setAt(final int row, final int col, final float v) {
        return setAt(row * 4 + col, v);
    }

    public float getAt(final int i) {
        switch (i) {
        case 0:
            return this._11;
        case 1:
            return this._12;
        case 2:
            return this._13;
        case 3:
            return this._14;
        case 4:
            return this._21;
        case 5:
            return this._22;
        case 6:
            return this._23;
        case 7:
            return this._24;
        case 8:
            return this._31;
        case 9:
            return this._32;
        case 10:
            return this._33;
        case 11:
            return this._34;
        case 12:
            return this._41;
        case 13:
            return this._42;
        case 14:
            return this._43;
        case 15:
            return this._44;
        default:
            throw new IndexOutOfBoundsException();
        }
    }

    public float getAt(final int row, final int col) {
        return getAt(row * 4 + col);
    }

    /**
     * Set the matrix to the specified values.
     */
    public Matrixf set(final float e11, final float e12, final float e13, final float e14,
                    final float e21, final float e22, final float e23, final float e24,
                    final float e31, final float e32, final float e33, final float e34,
                    final float e41, final float e42, final float e43, final float e44)
    {
        this._11 = e11;
        this._12 = e12;
        this._13 = e13;
        this._14 = e14;
        this._21 = e21;
        this._22 = e22;
        this._23 = e23;
        this._24 = e24;
        this._31 = e31;
        this._32 = e32;
        this._33 = e33;
        this._34 = e34;
        this._41 = e41;
        this._42 = e42;
        this._43 = e43;
        this._44 = e44;
        return this;
    }

    /**
     * Set this matrix to be the identity matrix.
     * @return this
     */
    public Matrixf setIdentity() {
        this._11 = 1.0f;
        this._12 = 0.0f;
        this._13 = 0.0f;
        this._14 = 0.0f;
        this._21 = 0.0f;
        this._22 = 1.0f;
        this._23 = 0.0f;
        this._24 = 0.0f;
        this._31 = 0.0f;
        this._32 = 0.0f;
        this._33 = 1.0f;
        this._34 = 0.0f;
        this._41 = 0.0f;
        this._42 = 0.0f;
        this._43 = 0.0f;
        this._44 = 1.0f;
        return this;
    }

    /**
     * Set this matrix to 0.
     * @return this
     */
    public Matrixf setZero() {
        this._11 = 0.0f;
        this._12 = 0.0f;
        this._13 = 0.0f;
        this._14 = 0.0f;
        this._21 = 0.0f;
        this._22 = 0.0f;
        this._23 = 0.0f;
        this._24 = 0.0f;
        this._31 = 0.0f;
        this._32 = 0.0f;
        this._33 = 0.0f;
        this._34 = 0.0f;
        this._41 = 0.0f;
        this._42 = 0.0f;
        this._43 = 0.0f;
        this._44 = 0.0f;
        return this;
    }

    /**
     * Copy the source matrix to the destination matrix
     * @param src The source matrix
     * @return this matrix
     */
    public Matrixf set(final Matrixf src) {
        this._11 = src._11;
        this._12 = src._12;
        this._13 = src._13;
        this._14 = src._14;
        this._21 = src._21;
        this._22 = src._22;
        this._23 = src._23;
        this._24 = src._24;
        this._31 = src._31;
        this._32 = src._32;
        this._33 = src._33;
        this._34 = src._34;
        this._41 = src._41;
        this._42 = src._42;
        this._43 = src._43;
        this._44 = src._44;
        return this;
    }

    /**
     * Returns a string representation of this matrix
     */
    @Override
    public String toString() {
        final StringBuilder buf = new StringBuilder();
        buf.append("Matrix(").
                append(_11).append(',').
                append(_12).append(',').
                append(_13).append(',').
                append(_14).append(',').
                append(_21).append(',').
                append(_22).append(',').
                append(_23).append(',').
                append(_24).append(',').
                append(_31).append(',').
                append(_32).append(',').
                append(_33).append(',').
                append(_34).append(',').
                append(_41).append(',').
                append(_42).append(',').
                append(_43).append(',').
                append(_44).append(')');
        return buf.toString();
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o)
            return true;
        if (!(o instanceof Matrixf))
            return false;
        final Matrixf rhs = (Matrixf)o;
        return (_11 == rhs._11) && (_12 == rhs._12) && (_13 == rhs._13) && (_14 == rhs._14)
               && (_21 == rhs._21) && (_22 == rhs._22) && (_23 == rhs._23) && (_24 == rhs._24)
               && (_31 == rhs._31) && (_32 == rhs._32) && (_33 == rhs._33) && (_34 == rhs._34)
               && (_41 == rhs._41) && (_42 == rhs._42) && (_43 == rhs._43) && (_44 == rhs._44);
    }

    @Override
    public int hashCode() {
        return Hash.hashMatrix(
                _11, _12, _13, _14,
                _21, _22, _23, _24,
                _31, _32, _33, _34,
                _41, _42, _43, _44);
    }

    @Override
    public Matrixf clone() {
        return (new Matrixf()).set(this);
    }

    /**
     * Copy the object into this matrix.
     * @throws ClassCastException if the object is not of a compatible type
     */
    public Matrixf set(final Object o) throws ClassCastException {
        return this.set((Matrixf)o);
    }
}
