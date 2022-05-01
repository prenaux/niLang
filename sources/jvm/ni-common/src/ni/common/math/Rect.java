package ni.common.math;

import ni.types.Vec2f;
import ni.types.Vec2i;
import ni.types.Vec4f;
import ni.types.Vec4i;

public class Rect extends Vec {
    protected Rect() {
    }

    //===========================================================================
    //
    // Rectf
    //
    //===========================================================================
    public static final Vec4f RECTF_NULL = Vec4f();

    public static Vec4f Rectf() {
        return new Vec4f();
    }

    public static Vec4f Rectf(final int x, final int y, final int width, final int height)
    {
        return (new Vec4f()).set(x, y, x + width, y + height);
    }

    public static Vec4f Rectf(final float x, final float y, final float width, final float height)
    {
        return (new Vec4f()).set(x, y, x + width, y + height);
    }

    public static Vec4f Rectf(final Object o) {
        return (new Vec4f()).set(o);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4f move(Vec4f Out, final Vec4f V, final float x, final float y)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        Out.x = V.x + x;
        Out.z = V.z + x;
        Out.y = V.y + y;
        Out.w = V.w + y;
        return Out;
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4f move(final Vec4f Out, final Vec4f V, final Vec2f p) {
        return move(Out, V, p.x, p.y);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4f move(final Vec4f V, final float x, final float y) {
        return move(V, V, x, y);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4f move(final Vec4f V, final Vec2f p) {
        return move(V, V, p.x, p.y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4f moveTo(Vec4f Out, final Vec4f V, final float x, final float y)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        final float w = V.getWidth();
        final float h = V.getHeight();
        Out.x = x;
        Out.z = x + w;
        Out.y = y;
        Out.w = y + h;
        return Out;
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4f moveTo(final Vec4f Out, final Vec4f V, final Vec2f p)
    {
        return moveTo(Out, V, p.x, p.y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4f moveTo(final Vec4f V, final float x, final float y) {
        return moveTo(V, V, x, y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4f moveTo(final Vec4f V, final Vec2f p) {
        return moveTo(V, V, p.x, p.y);
    }

    public static boolean intersectRect(final Vec4f A, final Vec4f B) {
        if (B.getRight() < A.getLeft())
            return false;
        if (B.getBottom() < A.getTop())
            return false;
        if (B.getLeft() > A.getRight())
            return false;
        if (B.getTop() > A.getBottom())
            return false;
        return true;
    }

    public static boolean intersect(final Vec4f A, final float px, final float py) {
        return ((px >= A.getLeft()) && (px < A.getRight()) &&
                (py >= A.getTop()) && (py < A.getBottom()));
    }

    public static boolean intersect(final Vec4f A, final Vec2f aPoint) {
        return intersect(A, aPoint.x, aPoint.y);
    }

    public static Vec4f clipRect(Vec4f Out, final Vec4f A, final Vec4f aRect)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        Out.setLeft(
                aRect.getLeft() < A.getLeft() ? A.getLeft() :
                        (aRect.getLeft() > A.getRight()) ? A.getRight() : aRect.getLeft());
        Out.setRight(
                aRect.getRight() < A.getLeft() ? A.getLeft() :
                        (aRect.getRight() > A.getRight()) ? A.getRight() : aRect.getRight());
        Out.setTop(
                aRect.getTop() < A.getTop() ? A.getTop() :
                        (aRect.getTop() > A.getBottom()) ? A.getBottom() : aRect.getTop());
        Out.setBottom(
                aRect.getBottom() < A.getTop() ? A.getTop() :
                        (aRect.getBottom() > A.getBottom()) ? A.getBottom() : aRect.getBottom());
        return Out;
    }

    public static Vec4f getFrameCenter(Vec4f Out, final Vec4f A, final float aLeft, final float aRight, final float aTop, final float aBottom)
    {
        if (Out == null) {
            Out = new Vec4f();
        }
        Out.setLeft(A.getLeft() + aLeft);
        Out.setRight(A.getRight() - aRight);
        Out.setTop(A.getTop() + aTop);
        Out.setBottom(A.getBottom() - aBottom);
        return Out;
    }

    public static Vec4f getFrameCenter(final Vec4f Out, final Vec4f A, final Vec4f frame)
    {
        return getFrameCenter(Out, A, frame.getLeft(), frame.getRight(), frame.getTop(), frame.getBottom());
    }

    //===========================================================================
    //
    // Recti
    //
    //===========================================================================
    public static final Vec4i RECTI_ZERO = Vec4i(0, 0, 0, 0);

    public static Vec4i Recti() {
        return new Vec4i();
    }

    public static Vec4i Recti(final int x, final int y, final int width, final int height)
    {
        return (new Vec4i()).set(x, y, x + width, y + height);
    }

    public static Vec4i Recti(final float x, final float y, final float width, final float height)
    {
        return (new Vec4i()).set(x, y, x + width, y + height);
    }

    public static Vec4i Recti(final Object o) {
        return (new Vec4i()).set(o);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4i move(Vec4i Out, final Vec4i V, final int x, final int y)
    {
        if (Out == null) {
            Out = new Vec4i();
        }
        Out.x = V.x + x;
        Out.z = V.z + x;
        Out.y = V.y + y;
        Out.w = V.w + y;
        return Out;
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4i move(final Vec4i Out, final Vec4i V, final Vec2i p) {
        return move(Out, V, p.x, p.y);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4i move(final Vec4i V, final int x, final int y) {
        return move(V, V, x, y);
    }

    /**
     * Move the rectangle of the specified offset.
     */
    public static Vec4i move(final Vec4i V, final Vec2i p) {
        return move(V, V, p.x, p.y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4i moveTo(Vec4i Out, final Vec4i V, final int x, final int y)
    {
        if (Out == null) {
            Out = new Vec4i();
        }
        final int w = V.getWidth();
        final int h = V.getHeight();
        Out.x = x;
        Out.z = x + w;
        Out.y = y;
        Out.w = y + h;
        return Out;
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4i moveTo(final Vec4i Out, final Vec4i V, final Vec2i p)
    {
        return moveTo(Out, V, p.x, p.y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4i moveTo(final Vec4i V, final int x, final int y) {
        return moveTo(V, V, x, y);
    }

    /**
     * Move the rectangle to the specified position.
     */
    public static Vec4i moveTo(final Vec4i V, final Vec2i p) {
        return moveTo(V, V, p.x, p.y);
    }

    public static boolean intersectRect(final Vec4i A, final Vec4i B) {
        if (B.getRight() < A.getLeft())
            return false;
        if (B.getBottom() < A.getTop())
            return false;
        if (B.getLeft() > A.getRight())
            return false;
        if (B.getTop() > A.getBottom())
            return false;
        return true;
    }

    public static boolean intersect(final Vec4i A, final int px, final int py) {
        return ((px >= A.getLeft()) && (px < A.getRight()) &&
                (py >= A.getTop()) && (py < A.getBottom()));
    }

    public static boolean intersect(final Vec4i A, final Vec2i aPoint) {
        return intersect(A, aPoint.x, aPoint.y);
    }

    public static Vec4i clipRect(Vec4i Out, final Vec4i A, final Vec4i aRect)
    {
        if (Out == null) {
            Out = new Vec4i();
        }
        Out.setLeft(
                aRect.getLeft() < A.getLeft() ? A.getLeft() :
                        (aRect.getLeft() > A.getRight()) ? A.getRight() : aRect.getLeft());
        Out.setRight(
                aRect.getRight() < A.getLeft() ? A.getLeft() :
                        (aRect.getRight() > A.getRight()) ? A.getRight() : aRect.getRight());
        Out.setTop(
                aRect.getTop() < A.getTop() ? A.getTop() :
                        (aRect.getTop() > A.getBottom()) ? A.getBottom() : aRect.getTop());
        Out.setBottom(
                aRect.getBottom() < A.getTop() ? A.getTop() :
                        (aRect.getBottom() > A.getBottom()) ? A.getBottom() : aRect.getBottom());
        return Out;
    }

    public static Vec4i getFrameCenter(Vec4i Out, final Vec4i A, final int aLeft, final int aRight, final int aTop, final int aBottom)
    {
        if (Out == null) {
            Out = new Vec4i();
        }
        Out.setLeft(A.getLeft() + aLeft);
        Out.setRight(A.getRight() - aRight);
        Out.setTop(A.getTop() + aTop);
        Out.setBottom(A.getBottom() - aBottom);
        return Out;
    }

    public static Vec4i getFrameCenter(final Vec4i Out, final Vec4i A, final Vec4i frame)
    {
        return getFrameCenter(Out, A, frame.getLeft(), frame.getRight(), frame.getTop(), frame.getBottom());
    }
}
