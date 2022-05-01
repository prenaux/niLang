package ni.android;

import java.io.File;

import ni.common.lang3.reflect.FieldUtils;
import ni.common.util.Log;
import ni.niLang.IFile;
import ni.niLang.IURLFileHandler;
import ni.niLang.IURLFileHandlerImpl;
import ni.niLang.Lib;
import ni.niLang.util.Files;
import ni.types.IUnknown;
import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import ni.android.util.URLFileHandlerFolder;
import ni.android.util.URLFileHandlerPrefixed;;

/**
 * Selena Application for Android
 * <p>
 * This class takes care of initializing all the low-level AOM services
 * automatically, to use it in your Android application just add
 * {@code <application android:name="selena.android.app.SelenaApplication" ... /> }
 * in your AndroidManifest.xml.
 *
 * @author Pierre Renaux
 */
public class App extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        initContextProperties(this);
    }

    /**
     * Initialize the aglSystem for the specified context.
     * <p>
     * Ideally you shouldn't be using this method directly but instead use
     * {@link App} as you Android application.
     *
     * @param ctx
     */
  static public void initContextProperties(final Context ctx) {

    final ApplicationInfo ai = ctx.getApplicationInfo();
    final File homeDir = ctx.getDir("home", Context.MODE_PRIVATE);
    String nativeLibDir;
    try {
      nativeLibDir = (String)FieldUtils.readField(ai,"nativeLibraryDir");
      Log.i("NativeLibDir from Reflection: " + nativeLibDir);
    }
    catch (Throwable e) {
      final String appDir = homeDir.getParent();
      nativeLibDir = appDir + "/lib";
      Log.i("NativeLibDir from DataDir: " + nativeLibDir);
    }

    System.setProperty(Lib.PROPERTY_NI_DIRS_HOME, homeDir.getAbsolutePath());
    System.setProperty(Lib.PROPERTY_NI_DIRS_DOWNLOADS, ctx.getDir("downloads", Context.MODE_PRIVATE).getAbsolutePath());
    System.setProperty(Lib.PROPERTY_NI_DIRS_BIN, nativeLibDir);

    // Add the zip/apk resource loader
    try {
      IFile fpAPK = Files.urlOpen(ai.publicSourceDir);
      IURLFileHandler handler = IURLFileHandler.query(Lib.createInstance("niLang.URLFileHandlerZip",fpAPK));
      Lib.getLang().setGlobalInstance("URLFileHandler.default", handler);
      URLFileHandlerFolder.create(handler, "scripts");
      URLFileHandlerPrefixed.create(handler, "niUI");
    }
    catch (Throwable e) {
      Log.e("Can't set apk as the application's source: " + ai.publicSourceDir);
    }
  }
}
