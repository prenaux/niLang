package ni.niLang.util;

import java.util.Collection;
import java.util.Iterator;

import ni.niLang.ICollection;
import ni.niLang.IConcurrent;
import ni.niLang.IIterator;
import ni.niLang.IMessageHandler;
import ni.niLang.Lib;

import ni.common.util.Log;
import ni.types.MessageID;

public class Msg {
    static public IConcurrent _concurrent = Lib.getConcurrent();

    public static void send(final IMessageHandler handler, final int msg) {
        _concurrent.sendMessage(handler, msg, null, null);
    }

    public static void send(final IMessageHandler handler, final int msg, final Object a)
    {
        _concurrent.sendMessage(handler, msg, a, null);
    }

    public static void send(final IMessageHandler handler, final int msg, final Object a, final Object b)
    {
        _concurrent.sendMessage(handler, msg, a, b);
    }

    public static void send(final MessageHandler handler, final int msg, final Object a, final Object b)
    {
        send(handler.getHandler(), msg, a, b);
    }

    public static void send(final MessageHandler handler, final int msg, final Object a) {
        send(handler.getHandler(), msg, a, null);
    }

    public static void send(final MessageHandler handler, final int msg) {
        send(handler.getHandler(), msg, null, null);
    }

    public static void send(final ICollection coll, final int msg, final Object a, final Object b)
    {
        if (coll.isEmpty())
            return;
        final IIterator it = coll.iterator();
        for (Object o = it.value(); it.hasNext(); o = it.next()) {
            final IMessageHandler t = IMessageHandler.query(o);
            if (t != null) {
                send(t, msg, a, b);
            }
        }
    }

    public static void send(final ICollection coll, final int msg, final Object a) {
        send(coll, msg, a, null);
    }

    public static void send(final ICollection coll, final int msg) {
        send(coll, msg, null, null);
    }

    public static void send(final Collection<IMessageHandler> coll, final int msg, final Object a, final Object b)
    {
        final Iterator<IMessageHandler> it = coll.iterator();
        while (it.hasNext()) {
            final IMessageHandler t = it.next();
            if (t != null) {
                send(t, msg, a, b);
            }
        }
    }

    public static void send(final Collection<IMessageHandler> coll, final int msg, final Object a)
    {
        send(coll, msg, a, null);
    }

    public static void send(final Collection<IMessageHandler> coll, final int msg) {
        send(coll, msg, null, null);
    }
}
