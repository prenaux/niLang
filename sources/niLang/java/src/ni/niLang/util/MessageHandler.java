package ni.niLang.util;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;

import ni.niLang.IMessageHandler;
import ni.niLang.IMessageHandlerImpl;
import ni.niLang.Lib;
import ni.common.util.Collections;
import ni.common.util.Enums;
import ni.common.util.Log;
import ni.common.util.ReflectSinks;
import ni.common.util.Strings;

/**
 * Message Target that implements {@link IMessageTargetImpl} using Reflection.
 * <p>
 * Message Method: [<b>void</b>|<b>boolean</b>]
 * <b>on</b>[<i>ThisClassMessageName</i>|<i>OtherClassMessageName</i>]( <br>
 * <b>param1</b>: Extra1 or Extra2 or Sender</li> <br>
 * <b>param2</b>: Extra2 or Extra1 or Sender</li> <br>
 * <b>param3</b>: Sender</li> <br>
 * )<br>
 * If the return type is void, false is returned by the sink.
 *
 * @author Pierre Renaux
 */
public class MessageHandler implements IMessageHandlerImpl {
    private Map<Integer,Method> jumpTable;
    private final Object[] invokeParams1 = { null };
    private final Object[] invokeParams2 = { null, null };
    private final IMessageHandler target;
    private final long threadId;

    public MessageHandler() {
        this(null, null);
    }

    public MessageHandler(final Class<?>... otherClass) {
        this(null, otherClass);
    }

    public MessageHandler(Class<?> thisClass, final Class<?>[] otherClass) {
        threadId = Lib.getConcurrent().getCurrentThreadID();

        if (thisClass == null) {
            thisClass = this.getClass();
        }
        ReflectSinks.validate(thisClass);

        final Method[] meths = thisClass.getDeclaredMethods();
        for (Method m : meths) {
            if (m.getName().equals("onMessageSink")) {
                // That's the name of the method in IMessageSinkImpl avoid a pointless warning
                continue;
            }
            if (m.getName().startsWith("on")) {
                m = ReflectSinks.getAccessibleMethod(m);
                final String keyName = Strings.substringAfter(m.getName(), "on");
                int msgId = Enums.getValue(thisClass, "MSG_" + keyName);
                if ((msgId == -1) && (otherClass != null)) {
                    for (final Class<?> cl : otherClass) {
                        msgId = Enums.getValue(cl, "MSG_" + keyName);
                        if (msgId != -1)
                            break;
                    }
                }
                if (msgId != -1) {
                    if (jumpTable == null) {
                        jumpTable = Collections.HashMap();
                    }
                    jumpTable.put(msgId, m);
                }
                else {
                    Log.w("Receiver method '" + m.getName() + "' cannot find a message to bind to.");
                }
            }
        }

        target = IMessageHandler.impl(this);
    }

    @Override
    public void handleMessage(final int msg, final Object avarA, final Object avarB) {
        /*
         * Call Message Method
         */
        if (jumpTable != null) {
            final Method m = jumpTable.get(msg);
            if (m != null) {
                try {
                    final Class<?>[] params = m.getParameterTypes();
                    switch (params.length) {
                    case 0:
                        m.invoke(this);
                        break;
                    case 1:
                        invokeParams1[0] = getParam0(params[0], avarA, avarB);
                        m.invoke(this, invokeParams1);
                        break;
					default:
                    case 2:
                        invokeParams2[0] = getParam0(params[0], avarA, avarB);
                        invokeParams2[1] = getParam1(params[1], avarA, avarB);
                        m.invoke(this, invokeParams2);
                        break;
                    }
                }
                catch (final Exception e) {
                    throw new RuntimeException(e);
                }
            }
        }
    }

    // VarA or VarB
    private Object getParam0(final Class<?> paramType, final Object avarA, final Object avarB) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;
        r = ReflectSinks.convertParamObject(paramType, avarA);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, avarB);
        if (r != null)
            return r;
        return null;
    }

    // VarA or VarB
    private Object getParam1(final Class<?> paramType, final Object avarA, final Object avarB) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;
        r = ReflectSinks.convertParamObject(paramType, avarB);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, avarA);
        if (r != null)
            return r;
        return null;
    }

    @Override
    public long getThreadID() {
        return threadId;
    }

    public IMessageHandler getHandler() {
        return target;
    }
}
