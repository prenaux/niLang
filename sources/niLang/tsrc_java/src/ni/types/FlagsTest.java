package ni.types;

import junit.framework.*;

public class FlagsTest extends TestCase {
    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    private static int flagA = Flags.bit(0);
    private static int flagB = Flags.bit(1);
    private static int flagC = Flags.bit(2);
    private static int flagD = Flags.bit(3);
    @SuppressWarnings("unused")
    private static int flagAll = ~0;

    public void testBase() {
        final int a = Flags.on(0, flagA);
        assertEquals(true, Flags.is(a, flagA));
        assertEquals(false, Flags.is(a, flagB));
        assertEquals(true, Flags.isNot(a, flagB));

        final int b = Flags.onIf(a, flagB, true);
        assertEquals(true, Flags.is(b, flagB));
        assertEquals(false, Flags.is(b, flagC));
        assertEquals(true, Flags.isNot(b, flagD));
        assertEquals(b, Flags.onIf(b, flagC, false));

        final int c = Flags.on(Flags.offIf(b, flagB, true), flagC);
        assertEquals(true, Flags.is(c, flagA));
        assertEquals(true, Flags.is(c, flagC));
        assertEquals(true, Flags.isNot(c, flagB));
        assertEquals(c, Flags.offIf(c, flagC, false));

        assertEquals(true, Flags.testMask(c, flagD, flagA));
        assertEquals(false, Flags.testMask(c, flagA, flagA));
        assertEquals(false, Flags.testMask(c, flagD, flagB));
        assertEquals(false, Flags.testMask(c, 0, flagD));
        assertEquals(true, Flags.testMask(c, 0, flagA));
    }
};
