package ni.android.gl;

import javax.microedition.khronos.opengles.GL10;

import ni.common.util.Log;

public class GLConfig {
    public int glesVersion = GLOptions.USE_GL_ES2 ? 2 : 1;
    /** The number of bits requested for the red component */
    public int redSize = (GLOptions.USE_COLOR_BUFFER_32 || GLOptions.TRANSLUCENT_WINDOW) ? 8 : 5;
    /** The number of bits requested for the green component */
    public int greenSize = (GLOptions.USE_COLOR_BUFFER_32 || GLOptions.TRANSLUCENT_WINDOW) ? 8 : 6;
    /** The number of bits requested for the blue component */
    public int blueSize = (GLOptions.USE_COLOR_BUFFER_32 || GLOptions.TRANSLUCENT_WINDOW) ? 8 : 5;
    /** The number of bits requested for the alpha component */
    public int alphaSize = (GLOptions.USE_COLOR_BUFFER_32 || GLOptions.TRANSLUCENT_WINDOW) ? 8 : 0;
    /** The number of bits requested for the depth component */
    public int depthSize = GLOptions.USE_DEPTH_BUFFER_24 ? 24 : 16;
    /** The number of bits requested for the stencil component */
    public int stencilSize = 8;
    /** The prefered number of AA samples **/
    public int aaSamples = GLOptions.USE_FORCE_AA ? 4 : 0;
    /** GL strings */
    public String strVendor = "";
    public String strRenderer = "";
    public String strVersion = "";
    // public String strExts = null;
    /** False when shouldChangeConfig has already been called */
    public boolean canChangeConfig = GLOptions.USE_CHECKHASBETTERCONFIG;

    GLConfig() {
    }

    private boolean checkHasBetterConfig(final GL10 gl) {
        boolean ret = false;

        try {
            strVendor = gl.glGetString(GL10.GL_VENDOR);
            strRenderer = gl.glGetString(GL10.GL_RENDERER);
            strVersion = gl.glGetString(GL10.GL_VERSION);
            Log.v("GL_VENDOR: " + strVendor);
            Log.v("GL_RENDERER: " + strRenderer);
            Log.v("GL_VERSION: " + strVersion);

            // MALI GPUs have free 4x AA (really...)
            if (strRenderer.contains("Mali-") ||
                strRenderer.contains("MALI-") ||
                strRenderer.contains("mali-"))
            {
                if (aaSamples != 4)
                    aaSamples = 4;
                ret = true; // changed the config...
            }
        }
        catch (final Exception e) {
            Log.v("Error", e);
        }

        return ret;
    }

    public boolean shouldChangeConfig(final GL10 gl) {
        if (!canChangeConfig)
            return false;
        canChangeConfig = false;
        return checkHasBetterConfig(gl);
    }
}
