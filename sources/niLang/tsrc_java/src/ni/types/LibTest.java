package ni.types;

import ni.niLang.Lib;
import junit.framework.*;

public class LibTest extends TestCase {
    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    public void testBase() {
        System.out.println("getModulePath(niLang): " + Lib.getModulePath("niLang"));
    }
};
