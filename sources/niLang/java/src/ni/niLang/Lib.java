package ni.niLang;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.HashMap;

import ni.common.util.Log;
import ni.types.IUnknown;
import ni.types.annotations.Platform;
import ni.types.annotations.Static;

@Platform(include = {
        "stdafx.h",
        "niLang_JNI.h"
})
public class Lib {
    protected Lib() {
    }

    public static final String PROPERTY_NI_BUILD_DEFAULT = "ni.build.default";
    public static final String PROPERTY_NI_DIRS_BIN = "ni.dirs.bin";
    public static final String PROPERTY_NI_DIRS_DATA = "ni.dirs.data";
    public static final String PROPERTY_NI_DIRS_HOME = "ni.dirs.home";
    public static final String PROPERTY_NI_DIRS_DOWNLOADS = "ni.dirs.downloads";

    private static final String REDIST = "LOCAL";

	private static final int INVALID_HANDLE = 0xDEADBEEF;

    protected static ILang mLang = null;
    private static IFileSystem mRootFS = null;
    private static IMath mMath = null;

    public static String getDefaultBuild() {
        return Modules.mDefaultBuild;
    }

    public static String getModulePath(final String name) {
        loadLang();
        return Modules.getModulePath(name, Modules.mDefaultBuild);
    }

    @Static(call = "ni::GetLang()")
    private native static ILang _getLang();

    public static void loadLang(String systemBuild) {
        if (mLang == null) {
            Modules.loadModule("niLang", systemBuild);
            mLang = _getLang();
            mLang.loadModuleDef("niLang", null);
            if (Log.mLogSink == null) {
            	Log.mLogSink = new LogSink();
            }
        }
    }

    public static void loadLang() {
        loadLang(Modules.mDefaultBuild);
    }

    public static void loadModule(final String moduleName) {
        loadLang();
        Modules.loadModule(moduleName, Modules.mDefaultBuild);
    }

    public static ILang getLang() {
        loadLang();
        return mLang;
    }

    public static IFileSystem getRootFS() {
        if (mRootFS == null) {
            mRootFS = getLang().getRootFS();
        }
        return mRootFS;
    }

    public static IUnknown createInstance(final String oid, final Object a, final Object b) throws CreateInstanceException
    {
        if (!oid.contains("."))
            throw new CreateInstanceException("invalid object id: " + oid);

        if (getLang().getCreateInstanceMap().find(oid) == null)
        {
            final String moduleName = oid.substring(0, oid.indexOf("."));
            if (!Modules.isModuleLoaded(moduleName)) {
                loadModule(moduleName);
                if (!Modules.isModuleLoaded(moduleName)) {
                    throw new CreateInstanceException("Can't load module: " + moduleName);
                }
            }
        }

        final IUnknown o = getLang().createInstance(oid, a, b);
        if ((o == null) || !o.isOK()) {
            throw new CreateInstanceException("createInstance failed: " + oid);
        }

        return o;
    }

    public static IUnknown createGlobalInstance(final String oid, final Object a, final Object b) throws CreateInstanceException
    {
		IUnknown o = getLang().getGlobalInstance(oid);
		if (o != null) {
			return o;
		}

        o = createInstance(oid, a, b);

		getLang().setGlobalInstance(oid, o);

		return o;
    }

    public static IUnknown createInstance(final String oid, final Object a) throws CreateInstanceException
    {
        return createInstance(oid, a, null);
    }

    public static IUnknown createGlobalInstance(final String oid, final Object a) throws CreateInstanceException
    {
        return createGlobalInstance(oid, a, null);
    }

    public static IUnknown createInstance(final String oid) throws CreateInstanceException
    {
        return createInstance(oid, null, null);
    }

    public static IUnknown createGlobalInstance(final String oid) throws CreateInstanceException
    {
        return createGlobalInstance(oid, null, null);
    }

    public static IMath getMath() {
        if (mMath == null) {
            mMath = IMath.query(getLang().createInstance("niLang.Math", null, null));
        }
        return mMath;
    }

    private static IZip zip = null;

    public static IZip getZip() {
        if (zip == null) {
            zip = IZip.query(getLang().createInstance("niLang.Zip", null, null));
        }
        return zip;
    }

	private static IConcurrent mConcurrent = null;

    public static IConcurrent getConcurrent() {
        if (mConcurrent == null) {
            mConcurrent = IConcurrent.query(Lib.createGlobalInstance("niLang.Concurrent"));
        }
        return mConcurrent;
    }

    private static IConsole mConsole = null;

    public static IConsole getConsole() {
        if (mConsole == null) {
            mConsole = IConsole.query(Lib.createGlobalInstance("niLang.Console"));
        }
        return mConsole;
    }

    public static IHString hstr(final String str) {
        return getLang().createHString(str);
    }

    public static String getTestInputFilePath(final String fn) {
        final String dir = getLang().getProperty("ni.dirs.data") + "tests/";
        if (fn == null)
            return dir;
        return dir + fn;
    }

    public static String getTestOutputFilePath(final String fn) {
        final String dir = getLang().getProperty("ni.dirs.home") + "niApp/Tests/";
        getRootFS().fileMakeDir(dir);
        if (fn == null)
            return dir;
        return dir + fn;
    }

    public static void jvmSetProperty(final String name, final String value) {
        Modules.doSetProperty(name, value);
    }

    public static String jvmGetProperty(final String name, final String defaultValue) {
        return Modules.doGetProperty(name, defaultValue);
    }

    public static String jvmGetProperty(final String name) {
        return Modules.doGetProperty(name);
    }

    private static class Modules {

        protected static final String mDllPrefix;
        protected static final String mDllExt;
        protected static final String mBinDir;
        protected static final boolean mUseSystemLoadLibrary;
        private static final String mDefaultBuild;
        private static final HashMap<String,String> mLoadedModules = new HashMap<String,String>();

        static {
            final String jvmName = doGetProperty("java.vm.name").toLowerCase();
            System.out.println("W/ jvmName: " + jvmName);
            final String osName = doGetProperty("os.name").toLowerCase();
            System.out.println("W/ osName: " + osName);
            final String osArch = doGetProperty("os.arch").toLowerCase();
            System.out.println("W/ osArch: " + osArch);

            {
                String defaultBuild = doGetProperty(PROPERTY_NI_BUILD_DEFAULT);
                if (defaultBuild == null) {
                    defaultBuild = "ra";
                }
                mDefaultBuild = defaultBuild;
                if (defaultBuild.equals("ra") || defaultBuild.equals("da")) {
					// we're good...
                }
                else {
                    throw new RuntimeException("Invalid build type '" + mDefaultBuild + "'.");
                }
            }

            if (jvmName.startsWith("dalvik")) {
                mDllPrefix = "lib";
                mDllExt = ".so";
                mUseSystemLoadLibrary = true;
            }
            else if (osName.startsWith("linux")) {
                mDllPrefix = "lib";
                mDllExt = ".so";
                mUseSystemLoadLibrary = false;
            }
            else if (osName.startsWith("mac os x")) {
                // Mac OS X
                mDllPrefix = "lib";
                mDllExt = ".dylib";
                mUseSystemLoadLibrary = false;
            }
            else if (osName.contains("win")) {
                // windows
                mDllPrefix = "";
                mDllExt = ".dll";
                mUseSystemLoadLibrary = false;
            }
            else {
                throw new RuntimeException("OS not supported.");
            }

            {
                String binDir = doGetProperty(PROPERTY_NI_DIRS_BIN);
                if (binDir == null) {
                    binDir = doGetProperty("java.library.path");
                }
                if (binDir == null) {
                    throw new RuntimeException("No " + PROPERTY_NI_DIRS_BIN + " set.");
                }
                if (!binDir.endsWith("/")) {
                    binDir += "/";
                }
                if (dirExists(binDir) == null) {
                    throw new RuntimeException(PROPERTY_NI_DIRS_BIN + " (" + binDir + ") directory does not exist.");
                }
                doSetProperty(PROPERTY_NI_DIRS_BIN, binDir);
                mBinDir = binDir;
            }
            System.out.println("W/ binDir: " + mBinDir);
        }

        private static String getModuleASL(final String name) {
            final String fileName = name + ".asl";
            final String path = mBinDir + fileName;
            if (fileExists(path))
                return readToString(path);
            return null;
        }

        public static String getModuleFileName(final String name, final String build) {
            if (mUseSystemLoadLibrary) {
                return name + "_" + build;
            }
            else {
                return mDllPrefix + name + "_" + build + mDllExt;
            }
        }

        public static String getModulePath(final String name, final String defaultBuild) {
            final String loadedPath = mLoadedModules.get(name);
            if (loadedPath != null)
                return loadedPath;

            if (mUseSystemLoadLibrary) {
                return getModuleFileName(name, defaultBuild);
            }
            else {
                final String build;
                final String asl = getModuleASL(name);
                if ((asl != null) && asl.contains("[Release]")) {
                    build = "ra";
                }
                else if ((asl != null) && asl.contains("[Debug]")) {
                    build = "da";
                }
                else {
                    build = defaultBuild;
                }

                final String fileName = getModuleFileName(name, build);
                final String path = mBinDir + fileName;
                if (fileExists(path))
                    return path;

                return null;
            }
        }

        public static boolean isModuleLoaded(final String name) {
            return mLoadedModules.containsKey(name);
        }

        private static String doGetProperty(final String name, final String defaultValue)
        {
            return AccessController.doPrivileged(new PrivilegedAction<String>() {
                @Override
                public String run() {
                    try {
                        return System.getProperty(name, defaultValue);
                    }
                    catch (final Exception e) {
                        e.printStackTrace();
                    }
                    return defaultValue;
                }
            });
        }

        private static String doGetProperty(final String name) {
            return doGetProperty(name, null);
        }

        private static void doSetProperty(final String name, final String value) {
            AccessController.doPrivileged(new PrivilegedAction<Object>() {
                @Override
                public Object run() {
                    try {
                        System.setProperty(name, value);
                    }
                    catch (final Exception e) {
                        e.printStackTrace();
                    }
                    return null;
                }
            });
        }

        private static void doLoadLibrary(final String name) {
            AccessController.doPrivileged(new PrivilegedAction<Object>() {
                @Override
                public Object run() {
                    try {
                        if (mUseSystemLoadLibrary) {
                            System.loadLibrary(name);
                        }
                        else {
                            System.load(name);
                        }
                    }
                    catch (final Exception e) {
                        e.printStackTrace();
                    }
                    return null;
                }
            });
        }

        public static void loadModule(final String name, final String build) throws UnsatisfiedLinkError
        {
            // Already loaded ?
            if (mLoadedModules.containsKey(name))
                return;

            System.out.println("I/ Trying to load Module '" + name + "' (" + build + ").");

            final String modPath = getModulePath(name, build);
            if (modPath == null) {
                throw new RuntimeException("Can't find module '" + name + "' path.");
            }

            System.out.println("I/ Loading Module '" + name + "' from '" + modPath + "'.");
            doLoadLibrary(modPath);
            if (!name.equals("niLang")) {
            	Lib.getLang().loadModuleDef(name, null);
            }
            mLoadedModules.put(name, modPath);
        }

        protected static boolean fileExists(final String absPath) {
            return (new File(absPath)).exists();
        }

        private static String dirExists(final String absPath) {
            final File f = new File(absPath);
            if (f.exists() && f.isDirectory())
                return f.getAbsolutePath();
            return null;
        }

        private static String readToString(final String filePath) {
            try {
                final File fp = new File(filePath);
                return readToString(fp);
            }
            catch (final IOException e) {
            }
            return "";
        }

        private static String readToString(final File file) throws IOException {
            if ((file == null) || !file.exists())
                return null;
            final FileReader reader = new FileReader(file);
            final StringBuilder out = new StringBuilder();
            final char[] buffer = new char[1024 * 4];
            int numRead = 0;
            while ((numRead = reader.read(buffer)) > -1) {
                out.append(String.valueOf(buffer, 0, numRead));
            }
            reader.close();
            return out.toString();
        }
    }
}

class LogSink implements Log.ILogSink {
	public LogSink() {
	}

	@Override
	public void log(int type, String file, String func, int line, String msg) {
		Lib.mLang.log(type, file, func, line, msg);
	}
}
