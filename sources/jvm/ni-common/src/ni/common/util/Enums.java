package ni.common.util;

import ni.common.lang3.reflect.FieldUtils;
import ni.common.lang3.reflect.MethodUtils;

public class Enums {
    protected Enums() {
    }

    static public int getValue(final Class<?> classz, final String keyName) {
        try {
            return (Integer)FieldUtils.readDeclaredStaticField(classz, keyName);
        }
        catch (final Throwable e) {
        }

        // Failed try with a static function of the same name (Scala) ...
        try {
            return (Integer)MethodUtils.invokeStaticMethod(classz, keyName);
        }
        catch (final Throwable e) {

        }

        return -1;
    }
}
