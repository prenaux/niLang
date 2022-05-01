package ni.types;
import junit.framework.*;

public class ULColorTest extends TestCase {
    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
    }
    
    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    public void testColorZero() {
        assertEquals(0,ULColor.build(0,0,0,0));
        assertEquals(0,ULColor.build(0,0,0));
        assertEquals(0,ULColor.buildf(0,0,0,0));
        assertEquals(0,ULColor.buildf(0,0,0));
    }
    public void testColorWhite() {
        assertEquals(0xFFFFFFFF,ULColor.build(255,255,255,255));
        assertEquals(0x00FFFFFF,ULColor.build(255,255,255));
        assertEquals(0xFFFFFFFF,ULColor.buildf(1,1,1,1));
        assertEquals(0x00FFFFFF,ULColor.buildf(1,1,1));
    }
    public void testColorBlueViolet() {
        final int c = ULColor.build(138,43,226,255);
        assertEquals(0xFF8A2BE2,ULColor.build(138,43,226,255));
        assertEquals(0x008A2BE2,ULColor.build(138,43,226));
        final int ir = ULColor.getR(c);
        final int ig = ULColor.getG(c);
        final int ib = ULColor.getB(c);
        final int ia = ULColor.getA(c);
        assertEquals(0xFF8A2BE2,ULColor.build(ir,ig,ib,ia));
        assertEquals(0x008A2BE2,ULColor.build(ir,ig,ib));
        final float fr = ULColor.getRf(c);
        final float fg = ULColor.getGf(c);
        final float fb = ULColor.getBf(c);
        final float fa = ULColor.getAf(c);
        assertEquals(0xFF8A2BE2,ULColor.buildf(fr,fg,fb,fa));
        assertEquals(0x008A2BE2,ULColor.buildf(fr,fg,fb));
    }
};
 
