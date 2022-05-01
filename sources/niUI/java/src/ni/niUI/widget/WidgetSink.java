package ni.niUI.widget;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;

import ni.niLang.IHString;
import ni.niLang.Lib;
import ni.niUI.EUIMessage;
import ni.niUI.IWidget;
import ni.niUI.IWidgetCommand;
import ni.niUI.IWidgetSinkImpl;
import ni.common.util.Classz;
import ni.common.util.Collections;
import ni.common.util.Enums;
import ni.common.util.Log;
import ni.common.util.ReflectSinks;
import ni.common.util.Strings;

/**
 * Default Widget Sink that implements {@link IWidgetSinkImpl} using Reflection.
 * <p>
 * Message Method: [<b>void</b>|<b>boolean</b>]
 * <b>on</b>[<i>ThisClassMessageName </i>|<i>OtherClassMessageName</i>|<i>
 * {@link EUIMessage}</i>]( <br>
 * <b>param0</b>: Widget or Extra1 or Extra2 <br>
 * <b>param1</b>: Extra1 or Extra2</li> <br>
 * <b>param2</b>: Extra2 or Extra1</li> <br>
 * )<br>
 * If the return type is void, false is returned by the sink.
 * <p>
 * Command Method: [<b>void</b>|<b>boolean</b>] [<i>ID_WIDGET</i>]( <br>
 * <b>param0</b>: IWidget w, Command ID</li> <br>
 * <b>param1</b>: IWidgetCommand, Command ID, Extra1, Extra2, Sender</li> <br>
 * <b>param2</b>: Extra1, Extra2, Sender</li> <br>
 * <b>param3</b>: Extra2, Extra1, Sender</li> <br>
 * <b>param4</b>: Sender</li> <br/>
 * )<br>
 * If the return type is void, false is returned by the sink.
 * 
 * @author Pierre Renaux
 */
public class WidgetSink implements IWidgetSinkImpl {
    Map<Integer,Method> jumpTable;
    
    class MethodID {
    	final Method method;
    	final IHString id;
    	MethodID(Method aMethod, IHString aID) {
    		method = aMethod;
    		id = aID;
    	}
    }
    
    Map<Long,MethodID> commandTable;
    
    Object[] invokeParams1 = { null };
    Object[] invokeParams2 = { null, null };
    Object[] invokeParams3 = { null, null, null };
    Object[] invokeParams4 = { null, null, null, null, null };
    Object[] invokeParams5 = { null, null, null, null, null, null };

    public WidgetSink() {
        this(null, (Class<?>[])null);
    }

    public WidgetSink(Class<?> thisClass, final Class<?>[] otherClass) {
        if (thisClass == null) {
            thisClass = this.getClass();
        }
        ReflectSinks.validate(thisClass);

        final Method[] meths = thisClass.getDeclaredMethods();

        for (Method m : meths) {
            final String methName = m.getName();
            if (methName.startsWith("on")) {
                m = ReflectSinks.getAccessibleMethod(m);
                final String keyName = Strings.substringAfter(m.getName(), "on");
                int msgId = Enums.getValue(thisClass, "MSG_" + keyName);
                if (msgId == -1) {
                    if (otherClass != null) {
                        for (final Class<?> cl : otherClass) {
                            msgId = Enums.getValue(cl, "MSG_" + keyName);
                            if (msgId != -1)
                                break;
                        }
                    }
                    if (msgId == -1) {
                        msgId = Enums.getValue(EUIMessage.class, keyName);
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
            else if (m.getName().startsWith("ID_")) {
                m = ReflectSinks.getAccessibleMethod(m);
                if (commandTable == null) {
                    commandTable = Collections.HashMap();
                }
                final IHString id = Lib.hstr(m.getName());
                commandTable.put(id.getObjPtr(), new MethodID(m,id));
            }
        }
    }

    @Override
    public boolean onWidgetSink(final IWidget w, final int msg, final Object a, final Object b)
    {
        boolean r = false;
        /*
         * Call Message Method
         */
        if (jumpTable != null) {
            final Method m = jumpTable.get(msg);
            if (m != null) {
                try {
                    Object ir;
                    final Class<?>[] params = m.getParameterTypes();
                    switch (params.length) {
                    case 0:
                        ir = m.invoke(this);
                        break;
                    case 1:
                        invokeParams1[0] = getParam0(params[0], w, a, b);
                        ir = m.invoke(this, invokeParams1);
                        break;
                    case 2:
                        invokeParams2[0] = getParam0(params[0], w, a, b);
                        invokeParams2[1] = getParam1(params[1], w, a, b);
                        ir = m.invoke(this, invokeParams2);
                        break;
                    default:
                        invokeParams3[0] = getParam0(params[0], w, a, b);
                        invokeParams3[1] = getParam1(params[1], w, a, b);
                        invokeParams3[2] = getParam2(params[2], w, a, b);
                        ir = m.invoke(this, invokeParams3);
                        break;
                    }
                    if (ir instanceof Boolean) {
                        r = ((Boolean)ir).booleanValue();
                    }
                    else if (ir == null) {
                        r = false;
                    }
                }
                catch (final Exception e) {
                    throw new RuntimeException("Invoke error : " + m.getName(), e);
                }
            }
        }

        /*
         * Call Command Method
         */
        if ((r == false) && (msg == EUIMessage.Command) && (commandTable != null)) {
            Method m = null;
            final IWidgetCommand cmd = IWidgetCommand.query(a);
            try {
                final IHString id = cmd.getSender().getID();
                MethodID methId = commandTable.get(id.getObjPtr());
                if (methId != null)
                	m = methId.method;
                if (m != null) {
                    Object ir;
                    final Class<?>[] params = m.getParameterTypes();
                    switch (params.length) {
                    case 0:
                        ir = m.invoke(this);
                        break;
                    case 1:
                        invokeParams1[0] = getCmdParam0(params[0], w, cmd);
                        ir = m.invoke(this, invokeParams1);
                        break;
                    case 2:
                        invokeParams2[0] = getCmdParam0(params[0], w, cmd);
                        invokeParams2[1] = getCmdParam1(params[1], w, cmd);
                        ir = m.invoke(this, invokeParams2);
                        break;
                    case 3:
                        invokeParams3[0] = getCmdParam0(params[0], w, cmd);
                        invokeParams3[1] = getCmdParam1(params[1], w, cmd);
                        invokeParams3[2] = getCmdParam2(params[2], w, cmd);
                        ir = m.invoke(this, invokeParams3);
                        break;
                    case 4:
                        invokeParams4[0] = getCmdParam0(params[0], w, cmd);
                        invokeParams4[1] = getCmdParam1(params[1], w, cmd);
                        invokeParams4[2] = getCmdParam2(params[2], w, cmd);
                        invokeParams4[3] = getCmdParam3(params[3], w, cmd);
                        ir = m.invoke(this, invokeParams4);
                        break;
                    default:
                        invokeParams5[0] = getCmdParam0(params[0], w, cmd);
                        invokeParams5[1] = getCmdParam1(params[1], w, cmd);
                        invokeParams5[2] = getCmdParam2(params[2], w, cmd);
                        invokeParams5[3] = getCmdParam3(params[3], w, cmd);
                        invokeParams5[4] = getCmdParam4(params[4], w, cmd);
                        ir = m.invoke(this, invokeParams5);
                        break;
                    }
                    if (ir instanceof Boolean) {
                        r = ((Boolean)ir).booleanValue();
                    }
                    else if (ir == null) {
                        r = false;
                    }
                }
            }
            catch (final Exception e) {
                throw new RuntimeException("Invoke error : " + ((m != null) ? m.getName() : cmd), e);
            }
        }

        return r;
    }

    private Object getParam0(final Class<?> paramType, final IWidget w, final Object a, final Object b) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;
        r = ReflectSinks.convertParamObject(paramType, w);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, a);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, b);
        if (r != null)
            return r;
        return null;
    }

    private Object getParam1(final Class<?> paramType, final IWidget w, final Object a, final Object b) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;
        r = ReflectSinks.convertParamObject(paramType, a);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, b);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, w);
        if (r != null)
            return r;
        return null;
    }

    private Object getParam2(final Class<?> paramType, final IWidget w, final Object a, final Object b) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;
        r = ReflectSinks.convertParamObject(paramType, b);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, a);
        if (r != null)
            return r;
        r = ReflectSinks.convertParamObject(paramType, w);
        if (r != null)
            return r;
        return null;
    }

    // cmdParam0: IWidget w or Command ID
    private Object getCmdParam0(final Class<?> paramType, final IWidget w, final IWidgetCommand cmd) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        // IWidget ?
        if (paramType == IWidget.class)
            return w;

        // IWidgetCommand ?
        if (paramType == IWidgetCommand.class)
            return cmd;

        // Command ID ?
        if (Classz.isAssignable(Integer.class, paramType))
            return cmd.getID();

        return null;
    }

    // param1: IWidgetCommand or Command ID or Extra1 or Extra2 or Sender
    private Object getCmdParam1(final Class<?> paramType, final IWidget w, final IWidgetCommand cmd) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;

        // IWidgetCommand ?
        if (paramType == IWidgetCommand.class)
            return cmd;

        // Command ID ?
        if (Classz.isAssignable(Integer.class, paramType))
            return cmd.getID();

        // Extra 1 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra1());
        if (r != null)
            return r;

        // Extra 2 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra2());
        if (r != null)
            return r;

        // Sender ?
        if (paramType == IWidget.class)
            return cmd.getSender();

        return null;
    }

    // param2: Extra1 or Extra2 or Sender
    private Object getCmdParam2(final Class<?> paramType, final IWidget w, final IWidgetCommand cmd) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;

        // Extra 1 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra1());
        if (r != null)
            return r;

        // Extra 2 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra2());
        if (r != null)
            return r;

        // Sender ?
        if (paramType == IWidget.class)
            return cmd.getSender();

        return null;
    }

    // param3: Extra2 or Extra1 or Sender
    private Object getCmdParam3(final Class<?> paramType, final IWidget w, final IWidgetCommand cmd) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        Object r;

        // Extra 2 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra2());
        if (r != null)
            return r;

        // Extra 1 ?
        r = ReflectSinks.convertParamObject(paramType, cmd.getExtra1());
        if (r != null)
            return r;

        // Sender ?
        if (paramType == IWidget.class)
            return cmd.getSender();

        return null;
    }

    // param4: Sender
    private Object getCmdParam4(final Class<?> paramType, final IWidget w, final IWidgetCommand cmd) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
    {
        // Sender ?
        if (paramType == IWidget.class)
            return cmd.getSender();

        return null;
    }
}
