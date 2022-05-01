package ni.common.util;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.HashMap;

import ni.types.Flags;
import ni.types.IUnknown;

import ni.common.lang3.reflect.MethodUtils;


/**
 * Utility methods to deal with Callback interfaces built using the JVM's
 * reflection mechanism.
 * 
 * @author Pierre Renaux
 */
public class ReflectSinks {
    /**
     * Setting this to true allow us to implement dynamic Sink dispatch from
     * private and anonymous classes, however setAccessible is a restricted
     * operation (it wont work under some security setup - Applet, etc...) so I
     * decided to disable it by default, the flag is there if we *really* need
     * it later on. - Pierre
     */
    public static final boolean USE_SET_ACCESSIBLE = false;

    /*
     * Cache for queryInterface
     */
    private static final HashMap<Class<?>,Method> queryMethods = Collections.HashMap();

    /*
     * Protected constructor, this is a utility class 
     */
    protected ReflectSinks() {
    }

    /**
     * Validates the class can be used to implement a Sink using Reflection.
     * 
     * @param c the class to validate
     * @throws RuntimeException if the class can't be used as "Reflection Sink"
     */
    public static void validate(final Class<?> c) {
        if (USE_SET_ACCESSIBLE) {
            /* Do nothing Method.setAccessible should enforce any method to be called... */
        }
        else {
            if (Flags.isNot(c.getModifiers(), Modifier.PUBLIC)) {
                throw new RuntimeException("Sink class '" + c.getName() + "' must be public.");
            }
        }
    }

    /**
     * Get an accessible version of the specified method.
     * 
     * @param m the method
     * @return an accessible version of the method
     * @throws RuntimeException if no accessible version of the method can be
     *             found (and thus the class can't be used as a
     *             "Reflection Sink")
     */
    public static Method getAccessibleMethod(Method m) {
        if (USE_SET_ACCESSIBLE) {
            m.setAccessible(true);
        }
        else {
            final String methName = m.getName();
            m = MethodUtils.getAccessibleMethod(m);
            if (m == null) {
                throw new RuntimeException("Receiver method '" + methName + "' is not accessible, make sure it is public.");
            }
            if (Flags.is(m.getModifiers(), Modifier.STATIC)) {
                throw new RuntimeException("Receiver method '" + methName + "' cannot be static.");
            }
        }
        return m;
    }

    /**
     * Uses reflection to query the specified interface.
     * 
     * @param intf the class of the interface to query
     * @param v the IUnknown object to query the interface on
     * @return an instance of the intf type if v implements it, else null
     */
    @SuppressWarnings("unchecked")
    public static <T extends IUnknown> T queryInterface(final Class<T> intf, final IUnknown v)
    {
        if (intf == IUnknown.class) {
            return (T)v;
        }
        try {
            Method queryMethod = queryMethods.get(intf);
            if (queryMethod == null) {
                queryMethod = intf.getMethod("query", IUnknown.class);
                if (queryMethod == null) {
                    throw new RuntimeException("Can't find a query method for '" + intf + ".");
                }
                queryMethods.put(intf, queryMethod);
            }
            return (T)queryMethod.invoke(null, v);
        }
        catch (final Exception e) {
            return null;
        }
    }

    /**
     * Converts the specified object to the specified type.
     * 
     * @param paramType the type to convert the object to
     * @param v the object to convert
     * @return the converted object or null if no satisfactory conversion can be
     *         found
     */
    @SuppressWarnings("unchecked")
    public static Object convertParamObject(final Class<?> paramType, final Object v)
    {
        if (v == null)
            return null;

        final Class<?> vClass = v.getClass();
        if (paramType == vClass) {
            return v;
        }
        else if (paramType == Object.class) {
            return v;
        }
        else if (paramType.isPrimitive()) {
            if (Objects.isPrimitiveWrapperType(paramType, vClass)) {
                return v;
            }
            else {
                // Object type is not a valid primitive type...
                return null;
            }
        }
        else if (IUnknown.class.isAssignableFrom(paramType) &&
                 IUnknown.class.isAssignableFrom(vClass))
        {
            return queryInterface((Class<? extends IUnknown>)paramType, (IUnknown)v);
        }
        else if (!paramType.isAssignableFrom(vClass)) {
            // Types not compatible, return null
            return null;
        }
        return v;
    }
}
