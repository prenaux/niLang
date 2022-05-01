package ni.niLang;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.Arrays;

import junit.framework.TestCase;

public class Test_IFile extends TestCase {
    ILang sys;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        sys = Lib.getLang();
        assertTrue(sys != null);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void test001_writeByteArray() {
        final byte[] ba = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
        final Buffer b1 = ByteBuffer.wrap(ba);
        assertFalse(b1.isDirect());
        assertTrue(b1.hasArray());

        final ByteBuffer b2 = ByteBuffer.allocateDirect(ba.length);
        b2.put(ba);
        assertTrue(b2.isDirect());
        assertFalse(b2.hasArray());

        final ByteBuffer b3 = ByteBuffer.allocate(ba.length);
        b3.put(ba);
        assertFalse(b3.isDirect());
        assertTrue(b3.hasArray());

        final IFile fp = sys.getRootFS().fileOpen(Lib.getTestOutputFilePath("test01_writeByteArray.raw"), EFileOpenMode.Write);
        fp.writeString("Hello");
        fp.writeString("\nByteArray.wrap: ");
        assertEquals(ba.length, fp.writeRaw(b1, b1.capacity()));
        fp.writeString("\nByteArray.allocateDirect: ");
        assertEquals(ba.length, fp.writeRaw(b2, b2.capacity()));
        fp.writeString("\nByteArray.allocate: ");
        assertEquals(ba.length, fp.writeRaw(b3, b3.capacity()));
        fp.invalidate();
    }

    public void test002_readByteArray() {
        // makes sure the test01_writeByteArray.raw file has been written
        test001_writeByteArray();

        final byte[] baExpectedData = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
        final byte[] ba = { 0, 0, 0, 0, 0, 0, 0, 0 };
        final ByteBuffer b1 = ByteBuffer.wrap(ba);
        assertFalse(b1.isDirect());
        assertTrue(b1.hasArray());

        final ByteBuffer b2 = ByteBuffer.allocateDirect(ba.length);
        assertTrue(b2.isDirect());
        assertFalse(b2.hasArray());

        final ByteBuffer b3 = ByteBuffer.allocate(ba.length);
        assertFalse(b3.isDirect());
        assertTrue(b3.hasArray());

        final IFile fp = sys.getRootFS().fileOpen(Lib.getTestOutputFilePath("test01_writeByteArray.raw"), EFileOpenMode.Read);
        // skip "Hello" "\nByteArray.wrap: "
        fp.seek(5 + 17);
        Arrays.fill(ba, (byte)0);
        assertEquals(ba.length, fp.readRaw(b1, b1.capacity()));
        assertTrue(Arrays.equals(ba, baExpectedData));
        // skip "\nByteArray.allocateDirect: "
        fp.seek(27);
        Arrays.fill(ba, (byte)0);
        assertEquals(ba.length, fp.readRaw(b2, b2.capacity()));
        b2.get(ba);
        assertTrue(Arrays.equals(ba, baExpectedData));
        // skip "\nByteArray.allocate: "
        fp.seek(21);
        Arrays.fill(ba, (byte)0);
        assertEquals(ba.length, fp.readRaw(b3, b3.capacity()));
        b3.get(ba);
        assertTrue(Arrays.equals(ba, baExpectedData));
        fp.invalidate();
    }
}
