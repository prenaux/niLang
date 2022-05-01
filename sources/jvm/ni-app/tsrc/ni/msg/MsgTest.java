package ni.msg;

import junit.framework.TestCase;
import ni.niLang.IFile;
import ni.niLang.IMessageQueue;
import ni.niLang.Lib;
import ni.niLang.util.MessageHandler;
import ni.niLang.util.Msg;
import ni.types.MessageID;
import ni.common.util.Log;
import ni.common.util.ReflectSinks;

public class MsgTest extends TestCase {

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Lib.getLang();
    }

    MessageHandler foo;
    MessageHandler bar;

    private static final int SEND_COUNT = 100000;

    public static final int MSG_Ping = MessageID.buildLocal('p');
    public static final int MSG_PingInFile = MessageID.buildLocal('f');

    public class Foo extends MessageHandler {
        public Foo() {
            super(MsgTest.class);
        }

        public void onPing() {
            Msg.send(bar, MSG_Ping);
        }
    }

    public class PingDirect extends MessageHandler {
        public int pingCount = 0;

        public PingDirect() {
            super(MsgTest.class);
        }

        @Override
        public void handleMessage(final int msg, final Object varA, final Object varB) {
            if (msg == MSG_Ping) {
                ++pingCount;
            }
            else if (msg == MSG_PingInFile) {
                ++pingCount;
                final IFile pingFile = IFile.query(varA);
                pingFile.writeLE32(pingCount);
            }
        }
    }

    public class PingReflect extends MessageHandler {
        public int pingCount = 0;

        public PingReflect() {
            super(MsgTest.class);
        }

        public void onPing() {
            //Log.i("Bar::onPing");
            ++pingCount;
        }

        public void onPingInFile(final IFile pingFile) {
            ++pingCount;
            pingFile.writeLE32(pingCount);
        }
    }

    public void testMessageHandler() {
    	final IMessageQueue mq = Lib.getConcurrent().createMessageQueue(Lib.getConcurrent().getCurrentThreadID(), 0xFFFF);
    	
        Log.i("USE_SET_ACCESSIBLE: " + ReflectSinks.USE_SET_ACCESSIBLE);
        if (ReflectSinks.USE_SET_ACCESSIBLE) {
            foo = new MessageHandler(MsgTest.class) {
                @SuppressWarnings("unused")
                public void onPing() {
                    Msg.send(bar, MSG_Ping);
                }
            };
        }
        else {
            foo = new Foo();
        }
        bar = new PingReflect();
        Msg.send(foo, MSG_Ping);
        while (mq.pollAndDispatch()) {}
        assertEquals(1, ((PingReflect)bar).pingCount);
    }

    public void testPingDirectSendMessage() {
        final MessageHandler ping = new PingDirect();
        for (int i = 0; i < SEND_COUNT; ++i) {
            Msg.send(ping, MSG_Ping);
        }
        assertEquals(SEND_COUNT, ((PingDirect)ping).pingCount);
    }

    public void testPingInFileDirectSendMessage() {
        final IFile pingFile = Lib.getLang().createFileDynamicMemory(1000, "");
        final MessageHandler ping = new PingDirect();
        for (int i = 0; i < SEND_COUNT; ++i) {
            Msg.send(ping, MSG_PingInFile, pingFile);
        }
        assertEquals(SEND_COUNT, ((PingDirect)ping).pingCount);
        assertEquals(SEND_COUNT, pingFile.getSize() / 4);
    }

    public void testPingReflectSendMessage() {
        final MessageHandler ping = new PingReflect();
        for (int i = 0; i < SEND_COUNT; ++i) {
            Msg.send(ping, MSG_Ping);
        }
        assertEquals(SEND_COUNT, ((PingReflect)ping).pingCount);
    }

    public void testPingInFileReflectSendMessage() {
        final IFile pingFile = Lib.getLang().createFileDynamicMemory(1000, "");
        final MessageHandler ping = new PingReflect();
        for (int i = 0; i < SEND_COUNT; ++i) {
            Msg.send(ping, MSG_PingInFile, pingFile);
        }
        assertEquals(SEND_COUNT, ((PingReflect)ping).pingCount);
        assertEquals(SEND_COUNT, pingFile.getSize() / 4);
    }

    public void testPingDirectPostMessage_OneDispatch() {
    	final IMessageQueue mq = Lib.getConcurrent().createMessageQueue(Lib.getConcurrent().getCurrentThreadID(), 0xFFFF);
        final MessageHandler ping = new PingDirect();
        for (int i = 0; i < SEND_COUNT; ++i) {
            Msg.send(ping, MSG_Ping);
        }
        while (mq.pollAndDispatch()) {}
        assertEquals(SEND_COUNT, ((PingDirect)ping).pingCount);
    }
}
