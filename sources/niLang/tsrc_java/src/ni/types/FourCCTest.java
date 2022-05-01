package ni.types;
import junit.framework.*;

public class FourCCTest extends TestCase {
    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
    }
    
    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    public void testBase() {
        assertEquals(0xFFFFFFFF,FourCC.build(255,255,255,255));
        assertEquals(0x00FFFFFF,FourCC.build(255,255,255,0));
        assertEquals(0xFFFFFF00,FourCC.build(0,255,255,255));
        assertEquals(0x0000FFFF,FourCC.build(255,255,0,0));
        assertEquals(0xFFFF0000,FourCC.build(0,0,255,255));
        final int foo = FourCC.build(255,119,153,0);
        assertEquals(0x009977FF,foo);
        assertEquals(255,FourCC.getA(foo));
        assertEquals(119,FourCC.getB(foo));
        assertEquals(153,FourCC.getC(foo));
        assertEquals(0,  FourCC.getD(foo));
        final int oof = FourCC.build(0,153,119,255);
        assertEquals(0xFF779900,oof);
        assertEquals(255,FourCC.getD(oof));
        assertEquals(119,FourCC.getC(oof));
        assertEquals(153,FourCC.getB(oof));
        assertEquals(0,  FourCC.getA(oof));
    }
};
 
