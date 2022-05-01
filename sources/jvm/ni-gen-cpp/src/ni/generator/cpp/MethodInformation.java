package ni.generator.cpp;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.ByteBuffer;
import java.util.logging.Level;
import java.util.logging.Logger;

import ni.types.RefCountedPointer;
import ni.types.annotations.Allocator;
import ni.types.annotations.ArrayAllocator;
import ni.types.annotations.Function;
import ni.types.annotations.ImplWrapper;
import ni.types.annotations.MemberGetter;
import ni.types.annotations.MemberSetter;
import ni.types.annotations.Name;
import ni.types.annotations.Namespace;
import ni.types.annotations.NoOffset;
import ni.types.annotations.Ret;
import ni.types.annotations.Static;
import ni.types.annotations.ValueGetter;
import ni.types.annotations.ValueSetter;

public class MethodInformation {
    public Method method;
    public Annotation[] annotations;
    public int modifiers;
    public Class<?> returnType;
    public String retPrefix, retSuffix;
    public String name, memberName, memberNameSuffix;
    public String staticName, staticCall;
    public Class<?>[] parameterTypes;
    public Annotation[][] parameterAnnotations;
    public boolean overloaded, noOffset, deallocator, allocator, arrayAllocator,
            bufferGetter, valueGetter, valueSetter, memberGetter, memberSetter,
            refCounted;
    public Method pairedMethod;

    public static MethodInformation getMethodInformation(final Logger logger, final Method method)
    {
        if (!Modifier.isNative(method.getModifiers())) {
            return null;
        }
        final MethodInformation info = new MethodInformation();
        info.method = method;
        info.annotations = method.getAnnotations();
        info.modifiers = method.getModifiers();
        info.returnType = method.getReturnType();
        info.name = method.getName();
        info.memberName = info.name;
        info.refCounted = RefCountedPointer.class.isAssignableFrom(method.getDeclaringClass());
        info.parameterTypes = method.getParameterTypes();
        info.parameterAnnotations = method.getParameterAnnotations();

        Name memberName = method.getAnnotation(Name.class);
        if (memberName != null) {
            info.memberName = memberName.value();
            info.memberNameSuffix = memberName.suffix();
        }

        final Static staticName = method.getAnnotation(Static.class);
        if (staticName != null) {
            info.staticName = staticName.name();
            info.staticCall = staticName.call();
        }
        else if (info.parameterTypes.length == 1) {
            final Class<?> pc = info.parameterTypes[0];
            if (pc.getAnnotation(ImplWrapper.class) != null) {
                try {
                    final String namespaceName = pc.getAnnotation(Namespace.class).value();
                    final String interfaceName = pc.getAnnotation(Name.class).value();
                    info.staticCall = "NewJNIWrapper_" + namespaceName + "_" + interfaceName + "(e,p0)";
                }
                catch (final Exception e) {
                    info.staticCall = "<invalid impl wrapper type>";
                }
            }
        }

        final Ret retName = method.getAnnotation(Ret.class);
        if (retName != null) {
            info.retPrefix = retName.prefix();
            info.retSuffix = retName.suffix();
        }

        final Class<?> behavior = getBehavior(logger, method);
        boolean canBeGetter = info.returnType != void.class;
        boolean canBeSetter = true;
        for (int j = 0; j < info.parameterTypes.length; j++) {
            if ((info.parameterTypes[j] != int.class) && (info.parameterTypes[j] != long.class)) {
                canBeGetter = false;
                if (j < info.parameterTypes.length - 1) {
                    canBeSetter = false;
                }
            }
        }
        final boolean canBeAllocator = !Modifier.isStatic(info.modifiers) &&
                                       (info.returnType == void.class);
        final boolean canBeArrayAllocator = canBeAllocator && (info.parameterTypes.length == 1) &&
                                            ((info.parameterTypes[0] == int.class) || (info.parameterTypes[0] == long.class));

        boolean valueGetter = false;
        boolean valueSetter = false;
        boolean memberGetter = false;
        boolean memberSetter = false;
        Method pairedMethod = null;
        final Method[] methods = Builder.getSortedDeclaredMethods(method.getDeclaringClass());
        for (int i = 0; i < methods.length; i++) {
            final Method method2 = methods[i];
            final int modifiers2 = method2.getModifiers();
            final Class<?> returnType2 = method2.getReturnType();
            final String methodName2 = method2.getName();
            final Class<?>[] parameterTypes2 = method2.getParameterTypes();

            if (method.equals(method2) || !Modifier.isNative(modifiers2)) {
                continue;
            }

            if ("get".equals(info.name) && "put".equals(methodName2) && (info.parameterTypes.length == 0) &&
                (parameterTypes2.length == 1) && (parameterTypes2[0] == info.returnType) && canBeGetter) {
                valueGetter = true;
                pairedMethod = method2;
            }
            else if ("put".equals(info.name) && "get".equals(methodName2) && (info.parameterTypes.length == 1) &&
                       (parameterTypes2.length == 0) && (info.parameterTypes[0] == returnType2) && canBeSetter) {
                valueSetter = true;
                pairedMethod = method2;
            }
            else if (methodName2.equals(info.name)) {
                info.overloaded = true;

                boolean sameIndexParameters = true;
                for (int j = 0; (j < info.parameterTypes.length) && (j < parameterTypes2.length); j++) {
                    if (info.parameterTypes[j] != parameterTypes2[j]) {
                        sameIndexParameters = false;
                    }
                }
                if (sameIndexParameters && (parameterTypes2.length - 1 == info.parameterTypes.length) &&
                    (info.returnType == parameterTypes2[parameterTypes2.length - 1]) && canBeGetter) {
                    memberGetter = true;
                    pairedMethod = method2;
                }
                else if (sameIndexParameters && (info.parameterTypes.length - 1 == parameterTypes2.length) &&
                           (returnType2 == info.parameterTypes[info.parameterTypes.length - 1]) && canBeSetter) {
                    memberSetter = true;
                    pairedMethod = method2;
                }
            }
        }

        if (canBeGetter && (behavior == ValueGetter.class)) {
            info.valueGetter = true;
        }
        else if (canBeSetter && (behavior == ValueSetter.class)) {
            info.valueSetter = true;
        }
        else if (canBeGetter && (behavior == MemberGetter.class)) {
            info.memberGetter = true;
        }
        else if (canBeSetter && (behavior == MemberSetter.class)) {
            info.memberSetter = true;
        }
        else if (canBeAllocator && (behavior == Allocator.class)) {
            info.allocator = true;
        }
        else if (canBeArrayAllocator && (behavior == ArrayAllocator.class)) {
            info.allocator = info.arrayAllocator = true;
        }
        else if (behavior == null) {
            // try to guess the behavior of the method
            if ((info.returnType == void.class) && "deallocate".equals(info.name) &&
                !Modifier.isStatic(info.modifiers) && (info.parameterTypes.length == 2) &&
                (info.parameterTypes[0] == long.class) && (info.parameterTypes[1] == long.class)) {
                info.deallocator = true;
            }
            else if (canBeAllocator && "allocate".equals(info.name)) {
                info.allocator = true;
            }
            else if (canBeArrayAllocator && "allocateArray".equals(info.name)) {
                info.allocator = info.arrayAllocator = true;
            }
            else if (info.returnType.isAssignableFrom(ByteBuffer.class) && "asDirectBuffer".equals(info.name) &&
                       !Modifier.isStatic(info.modifiers) && (info.parameterTypes.length == 1) &&
                       ((info.parameterTypes[0] == int.class) || (info.parameterTypes[0] == long.class))) {
                info.bufferGetter = true;
            }
            else if (valueGetter) {
                info.valueGetter = true;
                info.pairedMethod = pairedMethod;
            }
            else if (valueSetter) {
                info.valueSetter = true;
                info.pairedMethod = pairedMethod;
            }
            else if (memberGetter) {
                info.memberGetter = true;
                info.pairedMethod = pairedMethod;
            }
            else if (memberSetter) {
                info.memberSetter = true;
                info.pairedMethod = pairedMethod;
            }
        }
        else {
            logger.log(Level.WARNING, "Method \"" + method + "\" cannot behave like a \"" +
                                      behavior + "\". No code will be generated.");
            return null;
        }

        if ((memberName == null) && (info.pairedMethod != null)) {
            memberName = info.pairedMethod.getAnnotation(Name.class);
            if (memberName != null) {
                info.memberName = memberName.value();
                info.memberNameSuffix = memberName.suffix();
            }
        }
        info.noOffset = method.isAnnotationPresent(NoOffset.class);
        if (!info.noOffset && (info.pairedMethod != null)) {
            info.noOffset = info.pairedMethod.isAnnotationPresent(NoOffset.class);
        }
        return info;
    }

    public static Class<? extends Annotation> getBehavior(final Logger logger, final Method method)
    {
        final Annotation[] annotations = method.getAnnotations();
        Annotation behaviorAnnotation = null;
        for (final Annotation a : annotations) {
            if ((a instanceof Function) || (a instanceof Allocator) || (a instanceof ArrayAllocator) ||
                (a instanceof ValueSetter) || (a instanceof ValueGetter) ||
                (a instanceof MemberGetter) || (a instanceof MemberSetter)) {
                if (behaviorAnnotation != null) {
                    logger.log(Level.WARNING, "Behavior annotation \"" + behaviorAnnotation +
                                              "\" already found. Ignoring superfluous annotation \"" + a + "\".");
                }
                else {
                    behaviorAnnotation = a;
                }
            }
        }
        return behaviorAnnotation == null ? null : behaviorAnnotation.annotationType();
    }

    public static String getExtendedPrimitiveTypeName(final Class<?> type) {
        Object o = "<__jniTypeName:error>";
        try {
            final Method m = type.getDeclaredMethod("__jniTypeName", (java.lang.Class<?>[])null);
            o = m.invoke(null);
        }
        catch (final Exception e) {
        }
        return (String)o;
    }

    public static String getExtendedPrimitiveNullRet(final Class<?> type) {
        Object o = "<__jniNullRet:error>";
        try {
            final Method m = type.getDeclaredMethod("__jniNullRet", (java.lang.Class<?>[])null);
            o = m.invoke(null);
        }
        catch (final Exception e) {
        }
        return (String)o;
    }

    public static String getExtendedPrimitiveNativeType(final Class<?> type) {
        Object o = "<__jniNativeType:error>";
        try {
            final Method m = type.getDeclaredMethod("__jniNativeType", (java.lang.Class<?>[])null);
            o = m.invoke(null);
        }
        catch (final Exception e) {
        }
        return (String)o;
    }
}
