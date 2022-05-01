package ni.niLang;

import junit.framework.TestCase;

public class Test_ILang extends TestCase {

    ILang sys;

    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
        sys = Lib.getLang();
        assertTrue(sys != null);
    }

    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    public void testBase() {
        System.out.println("getLangPath(niLang): " + Lib.getModulePath("niLang"));
    }
};
