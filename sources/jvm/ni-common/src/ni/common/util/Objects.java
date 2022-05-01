package ni.common.util;

import java.util.HashMap;

import ni.common.lang3.ObjectUtils;

public class Objects extends ObjectUtils {

    private static final HashMap<Class<?>,Class<?>> WRAPPER_TYPES = getWrapperTypes();

    public static boolean isPrimitiveWrapperType(final Class<?> primitiveClassz, final Class<?> clazz)
    {
        assert primitiveClassz.isPrimitive();
        return WRAPPER_TYPES.get(primitiveClassz) == clazz;
    }

    private static HashMap<Class<?>,Class<?>> getWrapperTypes()
    {
        final HashMap<Class<?>,Class<?>> ret = new HashMap<Class<?>,Class<?>>();
        ret.put(boolean.class, Boolean.class);
        ret.put(char.class, Character.class);
        ret.put(byte.class, Byte.class);
        ret.put(short.class, Short.class);
        ret.put(int.class, Integer.class);
        ret.put(long.class, Long.class);
        ret.put(float.class, Float.class);
        ret.put(double.class, Double.class);
        ret.put(void.class, Void.class);
        return ret;
    }

}
