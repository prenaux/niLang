package ni.generator.cpp;

import java.io.PrintWriter;
import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.Buffer;
import java.util.logging.Level;
import java.util.logging.Logger;

import ni.types.BytePointer;
import ni.types.ExtendedPrimitive;
import ni.types.Pointer;
import ni.types.RefCountedPointer;
import ni.types.annotations.ByRef;
import ni.types.annotations.ByString;
import ni.types.annotations.Cast;
import ni.types.annotations.Const;
import ni.types.annotations.ConstMeth;
import ni.types.annotations.Name;
import ni.types.annotations.Namespace;
import ni.types.annotations.Ret;
import ni.types.annotations.Unsigned;

/**
 * @author Pierre Renaux
 */
class WrapperGenerator {
    private final Logger logger;
    private final PrintWriter out;
    private final LinkedListRegister<Class<?>> jclasses;

    public WrapperGenerator(final Logger aLogger, final PrintWriter aOut, final LinkedListRegister<Class<?>> aJClasses) {
        logger = aLogger;
        out = aOut;
        jclasses = aJClasses;
    }

    public void forwardDeclaration(final Class<?> cls) {
        final String namespaceName = cls.getAnnotation(Namespace.class).value();
        final String interfaceName = cls.getAnnotation(Name.class).value();
        outln("niExportFunc(" + namespaceName + "::" + interfaceName + "*) NewJNIWrapper_" + namespaceName + "_" + interfaceName + "(JNIEnv* e, jobject o);");
    }

    public void gatherMethods(final Class<?> cls) {
        final Method[] methods = Builder.getSortedDeclaredMethods(cls);
        for (int i = 0; i < methods.length; i++) {
            final Method m = methods[i];
            final Class<?>[] paramTypes = m.getParameterTypes();
            final Class<?> retType = m.getReturnType();
            if (Modifier.isAbstract(m.getModifiers())) {
                if (paramTypes.length == 0)
                    continue;
                for (int j = 0; j < paramTypes.length; ++j) {
                    if (RefCountedPointer.class.isAssignableFrom(paramTypes[j])) {
                        jclasses.register(paramTypes[j]);
                    }
                }
            }
        }
    }

    public void genMethods(final Class<?> cls) {
        System.out.println("D/ Impl wrapper of class : " + cls.getName());
        outln("");
        outln("// JNI Wrapper Implementation for Java Class : " + cls.getName());
        if (cls.getAnnotation(Name.class) == null) {
            logger.log(Level.WARNING, "Class \"" + cls.getName() + "\" should be named with the interface to implement.");
            return;
        }
        if (cls.getAnnotation(Namespace.class) == null) {
            logger.log(Level.WARNING, "Class \"" + cls.getName() + "\" should be define the namespace in which it resides.");
            return;
        }

        final String namespaceName = cls.getAnnotation(Namespace.class).value();
        final String interfaceName = cls.getAnnotation(Name.class).value();
        outln("// Implement JNI Wrapper for " + namespaceName + "::" + interfaceName);
        outln("struct JNI_" + namespaceName + "_" + interfaceName + " : public ni::cIUnknownImpl<" + interfaceName + "> {");
        outln("  jclass mClass;");
        outln("  jobject mObject;");
        final Method[] methods = Builder.getSortedDeclaredMethods(cls);
        for (int i = 0; i < methods.length; i++) {
            final Method m = methods[i];
            if (Modifier.isAbstract(m.getModifiers())) {
                outln("  mutable jmethodID mm" + getCppName(m) + ";");
            }
        }
        outln("");
        outln("  JNI_" + namespaceName + "_" + interfaceName + "(JNIEnv* e, jobject o) {");
        outln("    niAssert(e != NULL);");
        outln("    niAssert(o != NULL);");
        outln("    mClass = (jclass)e->NewGlobalRef(e->GetObjectClass(o));");
        outln("    mObject = e->NewGlobalRef(o);");
        outln("    NI_JVM_WRAPPER_REGISTER(e,o,this);");
        outln("  }");
        outln("  ~JNI_" + namespaceName + "_" + interfaceName + "() {");
        outln("    this->Invalidate();");
        outln("  }");
        outln("  virtual ni::tBool __stdcall IsOK() const {");
        outln("    return mObject != (jobject)NULL;");
        outln("  }");
        outln("  virtual void __stdcall Invalidate() {");
        outln("    if (mObject == (jobject)NULL) return;");
        outln("    JNIEnv* e = niJVM_GetCurrentEnv();");
        outln("    niAssert(e != NULL);");
        outln("    NI_JVM_WRAPPER_UNREGISTER(e,o,this);");
        outln("    e->DeleteGlobalRef(mObject);");
        outln("    e->DeleteGlobalRef(mClass);");
        outln("    mObject = (jobject)NULL; mClass = (jclass)NULL;");
        outln("  }");
        outln("");
        for (int i = 0; i < methods.length; i++) {
            final Method m = methods[i];
            final Class<?>[] paramTypes = m.getParameterTypes();
            final Class<?> retType = m.getReturnType();
            if (Modifier.isAbstract(m.getModifiers())) {
                implMethod(namespaceName, interfaceName, m, paramTypes, retType);
            }
            else {
                outln("- Skip : " + getCppName(m));
            }
        }

        outln("};");

        outln("niExportFunc(" + namespaceName + "::" + interfaceName + "*) NewJNIWrapper_" + namespaceName + "_" + interfaceName + "(JNIEnv* e, jobject o) {");
        outln("  NI_JVM_WRAPPER_TRY_GET_EXISTING(" + namespaceName + "," + interfaceName + ",e,o);");
        outln("  return niNew JNI_" + namespaceName + "_" + interfaceName + "(e,o);");
        outln("}");
    }

    private void implMethod(final String nsName, final String intfName, final Method m, final Class<?>[] paramTypes, final Class<?> retType)
    {
        final Annotation[] annotations = m.getAnnotations();
        final Annotation[][] parameterAnnotations = m.getParameterAnnotations();

        boolean isConstMeth = false;
        for (final Annotation a : annotations) {
            if (a instanceof ConstMeth) {
                isConstMeth = true;
            }
        }

        String javaSig = "(";
        for (int j = 0; j < paramTypes.length; ++j) {
            final Class<?> ptype = paramTypes[j];
            javaSig += getJavaTypeName(ptype);
        }
        javaSig += ")" + getJavaTypeName(retType);

        final String methCppName = getCppName(m);
        final String methJName = getJName(m);

        outln("  // Method: " + methCppName + " -> " + javaSig);
        out("  virtual " + getCPPTypeName(retType, annotations) + " __stdcall " + methCppName + "(");
        if (paramTypes.length == 0) {
            out(")");
        }
        else {
            for (int j = 0; j < paramTypes.length; ++j) {
                final Class<?> ptype = paramTypes[j];
                out(getCPPTypeName(ptype, parameterAnnotations[j]) + " p" + j);
                if (j + 1 == paramTypes.length) {
                    out(")");
                }
                else {
                    out(", ");
                }
            }
        }
        if (isConstMeth)
            out(" const");
        outln(" {");

        final String retNullValue = getCPPTypeNullRet(retType, annotations);
        outln("    NI_JVM_WRAPPER_METH_ENTER(" + nsName + "," + intfName + "," + methCppName + "," + methJName + ",\"" + javaSig + "\"," + retNullValue + ");");
        if (paramTypes.length == 0) {
            outln("    jvalue* args = NULL;");
        }
        else {
            outln("    jvalue args[" + paramTypes.length + "];");
            for (int j = 0; j < paramTypes.length; ++j) {
                beforeCallParam(j, paramTypes[j], parameterAnnotations[j]);
            }
        }
        doCall(m, retType, annotations);
        if (paramTypes.length == 0) {
            for (int j = 0; j < paramTypes.length; ++j) {
                afterCallParam(j, paramTypes[j], parameterAnnotations[j]);
            }
        }
        outln("    NI_JVM_WRAPPER_METH_LEAVE(" + nsName + "," + intfName + "," + methCppName + "," + methJName + "," + retNullValue + ");");
        doRet(retType, annotations);
        outln("  }");
    }

    public void out(final String str) {
        // System.out.print(str);
        Generator.print(out, str);
    }

    public void outln(final String str) {
        // System.out.println(str);
        Generator.println(out, str);
    }

    public void beforeCallParam(final int j, final Class<?> type, final Annotation[] annotations)
    {
        out("    ");
        if (type == String.class) {
            outln("LocalJString jstr_p" + j + "(e,p" + j + ");");
            outln("    args[" + j + "].l = jstr_p" + j + ".o;");
        }
        else if (type == Object.class) {
            outln("jobject var_p" + j + " = niJVM_NewVar(e, p" + j + ");");
            outln("    LocalJRef<jobject> jvar_p" + j + "(e,var_p" + j + ");");
            outln("    args[" + j + "].l = jvar_p" + j + ".o;");
        }
        else if (Buffer.class.isAssignableFrom(type)) {
            outln("jobject var_p" + j + " = e->NewDirectByteBuffer(p" + j + ",p" + (j + 1) + ");");
            outln("    LocalJRef<jobject> jvar_p" + j + "(e,var_p" + j + ");");
            outln("    args[" + j + "].l = jvar_p" + j + ".o;");
        }
        else if (Pointer.class.isAssignableFrom(type)) {
            outln("    args[" + j + "].l = OUCH;");
        }
        else if (BytePointer.class.isAssignableFrom(type)) {
            outln("    args[" + j + "].l = OUCH;");
        }
        else if (RefCountedPointer.class.isAssignableFrom(type)) {
            String intfType = getCPPTypeName(type, null);
            intfType = intfType.substring(0, intfType.length() - 1);
            outln("jobject iu_p" + j + " = niJVM_IUnknownNew(e, ni_getClass(e, " + jclasses.getIndex(type) + "), &niGetInterfaceUUID(" + intfType + "), p" + j + ");");
            outln("    LocalJRef<jobject> jiu_p" + j + "(e,iu_p" + j + ");");
            outln("    args[" + j + "].l = jiu_p" + j + ".o;");
        }
        else if (ExtendedPrimitive.class.isAssignableFrom(type)) {
            outln("jobject xt_p" + j + " = niJVM_New" + MethodInformation.getExtendedPrimitiveTypeName(type) + "(e, p" + j + ");");
            outln("    LocalJRef<jobject> jxt_p" + j + "(e,xt_p" + j + ");");
            outln("    args[" + j + "].l = jxt_p" + j + ".o;");
        }
        else {
            out("args[" + j + "]");
            if (type == void.class) {
                out("<void error>");
            }
            else if (type == boolean.class) {
                out(".z = p" + j);
            }
            else if (type == byte.class) {
                out(".b = p" + j);
            }
            else if (type == short.class) {
                out(".s = p" + j);
            }
            else if (type == int.class) {
                out(".i = p" + j);
            }
            else if (type == long.class) {
                out(".j = p" + j);
            }
            else if (type == float.class) {
                out(".f = p" + j);
            }
            else if (type == double.class) {
                out(".d = p" + j);
            }
            else {
                out(".l = p" + j);
            }
            outln(";");
        }
    }

    public void afterCallParam(final int i, final Class<?> type, final Annotation[] annotations)
    {
    }

    public void doCall(final Method m, final Class<?> retType, final Annotation[] annotations)
    {
        final String closeExtra = "";
        out("    ");
        if (retType == void.class) {
            out("e->CallVoidMethodA");
        }
        else if (retType == boolean.class) {
            out("jboolean r = e->CallBooleanMethodA");
        }
        else if (retType == byte.class) {
            out("jbyte r = e->CallByteMethodA");
        }
        else if (retType == short.class) {
            out("jshort r = e->CallShortMethodA");
        }
        else if (retType == int.class) {
            out("jint r = e->CallIntMethodA");
        }
        else if (retType == long.class) {
            out("jlong r = e->CallLongMethodA");
        }
        else if (retType == float.class) {
            out("jfloat r = e->CallFloatMethodA");
        }
        else if (retType == double.class) {
            out("jdouble r = e->CallDoubleMethodA");
        }
        else if (retType == String.class) {
            out("jstring r = (jstring)e->CallObjectMethodA");
        }
        else {
            out("jobject r = e->CallObjectMethodA");
        }
        outln("(mObject,mm" + getCppName(m) + ",args)" + closeExtra + ";");
    }

    public void doRet(final Class<?> retType, final Annotation[] annotations) {
        if (void.class == retType)
            return; /* nothing to do... */

        out("    ");

        String cast = null;
        String retPrefix = null;
        String retSuffix = null;
        for (final Annotation a : annotations) {
            if (a instanceof Cast) {
                cast = ((Cast)a).value();
            }
			else if (a instanceof ByString) {
				cast = "const ni::achar*";
			}
            else if (a instanceof Ret) {
                retPrefix = ((Ret)a).prefix();
                retSuffix = ((Ret)a).suffix();
            }
        }

        if (retPrefix == null)
            retPrefix = "";
        if (retSuffix == null)
            retSuffix = "";

        if (cast != null) {
            outln("return " + retPrefix + "(" + cast + ")r" + retSuffix + ";");
        }
        else if ((retType == boolean.class) ||
                 (retType == byte.class) ||
                 (retType == short.class) ||
                 (retType == int.class) ||
                 (retType == long.class) ||
                 (retType == float.class) ||
                 (retType == double.class)) {
            outln("return " + retPrefix + "r" + retSuffix + ";");
        }
        else if (retType == Object.class) {
            outln("ni::Var var_r = niJVM_GetVar(e,r);");
            outln("return " + retPrefix + "var_r" + retSuffix + ";");
        }
        else if (ExtendedPrimitive.class.isAssignableFrom(retType)) {
            outln("return " + retPrefix + "niJVM_Get" + MethodInformation.getExtendedPrimitiveTypeName(retType) + "(e,r)" + retSuffix + ";");
        }
        else if (RefCountedPointer.class.isAssignableFrom(retType)) {
            String intfType = getCPPTypeName(retType, null);
            intfType = intfType.substring(0, intfType.length() - 1);
            outln("return " + retPrefix + "niJVM_GetIUnknown(e,r," + intfType + ")" + retSuffix + ";");
        }
        else {
            outln("return " + retPrefix + "r" + retSuffix + ";");
        }
    }

    public String getCPPTypeNullRet(final Class<?> type, final Annotation[] annotations) {
        /* boolean isUnsigned = false; */
        for (final Annotation a : annotations) {
            if (a instanceof Cast) {
                return "(" + ((Cast)a).value() + ")NULL";
            }
            else if (a instanceof Unsigned) {
                /* isUnsigned = true; */
            }
        }

        final Name name = type.getAnnotation(Name.class);
        if (name != null) {
            final Namespace ns = type.getAnnotation(Namespace.class);
            if (ns != null) {
                return "(" + ns.value() + "::" + name.value() + "*)NULL";
            }
            else {
                return "(" + name.value() + "*)NULL";
            }
        }

        if (type == void.class) {
            return ";";
        }
        else if ((type == Boolean.class) || (type == boolean.class)) {
            return "ni::eFalse";
        }
        else if ((type == Byte.class) || (type == byte.class)) {
            return "0";
        }
        else if ((type == Short.class) || (type == short.class)) {
            return "0";
        }
        else if ((type == Integer.class) || (type == int.class)) {
            return "0";
        }
        else if ((type == Long.class) || (type == long.class)) {
            return "0";
        }
        else if ((type == Float.class) || (type == float.class)) {
            return "0";
        }
        else if ((type == Double.class) || (type == double.class)) {
            return "0";
        }
        else if (type == String.class) {
            return "AZEROSTR";
        }
        else if (type == Object.class) {
            return "niVarNull";
        }
        else if (ExtendedPrimitive.class.isAssignableFrom(type)) {
            return MethodInformation.getExtendedPrimitiveNullRet(type);
        }
        else {
            return "<unknown type>";
        }
    }

    public String getCPPTypeName(final Class<?> type, final Annotation[] annotations) {
        boolean isConst = false;
        boolean isByRef = false;
        boolean isUnsigned = false;
		boolean isByString = false;
        if (annotations != null) {
            for (final Annotation a : annotations) {
                if (a instanceof Cast) {
                    return ((Cast)a).value();
                }
                else if (a instanceof Unsigned) {
                    isUnsigned = true;
                }
                else if (a instanceof Const) {
                    isConst = true;
                }
                else if (a instanceof ByRef) {
                    isByRef = true;
                }
                else if (a instanceof ByString) {
                    isByString = true;
                }
            }
        }

        String r = isConst ? "const " : "";

        final Name typeName = type.getAnnotation(Name.class);
        if (typeName != null) {
            final Namespace ns = type.getAnnotation(Namespace.class);
            if (ns != null) {
                r += ns.value() + "::" + typeName.value() + "*";
            }
            else {
                r += typeName.value() + "*";
            }
        }
        else if (type == void.class) {
            r += "void";
        }
        else if ((type == Boolean.class) || (type == boolean.class)) {
            r += "ni::tBool";
        }
        else if ((type == Byte.class) || (type == byte.class)) {
            if (isUnsigned)
                r += "ni::tU8";
            else
                r += "ni::tI8";
        }
        else if ((type == Short.class) || (type == short.class)) {
            if (isUnsigned)
                r += "ni::tU16";
            else
                r += "ni::tI16";
        }
        else if ((type == Integer.class) || (type == int.class)) {
            if (isUnsigned)
                r += "ni::tU32";
            else
                r += "ni::tI32";
        }
        else if ((type == Long.class) || (type == long.class)) {
            if (isUnsigned)
                r += "ni::tU64";
            else
                r += "ni::tI64";
        }
        else if ((type == Float.class) || (type == float.class)) {
            r += "ni::tF32";
        }
        else if ((type == Double.class) || (type == double.class)) {
            r += "ni::tF64";
        }
        else if (type == String.class) {
			if (isByString) {
				r += "ni::cString";
			}
			else {
				r += "const ni::achar*";
			}
        }
        else if (type == Object.class) {
            r += "ni::Var";
        }
        else if (ExtendedPrimitive.class.isAssignableFrom(type)) {
            r += MethodInformation.getExtendedPrimitiveNativeType(type);
        }
        else {
            return "<unknown type>";
        }
        if (isByRef) {
            r += "&";
        }
        return r;
    }

    public String getJavaTypeName(final Class<?> type) {
        if (type == void.class) {
            return "V";
        }
        else if (type == boolean.class) {
            return "Z";
        }
        else if (type == byte.class) {
            return "B";
        }
        else if (type == short.class) {
            return "S";
        }
        else if (type == int.class) {
            return "I";
        }
        else if (type == long.class) {
            return "J";
        }
        else if (type == float.class) {
            return "F";
        }
        else if (type == double.class) {
            return "D";
        }
        else {
            return "L" + type.getName().replace('.', '/') + ";";
        }
    }

    private String getCppName(final Method m) {
        final Name memberName = m.getAnnotation(Name.class);
        if (memberName != null) {
            return memberName.value();
        }
        return Character.toUpperCase(m.getName().charAt(0)) + m.getName().substring(1);
    }

    private String getJName(final Method m) {
        return Character.toLowerCase(m.getName().charAt(0)) + m.getName().substring(1);
    }
}
