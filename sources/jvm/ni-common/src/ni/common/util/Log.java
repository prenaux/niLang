package ni.common.util;

public class Log {
    protected Log() {
    }

    private static final String THIS_CLASSNAME = Log.class.getName();

    public static boolean ENABLED = true;
    public static boolean PRINT_STACKS = true;

    public final static int Raw = (1 << 0);
    public final static int RAW = (1 << 0);
    public final static int Debug = (1 << 1);
    public final static int DEBUG = (1 << 1);
    public final static int Info = (1 << 2);
    public final static int INFO = (1 << 2);
    public final static int Warning = (1 << 3);
    public final static int WARNING = (1 << 3);
    public final static int Error = (1 << 4);
    public final static int ERROR = (1 << 4);

    public interface ILogSink {
        public void log(int type, String file, String func, int line, String msg);
    };

    public static ILogSink mLogSink = null;

    public static void formatThrowable(final StringBuilder sb, final String msg, final Throwable e)
    {
        if (msg != null) {
            sb.append(msg);
            if (!msg.endsWith("\n")) {
                sb.append("\n");
            }
        }
        sb.append("Exception: ");
        sb.append(e.getMessage());
        sb.append("\n");
        if (PRINT_STACKS) {
            sb.append(Strings.getStackTrace(e));
        }
    }

    private static StringBuilder mBuilder = null;

    synchronized public static String getThrowableMessage(final String msg, final Throwable e)
    {
        if (mBuilder == null) {
            mBuilder = new StringBuilder();
        }
        else {
            mBuilder.setLength(0);
        }
        formatThrowable(mBuilder, msg, e);
        return mBuilder.toString();
    }

    private static int findFirstStackTraceElementAfterClass(final StackTraceElement[] s)
    {
        if (s == null)
            return -1;
        boolean foundClass = false;
        for (int i = 0; i < s.length; ++i) {
            if (!s[i].isNativeMethod() && s[i].getClassName().equals(THIS_CLASSNAME)) {
                foundClass = true;
            }
            else if (foundClass) {
                return i;
            }
        }
        return -1;
    }

    static private final String NO_STACK = "<nostack>";

    public static void doLog(final int type, final String msg) {
    	if (mLogSink == null) {
    		System.out.println("JAVA/" + msg);
    		return;
    	}

        final StackTraceElement[] stackTraceElements = Thread.currentThread().getStackTrace();
        final int stackIndex = findFirstStackTraceElementAfterClass(stackTraceElements);
        if (stackIndex < 0) {
            mLogSink.log(type, NO_STACK, NO_STACK, -1, msg);
        }
        else {
            mLogSink.log(type,
                         stackTraceElements[stackIndex].getFileName(),
                         stackTraceElements[stackIndex].getMethodName(),
                         stackTraceElements[stackIndex].getLineNumber(),
                         msg);
        }
    }

    public static void i(final String msg) {
        if (!ENABLED)
            return;
        doLog(Info, msg);
    }

    public static void i(final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Info, getThrowableMessage(null, e));
    }

    public static void i(final String msg, final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Info, getThrowableMessage(msg, e));
    }

    public static void w(final String msg) {
        if (!ENABLED)
            return;
        doLog(Warning, msg);
    }

    public static void w(final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Warning, getThrowableMessage(null, e));
    }

    public static void w(final String msg, final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Warning, getThrowableMessage(msg, e));
    }

    public static void e(final String msg) {
        if (!ENABLED)
            return;
        doLog(Error, msg);
    }

    public static void e(final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Error, getThrowableMessage(null, e));
    }

    public static void e(final String msg, final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Error, getThrowableMessage(msg, e));
    }

    public static void d(final String msg) {
        if (!ENABLED)
            return;
        doLog(Debug, msg);
    }

    public static void d(final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Debug, getThrowableMessage(null, e));
    }

    public static void d(final String msg, final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Debug, getThrowableMessage(msg, e));
    }

    public static void v(final String msg) {
        if (!ENABLED)
            return;
        doLog(Debug, msg);
    }

    public static void v(final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Debug, getThrowableMessage(null, e));
    }

    public static void v(final String msg, final Throwable e) {
        if (!ENABLED)
            return;
        doLog(Debug, getThrowableMessage(msg, e));
    }
};
