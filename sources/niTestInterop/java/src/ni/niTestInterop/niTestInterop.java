package ni.niTestInterop;

import static ni.niLang.Lib.*;

public class niTestInterop {
    private niTestInterop() {
    }

    public static ITestInterop createTestInterop() {
        load();
        return ITestInterop.query(createInstance("niTestInterop.TestInterop"));
    }

    private static boolean isLoaded = false;
    public static void load() {
        if (isLoaded) return;
        loadLang();
        loadModule("niTestInterop");
        isLoaded = true;
    }
}
