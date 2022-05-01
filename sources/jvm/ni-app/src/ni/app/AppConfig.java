package ni.app;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import ni.niLang.ILang;
import ni.niLang.Lib;
import ni.niUI.EClearBuffersFlags;
import ni.types.ULColor;
import ni.types.Vec4i;

public class AppConfig {
    public final String hostOS;
    public final boolean isEmbedded;

    public String renDriver = null;
    public String renBBFmt = null;
    public String renDSFmt = null;
    public int renBBFlags = 0;
    public int renSwapInterval = 0;

    public String windowTitle = "niApp";
    public boolean windowCenter = true;
    public int windowShowFlags = 0;
    public int windowFullScreen = -1;
    public Vec4i windowRect = null;
    // < 0  -> No refresh timer, no auto refresh
    // == 0 -> No refresh timer, redraw immediately
    // > 0  -> Refresh timer every 'windowRefreshTimer' seconds
    public float windowRefreshTimer = 0;

    // Update and draw the window even if it is inactive.
    public boolean windowDrawWhenInactive = false;

	// Background clear
	public int clearBuffers = EClearBuffersFlags.ColorDepthStencil;
	public int clearColor = 0;

    // 0 none, 1 draw profiler, 2 fps counter only
    public int drawFps = 0;

    public AppConfig() {
        final ILang sys = Lib.getLang();
        hostOS = sys.getProperty("ni.loa.os").toLowerCase();
        if (hostOS.startsWith("ios") ||
            hostOS.startsWith("adr") ||
            hostOS.contains("embedded")) {
            isEmbedded = true;
        }
        else {
            isEmbedded = false;
        }

        // default rendering driver
        {
            String renDefault = "Auto";
            if (isEmbedded) {
                renDefault = "GL2";
                renBBFmt = "R5G6B5";
                renDSFmt = "D16";
            }
            else {
                if (hostOS.startsWith("nt")) {
                    renDefault = "D3D9";
                }
                else {
                    renDefault = "GL2";
                }
                renBBFmt = "R8G8B8A8";
                renDSFmt = "D24S8";
            }
            renDriver = Lib.jvmGetProperty("ni.renderer.driver", renDefault);
        }
    }
}
