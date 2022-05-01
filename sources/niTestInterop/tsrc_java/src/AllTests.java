import junit.framework.*;

public class AllTests {
    public static Test suite() {
        TestSuite suite = new TestSuite("All Unit Tests");
        suite.addTestSuite(ni.niTestInterop.Test_ITestInterop.class);
        return suite;
    }
}
