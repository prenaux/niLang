package ni.android.gl;

import ni.types.Vec2i;
import ni.common.math.Vec;

public class GLOptions {
    public static boolean USE_FORCE_AA = false;
    public static boolean TRANSLUCENT_WINDOW = false;
    public static boolean USE_COLOR_BUFFER_32 = true;
    public static boolean USE_DEPTH_BUFFER_24 = true;
    public static boolean USE_DEFAULT_GL_CONTEXT = false;
    public static boolean USE_GL_POS = false;
    public static boolean USE_GL_LOWMEM = false;
    public static boolean USE_GL_ES2 = true;
    public static int DISPATCH_DRAW_WAIT_FOR_NFRAMES_COUNT = 1;
    public static boolean USE_CHECKHASBETTERCONFIG = true;

	public static Vec2i FIXED_SIZE = Vec.Vec2i(0,0);

    public static boolean LOG_THREADS = true;
    public static boolean LOG_SURFACE = true;
    public static boolean LOG_RENDERER = false;

    // Work-around for bug 2263168
    public static boolean DRAW_TWICE_AFTER_SIZE_CHANGED = true;
}
