import junit.framework.*;

public class AllTests {
    public static Test suite() {
        TestSuite suite = new TestSuite("All Unit Tests");
        suite.addTestSuite(ni.types.UUIDTest.class);
        suite.addTestSuite(ni.types.LibTest.class);
        suite.addTestSuite(ni.types.ULColorTest.class);
        suite.addTestSuite(ni.types.FourCCTest.class);
        suite.addTestSuite(ni.types.FlagsTest.class);
        suite.addTestSuite(ni.types.MessageIDTest.class);
        suite.addTestSuite(ni.niLang.Test_ILang.class);
        return suite;
    }
}
