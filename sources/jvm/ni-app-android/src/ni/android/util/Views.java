package ni.android.util;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import ni.common.math.Rect;
import ni.common.math.Vec;
import ni.common.util.Log;
import ni.types.Vec4i;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;

public class Views {
    protected Views() {
    }

    /**
     * Get the effective width (resolution) in pixels of the specified
     * SurfaceView
     */
    static public int getHolderWidth(final SurfaceView a) {
        return a.getHolder().getSurfaceFrame().width();
    }

    /**
     * Get the effective height (resolution) in pixels of the specified
     * SurfaceView
     */
    static public int getHolderHeight(final SurfaceView a) {
        return a.getHolder().getSurfaceFrame().height();
    }

    //////////////////////////////////////////////////
    public static boolean writeDrawable(final String absPath, final Drawable icon) throws FileNotFoundException
    {
        boolean savedIcon = false;
        if (icon != null) {
            BitmapDrawable bmpDrawable = (BitmapDrawable)icon;
            if (bmpDrawable != null) {
                Bitmap bmp = bmpDrawable.getBitmap();
                savedIcon = writeBitmap(absPath, bmp);
                bmp.recycle();
                bmp = null;
            }
            bmpDrawable.setCallback(null);
            bmpDrawable = null;
        }
        return savedIcon;
    }

    //////////////////////////////////////////////////
    public static boolean writeBitmap(final String absPath, final Bitmap bitmap)
    {
        if (bitmap != null) {
            try {
                // Try go guesstimate how much space the icon will take when serialized
                // to avoid unnecessary allocations/copies during the write.
                // int size = bitmap.getWidth() * bitmap.getHeight() * 4;
                // ByteArrayOutputStream out = new ByteArrayOutputStream(size);
                // bitmap.compress(Bitmap.CompressFormat.PNG, quality, out);
                final FileOutputStream out = new FileOutputStream(absPath);
                bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);
                out.flush();
                out.close();
                return true;
            }
            catch (final Exception e) {
                Log.w("Could not write bitmap: " + e.getMessage());
                e.printStackTrace();
            }
        }
        return false;
    }

    static public Bitmap createBitmapCache(final View b, final int w, final int h) {
        final Bitmap bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_4444);
        final Canvas c = new Canvas(bitmap);
        b.forceLayout();
        b.measure(w, h);
        b.layout(0, 0, w, h);
        b.draw(c);
        return bitmap;
    }

    static public View getClickableViewAt(final View v, final int x, final int y) {
        return getClickableViewAt(v, x, y, 0, 0);
    }

    static public View getClickableViewAt(final View v, final int x, final int y, final int absX, final int absY)
    {
        if (v == null)
            return null;

        if (v instanceof ViewGroup) {
            final ViewGroup group = (ViewGroup)v;
            final int childCount = group.getChildCount();
            for (int i = 0; i < childCount; ++i) {
                final View r = getClickableViewAt(group.getChildAt(i), x, y, absX + v.getLeft(), absY + v.getTop());
                if (r != null)
                    return r;
            }
        }

        if (v.isClickable()) {
            final Vec4i r = Vec.Vec4i(
                    v.getLeft() + absX, v.getTop() + absY,
                    v.getRight() + absX, v.getBottom() + absY);
            if (Rect.intersect(r, x, y)) {
                return v;
            }
        }

        return null;
    }
}
