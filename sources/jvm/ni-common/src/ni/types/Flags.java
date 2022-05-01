package ni.types;

public class Flags {
    private Flags() {
    }

    public static int bit(final int x) {
        return 1 << x;
    }

    public static int on(final int v, final int flags) {
        return v | flags;
    }

    public static int off(final int v, final int flags) {
        return v & (~flags);
    }

    public static int onIf(final int v, final int flags, final boolean cond) {
        return (cond) ?
                (v | flags) : (v);
    }

    public static int offIf(final int v, final int flags, final boolean cond) {
        return (cond) ?
                (v & (~flags)) : (v);
    }

    public static boolean test(final int v, final int flags) {
        return (flags != 0) && ((v & flags) == flags);
    }

    public static boolean is(final int v, final int flags) {
        return test(v, flags);
    }

    public static boolean isNot(final int v, final int flags) {
        return !test(v, flags);
    }

    public static boolean contained(final int fa, final int fb) {
        return ((fa & fb) != 0) || (fa == fb);
    }

    public static boolean testMask(final int flags, final int excluded, final int required)
    {
        if ((flags & excluded) != 0)
            return false;
        if (required == 0)
            return true;
        if ((flags & required) == 0)
            return false;
        return true;
    }
};
