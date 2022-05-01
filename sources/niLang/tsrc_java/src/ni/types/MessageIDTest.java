package ni.types;

import junit.framework.*;

public class MessageIDTest extends TestCase {
    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
    }

    @Override
    protected void tearDown() throws java.lang.Exception {
        super.tearDown();
    }

    @SuppressWarnings("unused")
    private static final int stuff1 = MessageID.build('K', 'E', 0, 0, 1);
    @SuppressWarnings("unused")
    private static final int stuff2 = MessageID.build('K', 'E', 'l', 'b', 2);
    @SuppressWarnings("unused")
    private static final int stuff3 = MessageID.build('K', 'E', 'k', 'a', 0);
    @SuppressWarnings("unused")
    private static final int stuff4 = MessageID.build('W', 'B', 'T', 'N', 'a');
    @SuppressWarnings("unused")
    private static final int stuff5 = MessageID.build('W', 'E', 'X', 'N', 'b');
    private static final int stuff6 = MessageID.build('W', 'S', 'B', 'N', 'c');
    private static final int maskABCD = MessageID.build('A', 'B', 'C', 'D', 0);

    public void testBase() {
        assertEquals(MessageID.build('W', 'S', 'B', 'N', 'd'),
                     (MessageID.build('W', 'S', 'B', 'N', 'c') + 1));

        assertTrue(MessageID.build('A', 0, 0, 0, 0) < MessageID.build('B', 0, 0, 0, 0));
        assertTrue(MessageID.build('C', 0, 0, 0, 0) > MessageID.build('B', 0, 0, 0, 0));
        assertTrue(MessageID.build('A', 0, 0, 0, 0) < MessageID.build('A', 'a', 0, 0, 0));
        assertTrue(MessageID.build('A', 0, 'a', 0, 0) < MessageID.build('A', 'a', 0, 0, 0));
        assertTrue(MessageID.build('A', 0, '9', '9', 255) < MessageID.build('A', 'a', 0, 0, 0));
    }

    public void testChars() {
        int id;

        assertEquals(0, MessageID.parse("????", 0));

        id = MessageID.parse("abcd", 54);
        assertEquals('a', MessageID.getCharA(id));
        assertEquals('b', MessageID.getCharB(id));
        assertEquals('c', MessageID.getCharC(id));
        assertEquals('d', MessageID.getCharD(id));
        assertEquals(54, MessageID.getByte(id));

        id = MessageID.parse("a2B4", 17);
        assertEquals('a', MessageID.getCharA(id));
        assertEquals('2', MessageID.getCharB(id));
        assertEquals('B', MessageID.getCharC(id));
        assertEquals('4', MessageID.getCharD(id));
        assertEquals(17, MessageID.getByte(id));

        id = MessageID.parse("_?A3", 89);
        assertEquals('_', MessageID.getCharA(id));
        assertEquals('-', MessageID.getCharB(id));
        assertEquals('A', MessageID.getCharC(id));
        assertEquals('3', MessageID.getCharD(id));
        assertEquals(89, MessageID.getByte(id));

        id = MessageID.parse("_?~@", 11);
        assertEquals('_', MessageID.getCharA(id));
        assertEquals('-', MessageID.getCharB(id));
        assertEquals('-', MessageID.getCharC(id));
        assertEquals('-', MessageID.getCharD(id));
        assertEquals(11, MessageID.getByte(id));

        id = MessageID.parse(("ABCD11"));
        assertEquals(MessageID.build('A', 'B', 'C', 'D', 0x11), id);
        assertEquals(("ABCD11"), MessageID.toString(id));

        id = MessageID.parse(("ABCD03"));
        assertEquals(MessageID.build('A', 'B', 'C', 'D', 3), id);
        assertEquals(("ABCD03"), MessageID.toString(id));

        id = MessageID.parse(("KE--03"));
        assertEquals(MessageID.build('K', 'E', 0, 0, 3), id);
        assertEquals(("KE--03"), MessageID.toString(id));

        id = MessageID.parse(("BLA"));
        assertEquals(0, id);
        assertEquals(("----00"), MessageID.toString(id));

        id = MessageID.parse(("ABCD11"));
        assertEquals(("A---00"), MessageID.toString(MessageID.maskA(id)));
        assertEquals(("AB--00"), MessageID.toString(MessageID.maskAB(id)));
        assertEquals(("ABC-00"), MessageID.toString(MessageID.maskABC(id)));
        assertEquals(("ABCD00"), MessageID.toString(MessageID.maskABCD(id)));

        assertTrue(maskABCD == MessageID.maskABCD(MessageID.parse(("ABCD00"))));
        assertTrue(maskABCD == MessageID.maskABCD(MessageID.parse(("ABCDFF"))));
        assertTrue(maskABCD == MessageID.maskABCD(MessageID.parse(("ABCD11"))));
        assertTrue(maskABCD == MessageID.maskABCD(MessageID.parse(("ABCD79"))));
        assertTrue(maskABCD == MessageID.maskABCD(MessageID.parse(("ABCD.p"))));
        assertTrue(maskABCD != MessageID.maskABCD(MessageID.parse(("aBCD.p"))));

        assertEquals(("WSBN.c"), MessageID.toString(stuff6));
        id = MessageID.parse(("WSBN.c"));
        assertEquals(stuff6, id);
        id = MessageID.parse(("WSBN63"));
        assertEquals(stuff6, id);

        assertTrue(MessageID.filterD(id, null) == false);
        assertTrue(MessageID.filterD(id, "") == false);
        assertTrue(MessageID.filterD(id, "!") == true);
        assertTrue(MessageID.filterD(id, "N") == true);
        assertTrue(MessageID.filterD(id, "abcdN") == true);
        assertTrue(MessageID.filterD(id, "n") == false);
        assertTrue(MessageID.filterD(id, "!N") == false);
        assertTrue(MessageID.filterD(id, "!X") == true);
    }
};
