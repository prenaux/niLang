/*
 * Copyright (C) 2011-2013 Samuel Audet
 *
 * Licensed either under the Apache License, Version 2.0, or (at your option)
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation (subject to the "Classpath" exception),
 * either version 2, or any later version (collectively, the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *     http://www.gnu.org/licenses/
 *     http://www.gnu.org/software/classpath/license.html
 *
 * or as provided in the LICENSE.txt file that accompanied this code.
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package ni.generator.cpp;

import java.io.Closeable;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.io.Writer;
import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.DoubleBuffer;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;

import ni.types.BytePointer;
import ni.types.CLongPointer;
import ni.types.CharPointer;
import ni.types.DoublePointer;
import ni.types.ExtendedPrimitive;
import ni.types.FloatPointer;
import ni.types.FunctionPointer;
import ni.types.IUnknown;
import ni.types.IntPointer;
import ni.types.LongPointer;
import ni.types.Pointer;
import ni.types.PointerPointer;
import ni.types.RefCountedPointer;
import ni.types.ShortPointer;
import ni.types.SizeTPointer;
import ni.types.annotations.ByPtr;
import ni.types.annotations.ByPtrPtr;
import ni.types.annotations.ByPtrRef;
import ni.types.annotations.ByRef;
import ni.types.annotations.BySmartPtr;
import ni.types.annotations.ByVal;
import ni.types.annotations.Cast;
import ni.types.annotations.Const;
import ni.types.annotations.Convention;
import ni.types.annotations.ImplWrapper;
import ni.types.annotations.Name;
import ni.types.annotations.Namespace;
import ni.types.annotations.NoOffset;
import ni.types.annotations.Opaque;
import ni.types.annotations.Platform;

/**
 * @author Samuel Audet
 */
public class Generator implements Closeable {
    public Generator(final Properties properties, final File file) {
        this.properties = properties;
        this.file = file;
        this.writer = null;
    }

    public static final String JNI_VERSION = "JNI_VERSION_1_4";

    private static final Logger logger = Logger.getLogger(Generator.class.getName());

    private final Properties properties;
    private final File file;
    private final PrintWriter writer;

    private PrintWriter out;
    private LinkedListRegister<String> functionDefinitions, functionPointers;
    private LinkedListRegister<Class<?>> deallocators, arrayDeallocators, jclasses;
    private HashMap<Class<?>,LinkedList<String>> members;
    private boolean generatedSomethingUseful;
    private String moduleName;

    public boolean generate(final String moduleName, final Class<?>... classes) throws FileNotFoundException
    {
        this.moduleName = (moduleName == null) ? "" : moduleName;

        // first pass using a null writer to fill up the LinkedListRegister objects
        out = new PrintWriter(new Writer() {
            @Override
            public void close() {
            }

            @Override
            public void flush() {
            }

            @Override
            public void write(final char[] cbuf, final int off, final int len) {
            }
        });
        functionDefinitions = new LinkedListRegister<String>(Builder.CMP_STRING);
        functionPointers = new LinkedListRegister<String>(Builder.CMP_STRING);
        deallocators = new LinkedListRegister<Class<?>>(Builder.CMP_CLASS);
        arrayDeallocators = new LinkedListRegister<Class<?>>(Builder.CMP_CLASS);
        jclasses = new LinkedListRegister<Class<?>>(Builder.CMP_CLASS);
        members = new HashMap<Class<?>,LinkedList<String>>();
        if (classes == null) {
            doBaseClasses();
        }
        else {
            doClasses(classes);
        }

        if (generatedSomethingUseful) {
            // second pass with the real writer
            out = writer != null ? writer : new PrintWriter(file);
            if (classes == null) {
                doBaseClasses();
            }
            else {
                doClasses(classes);
            }
            return true;
        }

        return false;
    }

    @Override
    public void close() {
        if (out != null) {
            out.flush();
            out.close();
        }
    }

    private void doIncludes() {
        final HashMap<String,Boolean> written = new HashMap<String,Boolean>();

        final String define = properties.getProperty("generator.define");
        if ((define != null) && (define.length() > 0)) {
            for (final String s : define.split("\u0000")) {
                Generator.println(out, "#define " + s);
            }
            Generator.println(out);
        }
        final String[] include = {
            properties.getProperty("generator.include"),
            properties.getProperty("generator.cinclude")
        };
        for (int i = 0; i < include.length; i++) {
            if ((include[i] != null) && (include[i].length() > 0)) {
                if (i == 1) {
                    Generator.println(out, "extern \"C\" {");
                }
                for (final String s : include[i].split("\u0000")) {
                    if (written.get(s) != null)
                        continue;

                    written.put(s, true);
                    final char[] chars = s.toCharArray();
                    Generator.print(out, "#include ");
                    char c = chars[0];
                    if ((c != '<') && (c != '"')) {
                        Generator.print(out, '"');
                    }
                    Generator.print(out, chars);
                    c = chars[chars.length - 1];
                    if ((c != '>') && (c != '"')) {
                        Generator.print(out, '"');
                    }
                    Generator.println(out);
                }
                if (i == 1) {
                    Generator.println(out, "}");
                }
                Generator.println(out);
            }
        }
        Generator.println(out, "#include <niLang/Utils/JNIUtils.h>");
    }

    private int writeGetClasses() {
        for (final String s : functionDefinitions) {
            Generator.println(out, s);
        }
        Generator.println(out);
        for (final String s : functionPointers) {
            Generator.println(out, "static jobject " + s + " = NULL;");
        }
        Generator.println(out);
        for (final Class<?> c : deallocators) {
            final String mangledName = mangle(c.getName());
            final String typeName = getCPPTypeName(c);
            Generator.println(out, "static void ni_" + mangledName + "_deallocate(" + typeName + " address) {");
            Generator.println(out, "    delete address;");
            Generator.println(out, "}");
        }
        for (final Class<?> c : arrayDeallocators) {
            final String mangledName = mangle(c.getName());
            final String typeName = getCPPTypeName(c);
            Generator.println(out, "static void ni_" + mangledName + "_deallocateArray(" + typeName + " address) {");
            Generator.println(out, "    delete[] address;");
            Generator.println(out, "}");
        }
        Generator.println(out);

        int maxMemberSize = 0;

        if (jclasses.size() > 0) {
            Generator.println(out, "static const char *ni_classNames[" + jclasses.size() + "] = {");
            final Iterator<Class<?>> classIterator = jclasses.iterator();
            while (classIterator.hasNext()) {
                final Class<?> c = classIterator.next();
                Generator.print(out, "        \"" + c.getName().replace('.', '/') + "\"");
                if (classIterator.hasNext()) {
                    Generator.println(out, ",");
                }
                final LinkedList<String> m = members.get(c);
                if ((m != null) && (m.size() > maxMemberSize)) {
                    maxMemberSize = m.size();
                }
            }
            Generator.println(out, " };");

            Generator.println(out, "static jclass ni_classes[" + jclasses.size() + "] = { NULL };");
            Generator.println(out);
            Generator.println(out, "static jclass ni_getClass(JNIEnv *e, int i) {");
            Generator.println(out, "    niAssert(i < niCountOf(ni_classes));");
            Generator.println(out, "    if (ni_classes[i] == NULL) {");
            Generator.println(out, "        jclass c = e->FindClass(ni_classNames[i]);");
            Generator.println(out, "        if (c == NULL || e->ExceptionCheck()) {");
            Generator.println(out, "            fprintf(stderr, \"Error loading class %s.\", ni_classNames[i]);");
            Generator.println(out, "            return NULL;");
            Generator.println(out, "        }");
            Generator.println(out, "        ni_classes[i] = (jclass)e->NewGlobalRef(c);");
            Generator.println(out, "        if (ni_classes[i] == NULL || e->ExceptionCheck()) {");
            Generator.println(out, "            fprintf(stderr, \"Error creating global reference of class %s.\", ni_classNames[i]);");
            Generator.println(out, "            return NULL;");
            Generator.println(out, "        }");
            Generator.println(out, "    }");
            Generator.println(out, "    return ni_classes[i];");
            Generator.println(out, "}");
            Generator.println(out);
        }

        return maxMemberSize;
    }

    private void writeDeleteClassesRefs() {
        for (final String s : functionPointers) {
            Generator.println(out, "    e->DeleteGlobalRef(" + s + ");");
        }
        if (jclasses.size() > 0) {
            Generator.println(out, "    for (int i = 0; i < " + jclasses.size() + "; i++) {");
            Generator.println(out, "        e->DeleteGlobalRef(ni_classes[i]);");
            Generator.println(out, "    }");
        }
    }

    private void writeMembersSize(final int maxMemberSize, final boolean isBaseClasses) {
        if ((maxMemberSize <= 0) || (jclasses.size() > 0))
            return;
        Generator.println(out, "    const char *members[" + jclasses.size() + "][" + maxMemberSize + "] = {");
        Iterator<Class<?>> classIterator = jclasses.iterator();
        while (classIterator.hasNext()) {
            Generator.print(out, "            { ");
            final LinkedList<String> m = members.get(classIterator.next());
            final Iterator<String> memberIterator = m == null ? null : m.iterator();
            while ((memberIterator != null) && memberIterator.hasNext()) {
                Generator.print(out, "\"" + memberIterator.next() + "\"");
                if (memberIterator.hasNext()) {
                    Generator.print(out, ", ");
                }
            }
            Generator.print(out, " }");
            if (classIterator.hasNext()) {
                Generator.println(out, ",");
            }
        }
        Generator.println(out, " };");
        Generator.println(out, "    int offsets[" + jclasses.size() + "][" + maxMemberSize + "] = {");
        classIterator = jclasses.iterator();
        while (classIterator.hasNext()) {
            Generator.print(out, "            { ");
            final Class<?> c = classIterator.next();
            final LinkedList<String> m = members.get(c);
            final Iterator<String> memberIterator = m == null ? null : m.iterator();
            while ((memberIterator != null) && memberIterator.hasNext()) {
                final String typeName = getCPPTypeName(c);
                String valueTypeName = typeName.substring(0, typeName.length() - 1);
                final String memberName = memberIterator.next();
                if ("sizeof".equals(memberName)) {
                    if ("void".equals(valueTypeName)) {
                        valueTypeName = "void*";
                    }
                    else if ("ni::iUnknown".equals(valueTypeName)) {
                        valueTypeName = "ni::iUnknown*";
                    }
                    Generator.print(out, "sizeof(" + valueTypeName + ")");
                }
                else {
                    Generator.print(out, "offsetof(" + valueTypeName + "," + memberName + ")");
                }
                if (memberIterator.hasNext()) {
                    Generator.print(out, ", ");
                }
            }
            Generator.print(out, " }");
            if (classIterator.hasNext()) {
                Generator.println(out, ",");
            }
        }
        Generator.println(out, " };");
        Generator.print(out, "    int memberOffsetSizes[" + jclasses.size() + "] = { ");
        classIterator = jclasses.iterator();
        while (classIterator.hasNext()) {
            final LinkedList<String> m = members.get(classIterator.next());
            Generator.print(out, m == null ? "0" : m.size());
            if (classIterator.hasNext()) {
                Generator.print(out, ", ");
            }
        }
        Generator.println(out, " };");
        Generator.println(out, "    for (int i = 0; i < " + jclasses.size() + " && !e->ExceptionCheck(); i++) {");
        Generator.println(out, "        for (int j = 0; j < memberOffsetSizes[i] && !e->ExceptionCheck(); j++) {");
        Generator.println(out, "            if (e->PushLocalFrame(2) == 0) {");
        Generator.println(out, "                if (!niJVM_PutMemberOffset(e,ni_classNames[i],members[i][j],offsets[i][j])) {");
        Generator.println(out, "                    return 0;");
        Generator.println(out, "                }");
        Generator.println(out, "                e->PopLocalFrame(NULL);");
        Generator.println(out, "            }");
        Generator.println(out, "        }");
        Generator.println(out, "    }");
    }

    private void doHeader() {
        Generator.println(out, "/* DO NOT EDIT THIS FILE - IT IS MACHINE GENERATED */");
        doIncludes();
        Generator.println(out, "#include <niLang/Types.h>");
        Generator.println(out);
        Generator.println(out, "#if !defined niJNI");
        Generator.println(out, "#pragma message(\"# JNI: NOT Compiling\")");
        Generator.println(out, "#endif // niJNI");
        Generator.println(out);
        Generator.println(out, "#ifdef niJNI");
        Generator.println(out, "#pragma message(\"# JNI: Compiling\")");
        Generator.println(out, "extern \"C\" {");
    }

    private void doFooter() {
        Generator.println(out);
        Generator.println(out, "} // extern \"C\"");
        Generator.println(out, "#endif // ifdef niJNI");
    }

    private static Class<?> baseClasses[] = {
        RefCountedPointer.class,
        Pointer.class,
        BytePointer.class,
        ShortPointer.class,
        IntPointer.class,
        LongPointer.class,
        FloatPointer.class,
        DoublePointer.class,
        CharPointer.class,
        PointerPointer.class,
        CLongPointer.class,
        SizeTPointer.class,
        IUnknown.class,
        // FunctionPointer.class,
    };

    private void doBaseClasses() {
        doHeader();
        Generator.println(out, "static sJVMContext* _JVM = NULL;");
        final int maxMemberSize = writeGetClasses();
        Generator.println(out, "niExportFunc(jint) niJVM_OnLoad_Base(JNIEnv* e) {");
        Generator.println(out, "    _JVM = niJVM_GetContext();");
        writeMembersSize(maxMemberSize, true);
        Generator.println(out, "    return e->GetVersion();");
        Generator.println(out, "}");
        Generator.println(out);
        Generator.println(out, "niExportFunc(void) niJVM_OnUnload_Base(JNIEnv* e) {");
        writeDeleteClassesRefs();
        Generator.println(out, "}");
        Generator.println(out);

        generatedSomethingUseful = false;
        for (Class<?> c : baseClasses) {
            gatherMethods(c);
        }
        for (Class<?> c : baseClasses) {
            doMethods(c);
        }
        doFooter();
    }

    private void doClasses(final Class<?>[] classes) {
        doHeader();
        Generator.println(out, "static sJVMContext* _JVM = NULL;");
        final int maxMemberSize = writeGetClasses();
        Generator.println(out, "JNIEXPORT jint JNICALL JNI_OnLoad_" + moduleName + "(JavaVM *vm, void *reserved) {");
        Generator.println(out, "    JNIEnv* e;");
        Generator.println(out, "    if (niJVM_OnLoad(vm, (void**)&e, " + JNI_VERSION + ") == 0) {");
        Generator.println(out, "        fprintf(stderr, \"niJVM_OnLoad() failed !\");");
        Generator.println(out, "        return 0;");
        Generator.println(out, "    }");
        Generator.println(out, "    _JVM = niJVM_GetContext();");
        writeMembersSize(maxMemberSize, false);
        Generator.println(out, "    return e->GetVersion();");
        Generator.println(out, "}");
        Generator.println(out);
        Generator.println(out, "JNIEXPORT void JNICALL JNI_OnUnload_" + moduleName + "(JavaVM *vm, void *reserved) {");
        Generator.println(out, "    JNIEnv* e;");
        Generator.println(out, "    if (niJVM_OnUnload(vm, (void**)&e, " + JNI_VERSION + ") != JNI_OK) {");
        Generator.println(out, "        fprintf(stderr, \"niJVM_OnUnload() failed !\");");
        Generator.println(out, "        return;");
        Generator.println(out, "    }");
        writeDeleteClassesRefs();
        Generator.println(out, "}");
        Generator.println(out);
        Generator.println(out, "#if !defined niLib");
        Generator.println(out, "JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {");
        Generator.println(out, "    return JNI_OnLoad_" + moduleName + "(vm,reserved);");
        Generator.println(out, "}");
        Generator.println(out, "JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {");
        Generator.println(out, "    JNI_OnUnload_" + moduleName + "(vm,reserved);");
        Generator.println(out, "}");
        Generator.println(out, "#endif // defined niLib");
        Generator.println(out);

        generatedSomethingUseful = false;
        final WrapperGenerator wrapGen = new WrapperGenerator(logger, out, jclasses);

        for (final Class<?> cls : classes) {
            System.out.println("# Generate for class : " + cls.getName());
            if (cls.getAnnotation(ImplWrapper.class) != null) {
                wrapGen.gatherMethods(cls);
            }
            else {
                gatherMethods(cls);
            }
        }

        Generator.println(out, "/// Forward declarations ///");
        for (final Class<?> cls : classes) {
            if (cls.getAnnotation(ImplWrapper.class) != null) {
                wrapGen.forwardDeclaration(cls);
            }
        }
        Generator.println(out);

        Generator.println(out, "/// Methods declarations ///");
        for (final Class<?> cls : classes) {
            if (cls.getAnnotation(ImplWrapper.class) != null) {
                wrapGen.genMethods(cls);
            }
            else {
                doMethods(cls);
            }
        }

        doFooter();
    }

    private boolean checkClassPlatform(final Class<?> cls) {
        final ni.types.annotations.Properties classProperties =
                cls.getAnnotation(ni.types.annotations.Properties.class);
        boolean platformMatches = false;
        if (classProperties != null) {
            for (final Platform p : classProperties.value()) {
                if (checkPlatform(p)) {
                    platformMatches = true;
                }
            }
        }
        else if (checkPlatform(cls.getAnnotation(Platform.class))) {
            platformMatches = true;
        }
        return platformMatches;
    }

    private void gatherMethods(final Class<?> cls) {
        if (!checkClassPlatform(cls))
            return;

        final Class<?>[] classes = Builder.getSortedDeclaredClasses(cls);
        for (int i = 0; i < classes.length; i++) {
            if (Pointer.class.isAssignableFrom(classes[i]) ||
                RefCountedPointer.class.isAssignableFrom(classes[i]) ||
                Pointer.Deallocator.class.isAssignableFrom(classes[i])) {
                gatherMethods(classes[i]);
            }
        }

        final Method[] methods = Builder.getSortedDeclaredMethods(cls);
        final boolean[] callbackAllocators = new boolean[methods.length];
        final Method functionMethod = doFunctionDefinitions(cls, callbackAllocators);
        for (int i = 0; i < methods.length; i++) {
            if (!checkPlatform(methods[i].getAnnotation(Platform.class))) {
                continue;
            }
            final MethodInformation methodInfo = MethodInformation.getMethodInformation(logger, methods[i]);
            if (methodInfo == null) {
                continue;
            }
            if (callbackAllocators[i]) {
                gatherCallback(functionMethod);
            }
            gatherReturn(cls, methodInfo);
        }
    }

    private void doMethods(final Class<?> cls) {
        if (!checkClassPlatform(cls)) {
            return;
        }

        generatedSomethingUseful = true;

        LinkedList<String> memberList = members.get(cls);
        if ((!cls.isAnnotationPresent(Opaque.class) || (cls == Pointer.class)) &&
            !RefCountedPointer.class.isAssignableFrom(cls) &&
            !FunctionPointer.class.isAssignableFrom(cls) &&
            !cls.isAnnotationPresent(NoOffset.class)) {
            if (memberList == null) {
                members.put(cls, memberList = new LinkedList<String>());
            }
            if (!memberList.contains("sizeof")) {
                memberList.add("sizeof");
            }
        }

        final Class<?>[] classes = Builder.getSortedDeclaredClasses(cls);
        for (int i = 0; i < classes.length; i++) {
            if (Pointer.class.isAssignableFrom(classes[i]) ||
                RefCountedPointer.class.isAssignableFrom(classes[i]) ||
                Pointer.Deallocator.class.isAssignableFrom(classes[i])) {
                doMethods(classes[i]);
            }
        }

        final Method[] methods = Builder.getSortedDeclaredMethods(cls);
        final boolean[] callbackAllocators = new boolean[methods.length];
        final Method functionMethod = doFunctionDefinitions(cls, callbackAllocators);
        for (int i = 0; i < methods.length; i++) {
            if (!checkPlatform(methods[i].getAnnotation(Platform.class))) {
                continue;
            }
            final MethodInformation methodInfo = MethodInformation.getMethodInformation(logger, methods[i]);
            if (methodInfo == null) {
                continue;
            }
            final String baseFunctionName = mangle(cls.getName()) + "_" + mangle(methodInfo.name);

            if ((methodInfo.memberGetter || methodInfo.memberSetter) && !methodInfo.noOffset &&
                (memberList != null) && !Modifier.isStatic(methodInfo.modifiers)) {
                if (!memberList.contains(methodInfo.memberName)) {
                    memberList.add(methodInfo.memberName);
                }
            }

            if (callbackAllocators[i]) {
                doCallback(cls, functionMethod, baseFunctionName);
            }

            Generator.println(out);
            Generator.print(out, "JNIEXPORT " + getJNITypeName(methodInfo.returnType) +
                                 " JNICALL Java_" + baseFunctionName);
            if (methodInfo.overloaded) {
                Generator.print(out, "__" + mangle(getSignature(methodInfo.parameterTypes)));
            }
            if (Modifier.isStatic(methodInfo.modifiers)) {
                Generator.print(out, "(JNIEnv *e, jclass c");
            }
            else {
                Generator.print(out, "(JNIEnv *e, jobject o");
            }
            for (int j = 0; j < methodInfo.parameterTypes.length; j++) {
                Generator.print(out, ", " + getJNITypeName(methodInfo.parameterTypes[j]) + " p" + j);
            }
            Generator.println(out, ") {");

            if (callbackAllocators[i]) {
                doCallbackAllocator(cls, functionMethod, baseFunctionName);
                continue;
            }
            else if (!Modifier.isStatic(methodInfo.modifiers) && methodInfo.refCounted) {
                // get our "this" pointer
                final String typeName = getCPPTypeName(cls);
                final String thisType = typeName;
                Generator.println(out, "    " + thisType + " pointer = niJVM_GetIUnknown(e,o," + thisType.substring(0, thisType.length() - 1) + ");");
                Generator.println(out, "    if (niJVM_CheckNull(e, (uintptr_t)pointer)) {");
                Generator.println(out, "        return" + (methodInfo.returnType == void.class ? ";" : " 0;"));
                Generator.println(out, "    }");
                Generator.println(out, "    niAssume(pointer != NULL);");
            }
            else if (!Modifier.isStatic(methodInfo.modifiers) && !methodInfo.allocator &&
                     !methodInfo.arrayAllocator && !methodInfo.deallocator) {
                // get our "this" pointer
                final String typeName = getCPPTypeName(cls);
                final String thisType = methodInfo.bufferGetter && "void*".equals(typeName) ?
                        "char*" : typeName;
                Generator.println(out, "    " + thisType + " pointer = (" + thisType + ")" +
                                       "jlong_to_ptr(e->GetLongField(o, _JVM->addressFieldID));");
                Generator.println(out, "    if (niJVM_CheckNull(e, (uintptr_t)pointer)) {");
                Generator.println(out, "        return" + (methodInfo.returnType == void.class ? ";" : " 0;"));
                Generator.println(out, "    }");
                Generator.println(out, "    niAssume(pointer != NULL);");
                if (!cls.isAnnotationPresent(Opaque.class) || methodInfo.bufferGetter) {
                    Generator.println(out, "    jint position = e->GetIntField(o, _JVM->positionFieldID);");
                    if (FunctionPointer.class.isAssignableFrom(cls)) {
                        Generator.println(out, "    pointer = position == 0 ? pointer : (" + thisType + ")((uintptr_t)pointer + position);");
                    }
                    else {
                        Generator.println(out, "    pointer += position;");
                    }
                }
            }

            final boolean[] hasPointer = new boolean[methodInfo.parameterTypes.length];
            final boolean[] mayChange = new boolean[methodInfo.parameterTypes.length];
            doParametersBefore(methodInfo, hasPointer, mayChange);
            final String returnVariable = doReturnBefore(cls, methodInfo);
            final boolean needsCatchBlock = doMainOperation(cls, methodInfo, returnVariable, hasPointer);
            doReturnAfter(cls, methodInfo, returnVariable, needsCatchBlock);
            doParametersAfter(methodInfo, needsCatchBlock, mayChange);
            if (methodInfo.returnType != void.class) {
                Generator.println(out, "    return r;");
            }
            Generator.println(out, "}");
        }
    }

    private void genAndLogFatalTypeError(final String kind, final Method method, final String typeName) {
        final String msg = kind + " '" + method + "' has unsupported return type '" + typeName + "'. Invalid code generated, compilation will fail.";
        Generator.println(out, "#error(\"" + msg + "\");");
        logger.log(Level.SEVERE, msg);
    }

    private void doParametersBefore(final MethodInformation methodInfo,
            final boolean[] hasPointer, final boolean[] mayChange)
    {
        for (int j = 0; j < methodInfo.parameterTypes.length; j++) {
            Class<?> passBy = getBy(methodInfo.parameterAnnotations[j]);
            if ((passBy == null) && (methodInfo.pairedMethod != null) &&
                (methodInfo.valueSetter || methodInfo.memberSetter)) {
                passBy = getBy(methodInfo.pairedMethod.getAnnotations());
            }
            hasPointer[j] = true;
            final String typeName = getCPPTypeName(methodInfo.parameterTypes[j]);

            if (RefCountedPointer.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                Generator.println(out, "    " + typeName + " pointer" + j + " = p" + j + " == NULL ? NULL : (" +
                                       typeName + ")jlong_to_ptr(e->GetLongField(p" + j + ", _JVM->refCountedObjPtrFieldID));");
                if (!methodInfo.parameterTypes[j].isAnnotationPresent(Opaque.class) && (passBy != ByPtrPtr.class) && (passBy != ByPtrRef.class)) {
                    /* ...nothing... */
                }
                else if ((passBy == ByPtrPtr.class) || (passBy == ByPtrRef.class)) {
                    mayChange[j] = true;
                }
            }
            else if (Pointer.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                Generator.println(out, "    " + typeName + " pointer" + j + " = p" + j + " == NULL ? NULL : (" +
                                       typeName + ")jlong_to_ptr(e->GetLongField(p" + j + ", _JVM->addressFieldID));");
                if (!methodInfo.parameterTypes[j].isAnnotationPresent(Opaque.class) &&
                    (passBy != ByPtrPtr.class) && (passBy != ByPtrRef.class)) {
                    Generator.println(out, "    jint position" + j + " = p" + j + " == NULL ? 0 : e->GetIntField(p" + j +
                                           ", _JVM->positionFieldID);");
                    if (FunctionPointer.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                        Generator.println(out, "    pointer" + j + " = position" + j + " == 0 ? pointer" + j +
                                               " : (" + typeName + ")((uintptr_t)pointer" + j + " + position" + j + ");");
                    }
                    else {
                        Generator.println(out, "    pointer" + j + " += position" + j + ";");
                    }
                }
                else if ((passBy == ByPtrPtr.class) || (passBy == ByPtrRef.class)) {
                    mayChange[j] = true;
                }
            }
            else if (methodInfo.parameterTypes[j] == String.class) {
                Generator.println(out, "    const char *pointer" + j +
                                       " = p" + j + " == NULL ? NULL : e->GetStringUTFChars(p" + j + ", NULL);");
            }
            else if (methodInfo.parameterTypes[j].isArray()) {
                final Class<?> t = methodInfo.parameterTypes[j].getComponentType();
                String s = t.getName();
                s = Character.toUpperCase(s.charAt(0)) + s.substring(1);
                Generator.println(out, "    " + getJNITypeName(t) + " *pointer" + j +
                                       " = p" + j + " == NULL ? NULL : e->Get" + s + "ArrayElements(p" + j + ", NULL);");
            }
            else if (Buffer.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                Generator.println(out, "    LockJBuffer lock_JBuffer" + j + "(e, p" + j + ");");
                Generator.println(out, "    " + typeName + " pointer" + j + " = (" + typeName + ")lock_JBuffer" + j + ".dataPtr;");
            }
            else if (methodInfo.parameterTypes[j].isPrimitive()) {
                hasPointer[j] = false;
            }
            else if (ExtendedPrimitive.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                hasPointer[j] = false;
            }
            else if (Object.class == methodInfo.parameterTypes[j]) {
                Generator.println(out, "    ni::Var var_" + j + " = niJVM_GetVar(e,p" + j + ");");
                hasPointer[j] = false;
            }
            else if (methodInfo.staticCall != null) {
            }
            else {
                genAndLogFatalTypeError("Method", methodInfo.method, methodInfo.parameterTypes[j].getCanonicalName());
            }
        }
    }

    private String doReturnBefore(final Class<?> cls, final MethodInformation methodInfo)
    {
        String returnVariable = "";
        if (methodInfo.returnType == void.class) {
            final String typeName = getCPPTypeName(cls);
            if (methodInfo.allocator || methodInfo.arrayAllocator) {
                Generator.println(out, "    if (!e->IsSameObject(e->GetObjectClass(o), ni_getClass(e, " +
                                       jclasses.getIndex(cls) + "))) {");
                Generator.println(out, "        return;");
                Generator.println(out, "    }");
                Generator.println(out, "    " + typeName + " rpointer;");
                returnVariable = "rpointer = ";
            }
        }
        else {
            final String[] typeName = getAnnotatedCPPTypeName(methodInfo.annotations, methodInfo.returnType);
            if (methodInfo.valueSetter || methodInfo.memberSetter) {
                Generator.println(out, "    jobject r = o;");
            }
            else if (methodInfo.returnType == String.class) {
                Generator.println(out, "    jstring r = NULL;");
                returnVariable = "r = niJVM_NewString(e,";
            }
            else if (ExtendedPrimitive.class.isAssignableFrom(methodInfo.returnType)) {
                Generator.println(out, "    jobject r = NULL;");
                returnVariable = "r = niJVM_New" + MethodInformation.getExtendedPrimitiveTypeName(methodInfo.returnType) + "(e,";
            }
            else if (methodInfo.bufferGetter) {
                Generator.println(out, "    jobject r = NULL;");
                Generator.println(out, "    char *rpointer;");
                returnVariable = "rpointer = ";
            }
            else if (RefCountedPointer.class.isAssignableFrom(methodInfo.returnType)) {
                final Class<?> returnBy = getBy(methodInfo.annotations);
                Generator.println(out, "    jobject r = NULL;");
                if (returnBy == ByVal.class) {
                    Generator.println(out, "    " + typeName[0] + "* rpointer" + typeName[1] + ";");
                    if (methodInfo.valueGetter || methodInfo.memberGetter) {
                        returnVariable = "rpointer = &";
                    }
                    else {
                        returnVariable = "rpointer = new " + typeName[0] + typeName[1] + "(";
                    }
                }
                else if (returnBy == BySmartPtr.class) {
                    final String valueTypeName = typeName[0];
                    Generator.println(out, "    ni::Ptr<" + valueTypeName + "> rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = ";
                }
                else if (returnBy == ByRef.class) {
                    final String valueTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + valueTypeName + "* rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = &";
                }
                else if (returnBy == ByPtrPtr.class) {
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + pointerTypeName + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = *";
                }
                else if (returnBy == ByPtrRef.class) {
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + pointerTypeName + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = ";
                }
                else { // default ByPtr
                    Generator.println(out, "    " + typeName[0] + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = ";
                }
            }
            else if (Pointer.class.isAssignableFrom(methodInfo.returnType)) {
                final Class<?> returnBy = getBy(methodInfo.annotations);
                Generator.println(out, "    jobject r = NULL;");
                if (returnBy == ByVal.class) {
                    Generator.println(out, "    " + typeName[0] + "* rpointer" + typeName[1] + ";");
                    if (methodInfo.valueGetter || methodInfo.memberGetter) {
                        returnVariable = "rpointer = &";
                    }
                    else {
                        returnVariable = "rpointer = new " + typeName[0] + typeName[1] + "(";
                    }
                }
                else if (returnBy == ByRef.class) {
                    final String valueTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + valueTypeName + "* rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = &";
                }
                else if (returnBy == ByPtrPtr.class) {
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + pointerTypeName + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = *";
                }
                else if (returnBy == ByPtrRef.class) {
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, "    " + pointerTypeName + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = ";
                }
                else { // default ByPtr
                    Generator.println(out, "    " + typeName[0] + " rpointer" + typeName[1] + ";");
                    returnVariable = "rpointer = ";
                }
            }
            else if (methodInfo.returnType.isPrimitive()) {
                Generator.println(out, "    " + getJNITypeName(methodInfo.returnType) + " r = 0;");
                returnVariable = typeName[0] + " rvalue" + typeName[1] + " = ";
            }
            else if (Object.class == methodInfo.returnType) {
                Generator.println(out, "    " + getJNITypeName(methodInfo.returnType) + " r = 0;");
                returnVariable = typeName[0] + " rvalue = ";
            }
            else {
                genAndLogFatalTypeError("Method", methodInfo.method, methodInfo.returnType.getCanonicalName());
            }
        }
        return returnVariable;
    }

    private boolean doMainOperation(final Class<?> cls, final MethodInformation methodInfo,
            final String returnVariable, final boolean[] hasPointer)
    {
        boolean needsCatchBlock = false;
        final String typeName = getCPPTypeName(cls);
        String valueTypeName = typeName.substring(0, typeName.length() - 1);
        String suffix = ")";
        final String retPrefix = methodInfo.retPrefix == null ? "" : methodInfo.retPrefix;
        boolean overridedCall = false;
        if (methodInfo.deallocator) {
            Generator.println(out, "    void *allocatedAddress = jlong_to_ptr(p0);");
            Generator.println(out, "    void (*deallocatorAddress)(void *) = (void(*)(void*))jlong_to_ptr(p1);");
            Generator.println(out, "    if (deallocatorAddress != NULL && allocatedAddress != NULL) {");
            Generator.println(out, "        (*deallocatorAddress)(allocatedAddress);");
            Generator.println(out, "    }");
            return false; // nothing else should be appended here for deallocator
        }
        else if (methodInfo.valueGetter || methodInfo.valueSetter ||
                 methodInfo.memberGetter || methodInfo.memberSetter) {
            Generator.print(out, "    " + returnVariable);
            if ((methodInfo.valueSetter || methodInfo.memberSetter) && (String.class ==
                methodInfo.parameterTypes[methodInfo.parameterTypes.length - 1])) {
                Generator.print(out, "strcpy(");
                suffix = ")";
            }
            else {
                suffix = "";
            }
            if (Modifier.isStatic(methodInfo.modifiers)) {
                Generator.print(out, retPrefix + methodInfo.memberName);
            }
            else if (methodInfo.memberGetter || methodInfo.memberSetter) {
                Generator.print(out, "pointer->" + retPrefix + methodInfo.memberName);
            }
            else { // methodInfo.valueGetter || methodInfo.valueSetter
                Generator.print(out, "*pointer");
            }
        }
        else if (methodInfo.bufferGetter) {
            Generator.println(out, "    " + returnVariable + "pointer;");
            Generator.print(out, "    jlong capacity = ");
            suffix = "";
        }
        else { // function call
            Generator.println(out, "    NI_JVM_CPP_TRY {");
            needsCatchBlock = true;
            Generator.print(out, "        " + returnVariable);
            Generator.print(out, retPrefix);
            if (FunctionPointer.class.isAssignableFrom(cls)) {
                Generator.print(out, "(*pointer)(");
            }
            else if (methodInfo.allocator) {
                Generator.print(out, "new " + valueTypeName + (methodInfo.arrayAllocator ? "[" : "("));
                suffix = methodInfo.arrayAllocator ? "]" : ")";
            }
            else if (Modifier.isStatic(methodInfo.modifiers)) {
                if ((methodInfo.staticCall != null) && (methodInfo.staticCall.length() > 0)) {
                    Generator.print(out, methodInfo.staticCall);
                    overridedCall = true;
                }
                else if ((methodInfo.staticName != null) && (methodInfo.staticName.length() > 0)) {
                    Generator.print(out, methodInfo.staticName + "(");
                }
                else {
                    if (valueTypeName.length() > 0) {
                        valueTypeName += "::";
                    }
                    Generator.print(out, valueTypeName + methodInfo.memberName + "(");
                }
            }
            else {
                Generator.print(out, "pointer->" + methodInfo.memberName + "(");
            }
        }

        if (!overridedCall) {
            for (int j = 0; j < methodInfo.parameterTypes.length; j++) {
                if ((methodInfo.memberSetter || methodInfo.valueSetter) &&
                    (j >= methodInfo.parameterTypes.length - 1)) {
                    if (methodInfo.memberNameSuffix != null) {
                        Generator.print(out, methodInfo.memberNameSuffix);
                    }
                    if (String.class == methodInfo.parameterTypes[methodInfo.parameterTypes.length - 1]) {
                        Generator.print(out, ", "); // for strcpy
                    }
                    else {
                        Generator.print(out, " = ");
                    }
                }
                else if (methodInfo.valueGetter || methodInfo.valueSetter ||
                         methodInfo.memberGetter || methodInfo.memberSetter) {
                    // print array indices to access array members
                    Generator.print(out, "[p" + j + "]");
                    continue;
                }

                String cast = getCast(methodInfo.parameterAnnotations[j]);
                if (((cast == null) || (cast.length() == 0)) && (methodInfo.pairedMethod != null) &&
                    (methodInfo.valueSetter || methodInfo.memberSetter)) {
                    cast = getCast(methodInfo.pairedMethod.getAnnotations());
                }
                if (("(void*)".equals(cast) || "(void *)".equals(cast)) &&
                    (methodInfo.parameterTypes[j] == long.class)) {
                    Generator.print(out, "jlong_to_ptr(p" + j + ")");
                }
                else if (ExtendedPrimitive.class.isAssignableFrom(methodInfo.parameterTypes[j])) {
                    Generator.print(out, cast + "niJVM_Get" + MethodInformation.getExtendedPrimitiveTypeName(methodInfo.parameterTypes[j]) + "(e,p" + j + ")");
                }
                else if (Object.class == methodInfo.parameterTypes[j]) {
                    Generator.print(out, cast + "var_" + j);
                }
                else if (hasPointer[j]) {
                    Class<?> passBy = getBy(methodInfo.parameterAnnotations[j]);
                    if ((passBy == null) && (methodInfo.pairedMethod != null) &&
                        (methodInfo.valueSetter || methodInfo.memberSetter)) {
                        passBy = getBy(methodInfo.pairedMethod.getAnnotations());
                    }
                    if ((passBy == ByVal.class) || (passBy == ByRef.class)) {
                        // XXX: add check for null pointer here somehow
                        Generator.print(out, "*" + cast + "pointer" + j);
                    }
                    else if (passBy == ByPtrPtr.class) {
                        Generator.print(out, cast + "&pointer" + j);
                    }
                    else if (passBy == BySmartPtr.class) { // BySmartPtr.class
                        Generator.print(out, cast + "pointer" + j + ".ptr()");
                    }
                    else { // ByPtr.class || ByPtrRef.class
                        Generator.print(out, cast + "pointer" + j);
                    }
                }
                else {
                    Generator.print(out, cast + "p" + j);
                }
                if (j < methodInfo.parameterTypes.length - 1) {
                    Generator.print(out, ", ");
                }
            }

            if (!methodInfo.memberSetter && !methodInfo.valueSetter && (methodInfo.memberNameSuffix != null)) {
                Generator.print(out, methodInfo.memberNameSuffix);
            }

            final Class<?> returnBy = getBy(methodInfo.annotations);
            if ((returnBy == ByVal.class) && !methodInfo.valueGetter &&
                !methodInfo.memberGetter && !methodInfo.bufferGetter) {
                Generator.print(out, ")");
            }
            Generator.print(out, suffix);
        }

        if (methodInfo.retSuffix != null) {
            Generator.print(out, methodInfo.retSuffix);
        }
        if (returnVariable.endsWith(",")) {
            Generator.print(out, ")");
        }
        Generator.println(out, ";");
        return needsCatchBlock;
    }

    private void gatherReturn(final Class<?> cls, final MethodInformation methodInfo)
    {
        if (methodInfo.returnType == void.class) {
            if (methodInfo.allocator || methodInfo.arrayAllocator) {
                jclasses.register(cls);
                jclasses.register(Pointer.class);
            }
        }
        else {
            if (methodInfo.valueSetter || methodInfo.memberSetter) {
                // nothing
            }
            else if (RefCountedPointer.class.isAssignableFrom(methodInfo.returnType)) {
                jclasses.register(methodInfo.returnType);
            }
            else if (Pointer.class.isAssignableFrom(methodInfo.returnType)) {
                final Class<?> returnBy = getBy(methodInfo.annotations);
                jclasses.register(methodInfo.returnType);
                if ((returnBy == ByVal.class) && !methodInfo.valueGetter &&
                    !methodInfo.memberGetter && !methodInfo.bufferGetter) {
                    jclasses.register(Pointer.class);
                }
            }
        }
    }

    private void doReturnAfter(final Class<?> cls, final MethodInformation methodInfo, final String returnVariable, final boolean needsCatchBlock)
    {
        if (returnVariable.endsWith(",")) {
            /* The statement is already closed... */
            return;
        }

        final String indent = needsCatchBlock ? "        " : "    ";
        if (methodInfo.returnType == void.class) {
            if (methodInfo.allocator || methodInfo.arrayAllocator) {
                Generator.println(out, indent + "jvalue args[2];");
                Generator.println(out, indent + "args[0].j = ptr_to_jlong(rpointer);");
                Generator.print(out, indent + "args[1].j = ptr_to_jlong(&ni_" + mangle(cls.getName()));
                if (methodInfo.arrayAllocator) {
                    Generator.println(out, "_deallocateArray);");
                    arrayDeallocators.register(cls);
                }
                else {
                    Generator.println(out, "_deallocate);");
                    deallocators.register(cls);
                }
                Generator.println(out, indent + "e->CallNonvirtualVoidMethodA(o, ni_getClass(e, " + jclasses.getIndex(Pointer.class) + "), _JVM->initMethodID, args);");
            }
        }
        else {
            if (methodInfo.valueSetter || methodInfo.memberSetter) {
                // nothing
            }
            else if (methodInfo.returnType == String.class) {
                Generator.println(out, indent + "if (rpointer != NULL) {");
                Generator.println(out, indent + "    r = e->NewStringUTF(rpointer);");
                Generator.println(out, indent + "}");
            }
            else if (methodInfo.bufferGetter) {
                Generator.println(out, indent + "if (rpointer != NULL) {");
                Generator.println(out, indent + "    r = e->NewDirectByteBuffer(rpointer, capacity);");
                Generator.println(out, indent + "}");
            }
            else if (RefCountedPointer.class.isAssignableFrom(methodInfo.returnType)) {
                final Class<?> returnBy = getBy(methodInfo.annotations);
                if (!Modifier.isStatic(methodInfo.modifiers) && (cls == methodInfo.returnType) && (returnBy != ByVal.class)) {
                    Generator.println(out, indent + "if (rpointer == pointer) {");
                    Generator.println(out, indent + "    r = o;");
                    Generator.println(out, indent + "} else if (niIsOK(rpointer)) {");
                }
                else {
                    Generator.println(out, indent + "if (niIsOK(rpointer)) {");
                }
                String typeName = getAnnotatedCPPTypeName(methodInfo.annotations, methodInfo.returnType)[0];
                typeName = getBaseTypeName(typeName);
                Generator.println(out, indent + "    r = niJVM_IUnknownNew(e, ni_getClass(e, " + jclasses.getIndex(methodInfo.returnType) + "), &niGetInterfaceUUID(" + typeName + "), rpointer);");
                Generator.println(out, indent + "}");
            }
            else if (Pointer.class.isAssignableFrom(methodInfo.returnType)) {
                final Class<?> returnBy = getBy(methodInfo.annotations);
                if (!Modifier.isStatic(methodInfo.modifiers) &&
                    (cls == methodInfo.returnType) && (returnBy != ByVal.class)) {
                    Generator.println(out, indent + "if (rpointer == pointer) {");
                    Generator.println(out, indent + "    r = o;");
                    Generator.println(out, indent + "} else if (rpointer != NULL) {");
                }
                else {
                    Generator.println(out, indent + "if (rpointer != NULL) {");
                }
                Generator.println(out, indent + "    r = e->AllocObject(ni_getClass(e, " +
                                       jclasses.getIndex(methodInfo.returnType) + "));");
                if ((returnBy == ByVal.class) && !methodInfo.valueGetter &&
                    !methodInfo.memberGetter && !methodInfo.bufferGetter) {
                    Generator.println(out, indent + "    jvalue args[2];");
                    Generator.println(out, indent + "    args[0].j = ptr_to_jlong(rpointer);");
                    Generator.println(out, indent + "    args[1].j = ptr_to_jlong(&ni_" + mangle(methodInfo.returnType.getName()) + "_deallocate);");
                    Generator.println(out, indent + "    e->CallNonvirtualVoidMethodA(r, ni_getClass(e, " + jclasses.getIndex(Pointer.class) + "), _JVM->initMethodID, args);");
                    deallocators.register(methodInfo.returnType);
                }
                else {
                    Generator.println(out, indent + "    e->SetLongField(r, _JVM->addressFieldID, ptr_to_jlong(rpointer));");
                }
                Generator.println(out, indent + "}");
            }
            else if (methodInfo.returnType.isPrimitive()) {
                Generator.println(out, indent + "r = (" + getJNITypeName(methodInfo.returnType) + ")rvalue;");
            }
            else if (Object.class == methodInfo.returnType) {
                Generator.println(out, indent + "r = niJVM_NewVar(e,rvalue);");
            }
        }
    }

    private void doParametersAfter(final MethodInformation methodInfo, final boolean needsCatchBlock, final boolean[] mayChange)
    {
        final String indent = needsCatchBlock ? "    " : "";
        for (int j = 0; j < methodInfo.parameterTypes.length; j++) {
            if (mayChange[j] && !methodInfo.valueSetter && !methodInfo.memberSetter) {
                if (methodInfo.refCounted) {
                    Generator.println(out, indent + "    if (p" + j + " != NULL) e->SetLongField(p" + j + ", _JVM->refCountedObjPtrFieldID, ptr_to_jlong(pointer" + j + "));");
                }
                else {
                    Generator.println(out, indent + "    if (p" + j + " != NULL) e->SetLongField(p" + j + ", _JVM->addressFieldID, ptr_to_jlong(pointer" + j + "));");
                }
            }
        }

        if (needsCatchBlock) {
            Generator.println(out, "    } NI_JVM_CPP_CATCH_AND_HANDLE_EXCEPTION(e);");
        }

        for (int j = 0; j < methodInfo.parameterTypes.length; j++) {
            if (methodInfo.parameterTypes[j] == String.class) {
                Generator.println(out, "    if (p" + j + " != NULL) e->ReleaseStringUTFChars(p" + j + ", pointer" + j + ");");
            }
            else if (methodInfo.parameterTypes[j].isArray()) {
                String s = methodInfo.parameterTypes[j].getComponentType().getName();
                s = Character.toUpperCase(s.charAt(0)) + s.substring(1);
                Generator.println(out, "    if (p" + j + " != NULL) e->Release" + s + "ArrayElements(p" + j + ", pointer" + j + ", 0);");
            }
        }
    }

    private Method doFunctionDefinitions(final Class<?> cls, final boolean[] callbackAllocators)
    {
        if (!FunctionPointer.class.isAssignableFrom(cls)) {
            return null;
        }

        final Method[] methods = Builder.getSortedDeclaredMethods(cls);
        final Convention convention = cls.getAnnotation(Convention.class);
        final String callingConvention = convention == null ? "" : convention.value() + " ";

        Method functionMethod = null;
        for (int i = 0; i < methods.length; i++) {
            final String methodName = methods[i].getName();
            final int modifiers = methods[i].getModifiers();
            final Class<?>[] parameterTypes = methods[i].getParameterTypes();
            final Class<?> returnType = methods[i].getReturnType();
            if (!Modifier.isNative(modifiers) || Modifier.isStatic(modifiers)) {
                continue;
            }
            if (methodName.startsWith("allocate") && (returnType == void.class) && (parameterTypes.length == 0)) {
                // found a callback allocator method
                callbackAllocators[i] = true;
            }
            else if (methodName.startsWith("call")) {
                // found a function caller method
                functionMethod = methods[i];
            }
        }
        if (functionMethod != null) {
            final Class<?> returnType = functionMethod.getReturnType();
            final Class<?>[] parameterTypes = functionMethod.getParameterTypes();
            final Annotation[] annotations = functionMethod.getAnnotations();
            final Annotation[][] parameterAnnotations = functionMethod.getParameterAnnotations();
            String[] typeName = getAnnotatedCPPTypeName(annotations, returnType);
            String s = "typedef " + typeName[0] + typeName[1] + " (" + callingConvention +
                       "ni_" + mangle(cls.getName()) + ")(";
            for (int j = 0; j < parameterTypes.length; j++) {
                typeName = getAnnotatedCPPTypeName(parameterAnnotations[j], parameterTypes[j]);
                s += typeName[0] + typeName[1] + " p" + j;
                if (j < parameterTypes.length - 1) {
                    s += ", ";
                }
            }
            functionDefinitions.register(s + ");");
        }
        return functionMethod;
    }

    private void gatherCallback(final Method callbackMethod)
    {
        final Class<?>[] callbackParameterTypes = callbackMethod.getParameterTypes();
        if (callbackParameterTypes.length > 0) {
            for (int j = 0; j < callbackParameterTypes.length; j++) {
                if (RefCountedPointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                    jclasses.getIndex(callbackParameterTypes[j]);
                }
                else if (Pointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                    jclasses.getIndex(callbackParameterTypes[j]);
                }
            }
        }
    }

    private void doCallback(final Class<?> cls, final Method callbackMethod, final String callbackName)
    {
        final Class<?> callbackReturnType = callbackMethod.getReturnType();
        final Class<?>[] callbackParameterTypes = callbackMethod.getParameterTypes();
        final Annotation[] callbackAnnotations = callbackMethod.getAnnotations();
        final Annotation[][] callbackParameterAnnotations = callbackMethod.getParameterAnnotations();
        final Convention convention = cls.getAnnotation(Convention.class);
        final String callingConvention = convention == null ? "" : convention.value() + " ";

        functionPointers.register("ni_" + callbackName + "_instance");
        Generator.println(out, "static jmethodID ni_" + callbackName + "_callMethodID = NULL;");
        String[] typeName = getAnnotatedCPPTypeName(callbackAnnotations, callbackReturnType);
        Generator.print(out, "static " + typeName[0] + typeName[1] + " " + callingConvention + "ni_" + callbackName + "_callback(");
        for (int j = 0; j < callbackParameterTypes.length; j++) {
            typeName = getAnnotatedCPPTypeName(callbackParameterAnnotations[j], callbackParameterTypes[j]);
            Generator.print(out, typeName[0] + typeName[1] + " p" + j);
            if (j < callbackParameterTypes.length - 1) {
                Generator.print(out, ", ");
            }
        }
        Generator.println(out, ") {");
        String returnVariable = "";
        if (callbackReturnType != void.class) {
            Generator.println(out, "    " + getJNITypeName(callbackReturnType) + " r = 0;");
            returnVariable = "r";
        }
        Generator.println(out, "    JNIEnv* e = niJVM_GetCurrentEnv();");
        Generator.println(out, "    niAssert(e != NULL);");
        if (callbackParameterTypes.length > 0) {
            Generator.println(out, "    jvalue args[" + callbackParameterTypes.length + "];");
            for (int j = 0; j < callbackParameterTypes.length; j++) {
                if (RefCountedPointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                    Generator.println(out, "    jobject o" + j + " = NULL;");
                    Generator.print(out, "    " + typeName[j] + " pointer" + j + " = (" + typeName[j] + ")p" + j + ";");
                    Generator.println(out, "    if (pointer" + j + " != NULL) { ");
                    Generator.println(out, "        o" + j + " = niJVM_IUnknownNew(e, ni_getClass(e, " + jclasses.getIndex(callbackParameterTypes[j]) + "), &niGetInterfaceUUID(" + typeName[j] + "), pointer" + j + ");");
                    Generator.println(out, "    }");
                    Generator.println(out, "    args[" + j + "].l = o" + j + ";");
                }
                else if (Pointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                    final Class<?> passBy = getBy(callbackParameterAnnotations[j]);
                    Generator.println(out, "    jobject o" + j + " = NULL;");
                    Generator.print(out, "    void *pointer" + j + " = (void*)");
                    if ((passBy == ByVal.class) || (passBy == ByRef.class)) {
                        Generator.println(out, "&p" + j + ";");
                    }
                    else if (passBy == ByPtrPtr.class) {
                        // XXX: add check for null pointer here somehow
                        Generator.println(out, "*p" + j + ";");
                    }
                    else if (passBy == BySmartPtr.class) {
                        // XXX: add check for null pointer here somehow
                        Generator.println(out, "p" + j + ".ptr();");
                    }
                    else { // ByPtr.class || ByPtrRef.class
                        Generator.println(out, "p" + j + ";");
                    }
                    final String s = "    o" + j + " = e->AllocObject(ni_getClass(e, " +
                                     jclasses.getIndex(callbackParameterTypes[j]) + "));";
                    if ((passBy == ByPtrPtr.class) || (passBy == ByPtrRef.class)) {
                        Generator.println(out, s);
                    }
                    else {
                        Generator.println(out, "    if (pointer" + j + " != NULL) { ");
                        Generator.println(out, "    " + s);
                        Generator.println(out, "    }");
                    }
                    Generator.println(out, "    if (o" + j + " != NULL) { ");
                    Generator.println(out, "        e->SetLongField(o" + j + ", _JVM->addressFieldID, ptr_to_jlong(pointer" + j + "));");
                    Generator.println(out, "    }");
                    Generator.println(out, "    args[" + j + "].l = o" + j + ";");
                }
                else if (callbackParameterTypes[j] == String.class) {
                    Generator.println(out, "    jstring o" + j + " = p" + j + " == NULL ? NULL : e->NewStringUTF(p" + j + ");");
                    Generator.println(out, "    args[" + j + "].l = o" + j + ";");
                }
                else if (callbackParameterTypes[j].isPrimitive()) {
                    Generator.println(out, "    args[" + j + "]." +
                                           getSignature(callbackParameterTypes[j]).toLowerCase() + " = p" + j + ";");
                }
                else {
                    genAndLogFatalTypeError("Callback", callbackMethod, callbackParameterTypes[j].getCanonicalName());
                }
            }
        }

        if (returnVariable.length() > 0) {
            returnVariable += " = ";
        }
        String s = "Object";
        if (callbackReturnType.isPrimitive()) {
            s = callbackReturnType.getName();
            s = Character.toUpperCase(s.charAt(0)) + s.substring(1);
        }
        Generator.println(out, "    " + returnVariable + "e->Call" + s + "MethodA(ni_" + callbackName +
                               "_instance, ni_" + callbackName + "_callMethodID, " +
                               (callbackParameterTypes.length == 0 ? "NULL);" : "args);"));

        for (int j = 0; j < callbackParameterTypes.length; j++) {
            if (RefCountedPointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                final Class<?> passBy = getBy(callbackParameterAnnotations[j]);
                if ((passBy == ByPtrPtr.class) || (passBy == ByPtrRef.class)) {
                    Generator.println(out, "    pointer" + j + " = jlong_to_ptr(e->GetLongField(o" + j + ", _JVM->refCountedObjPtrFieldID);");
                    if (passBy == ByPtrPtr.class) {
                        Generator.print(out, "    *p" + j);
                    }
                    else {
                        Generator.print(out, "    p" + j);
                    }
                    typeName = getAnnotatedCPPTypeName(callbackParameterAnnotations[j], callbackParameterTypes[j]);
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, " = (" + pointerTypeName + typeName[1] + ")pointer" + j + ";");
                }
            }
            else if (Pointer.class.isAssignableFrom(callbackParameterTypes[j])) {
                final Class<?> passBy = getBy(callbackParameterAnnotations[j]);
                if ((passBy == ByPtrPtr.class) || (passBy == ByPtrRef.class)) {
                    Generator.println(out, "    pointer" + j + " = jlong_to_ptr(e->GetLongField(o" + j + ", _JVM->addressFieldID);");
                    if (passBy == ByPtrPtr.class) {
                        Generator.print(out, "    *p" + j);
                    }
                    else {
                        Generator.print(out, "    p" + j);
                    }
                    typeName = getAnnotatedCPPTypeName(callbackParameterAnnotations[j], callbackParameterTypes[j]);
                    final String pointerTypeName = typeName[0].substring(0, typeName[0].length() - 1);
                    Generator.println(out, " = (" + pointerTypeName + typeName[1] + ")pointer" + j + ";");
                }
            }
            if (!callbackParameterTypes[j].isPrimitive()) {
                Generator.println(out, "    e->Delete(o" + j + ");");
            }
        }

        if (callbackReturnType != void.class) {
            if (RefCountedPointer.class.isAssignableFrom(callbackReturnType)) {
                typeName = getAnnotatedCPPTypeName(callbackAnnotations, callbackReturnType);
                Generator.println(out, "    return r == NULL ? NULL : (" + typeName[0] + typeName[1] +
                                       ")jlong_to_ptr(e->GetLongField(r, _JVM->refCountedObjPtrFieldID));");
            }
            else if (Pointer.class.isAssignableFrom(callbackReturnType)) {
                typeName = getAnnotatedCPPTypeName(callbackAnnotations, callbackReturnType);
                Generator.println(out, "    return r == NULL ? NULL : (" + typeName[0] + typeName[1] +
                                       ")jlong_to_ptr(e->GetLongField(r, _JVM->addressFieldID));");
            }
            else if (Buffer.class.isAssignableFrom(callbackReturnType)) {
                typeName = getAnnotatedCPPTypeName(callbackAnnotations, callbackReturnType);
                Generator.println(out, "    return r == NULL ? NULL : (" + typeName[0] + typeName[1] +
                                       ")e->GetDirectBufferAddress(r);");
            }
            else if (callbackReturnType.isPrimitive()) {
                Generator.println(out, "    return " + getCast(callbackAnnotations) + "r;");
            }
            else {
                genAndLogFatalTypeError("Callback", callbackMethod, callbackReturnType.getCanonicalName());
            }
        }
        Generator.println(out, "}");
    }

    private void doCallbackAllocator(final Class<?> cls, final Method callbackMethod, final String callbackName)
    {
        // XXX: Here, we should actually allocate new trampolines on the heap somehow...
        // For now it just bumps out from the global variable the last object that called this method
        Generator.println(out, "    e->DeleteGlobalRef(ni_" + callbackName + "_instance);");
        Generator.println(out, "    ni_" + callbackName + "_instance = e->NewGlobalRef(o);");
        Generator.println(out, "    if (ni_" + callbackName + "_instance == NULL) {");
        Generator.println(out, "        fprintf(stderr, \"Error creating global reference of " + cls.getName() + " instance for callback.\");");
        Generator.println(out, "        return;");
        Generator.println(out, "    }");
        Generator.println(out, "    ni_" + callbackName + "_callMethodID = e->GetMethodID(e->GetObjectClass(o), \"" +
                               callbackMethod.getName() + "\", \"(" + getSignature(callbackMethod.getParameterTypes()) + ")" +
                               getSignature(callbackMethod.getReturnType()) + "\");");
        Generator.println(out, "    if (ni_" + callbackName + "_callMethodID == NULL) {");
        Generator.println(out, "        fprintf(stderr, \"Error getting method ID of function caller \\\"" + callbackMethod + "\\\" for callback.\");");
        Generator.println(out, "        return;");
        Generator.println(out, "    }");
        Generator.println(out, "    e->SetLongField(o, _JVM->addressFieldID, ptr_to_jlong(&ni_" + callbackName + "_callback));");
        Generator.println(out, "}");
    }

    public boolean checkPlatform(final Platform platform) {
        if (platform == null) {
            return true;
        }
        final String platformName = properties.getProperty("platform.name");
        final String[][] names = { platform.value(), platform.not() };
        final boolean[] matches = { false, false };
        for (int i = 0; i < names.length; i++) {
            for (final String s : names[i]) {
                if (platformName.startsWith(s)) {
                    matches[i] = true;
                    break;
                }
            }
        }
        if (((names[0].length == 0) || matches[0]) && ((names[1].length == 0) || !matches[1])) {
            return true;
        }
        return false;
    }

    public static String getCast(final Annotation... annotations) {
        for (final Annotation a : annotations) {
            if (a instanceof Cast) {
                return "(" + ((Cast)a).value() + ")";
            }
        }
        return "";
    }

    public static Class<? extends Annotation> getBy(final Annotation... annotations) {
        Annotation byAnnotation = null;
        for (final Annotation a : annotations) {
            if ((a instanceof ByPtr) || (a instanceof ByPtrPtr) || (a instanceof ByPtrRef) ||
                (a instanceof ByRef) || (a instanceof ByVal) ||
                (a instanceof BySmartPtr))
            {
                if (byAnnotation != null) {
                    logger.log(Level.WARNING, "\"By\" annotation \"" + byAnnotation +
                                              "\" already found. Ignoring superfluous annotation \"" + a + "\".");
                }
                else {
                    byAnnotation = a;
                }
            }
        }
        return byAnnotation == null ? null : byAnnotation.annotationType();
    }

    public static String getBaseTypeName(String typeName) {
        if (typeName.startsWith("const "))
            typeName = typeName.substring(6, typeName.length());
        if (typeName.endsWith("*") || typeName.endsWith("&"))
            typeName = typeName.substring(0, typeName.length() - 1);
        return typeName;
    }

    public static String[] getAnnotatedCPPTypeName(final Annotation[] annotations, final Class<?> type)
    {
        String name = getCPPTypeName(type);
        for (final Annotation a : annotations) {
            if (a instanceof Const) {
                name = "const " + name;
                break;
            }
        }
        for (final Annotation a : annotations) {
            if (a instanceof Cast) {
                name = ((Cast)a).value();
                final int indexOfPtr = name.indexOf("Ptr<");
                if (indexOfPtr >= 0) {
                    final int lastIndexOfPtr = name.lastIndexOf(">");
                    name = name.substring(indexOfPtr+4,lastIndexOfPtr);
                }
            }
        }
        String prefix = name, suffix = "";
        final int parenthesis = name.indexOf(')');
        if (parenthesis > 0) {
            prefix = name.substring(0, parenthesis).trim();
            suffix = name.substring(parenthesis).trim();
        }

        final Class<?> by = getBy(annotations);
        if (by == BySmartPtr.class) {
        	if (prefix.endsWith("&") || prefix.endsWith("*"))
                prefix = prefix.substring(0, prefix.length() - 1);
        }
        else if (by == ByVal.class) {
            if (prefix.endsWith("&") || prefix.endsWith("*"))
                prefix = prefix.substring(0, prefix.length() - 1);
        }
        else if (by == ByRef.class) {
            if (prefix.endsWith("*"))
                prefix = prefix.substring(0, prefix.length() - 1) + "&";
            else if (!prefix.endsWith("&"))
                prefix = prefix + "&";
        }
        else if (by == ByPtrPtr.class) {
            prefix = prefix + "*";
        }
        else if (by == ByPtrRef.class) {
            if (!prefix.endsWith("&"))
                prefix = prefix + "&";
        } // else default to ByPtr.class
        return new String[] { prefix, suffix };
    }

    public static String getCPPTypeName(Class<?> type) {
        if (type == RefCountedPointer.class) {
            return "ni::iUnknown*";
        }
        else if ((type == Buffer.class) || (type == Pointer.class)) {
            return "void*";
        }
        else if ((type == byte[].class) || (type == ByteBuffer.class) || (type == BytePointer.class)) {
            return "signed char*";
        }
        else if ((type == short[].class) || (type == ShortBuffer.class) || (type == ShortPointer.class)) {
            return "short*";
        }
        else if ((type == int[].class) || (type == IntBuffer.class) || (type == IntPointer.class)) {
            return "int*";
        }
        else if ((type == long[].class) || (type == LongBuffer.class) || (type == LongPointer.class)) {
            return "jlong*";
        }
        else if ((type == float[].class) || (type == FloatBuffer.class) || (type == FloatPointer.class)) {
            return "float*";
        }
        else if ((type == double[].class) || (type == DoubleBuffer.class) || (type == DoublePointer.class)) {
            return "double*";
        }
        else if ((type == char[].class) || (type == CharBuffer.class) || (type == CharPointer.class)) {
            return "unsigned short*";
        }
        else if (type == PointerPointer.class) {
            return "void**";
        }
        else if (type == String.class) {
            return "const char*";
        }
        else if (type == byte.class) {
            return "signed char";
        }
        else if (type == char.class) {
            return "unsigned short";
        }
        else if (type == long.class) {
            return "jlong";
        }
        else if (type == boolean.class) {
            return "unsigned char";
        }
        else if (type.isPrimitive()) {
            return type.getName();
        }
        else if (FunctionPointer.class.isAssignableFrom(type)) {
            return "ni_" + mangle(type.getName()) + "*";
        }
        else if (ExtendedPrimitive.class.isAssignableFrom(type)) {
            return MethodInformation.getExtendedPrimitiveNativeType(type);
        }
        else if (Object.class == type) {
            return "ni::Var";
        }
        else {
            String spacedType = "";
            while (type != null) {
                final Namespace namespace = type.getAnnotation(Namespace.class);
                String spaceName = namespace != null ? namespace.value() : "";
                if (RefCountedPointer.class.isAssignableFrom(type)) {
                    final Name name = type.getAnnotation(Name.class);
                    String s;
                    if (name == null) {
                        s = type.getName();
                        s = s.substring(s.lastIndexOf("$") + 1);
                    }
                    else {
                        s = name.value();
                    }
                    if (spaceName.length() == 0) {
                        spaceName = s;
                    }
                    else {
                        spaceName = spaceName + "::" + s;
                    }
                }
                else if (Pointer.class.isAssignableFrom(type)) {
                    final Name name = type.getAnnotation(Name.class);
                    String s;
                    if (name == null) {
                        s = type.getName();
                        s = s.substring(s.lastIndexOf("$") + 1);
                    }
                    else {
                        s = name.value();
                    }
                    if (spaceName.length() == 0) {
                        spaceName = s;
                    }
                    else {
                        spaceName = spaceName + "::" + s;
                    }
                }
                if (spacedType.length() == 0) {
                    spacedType = spaceName;
                }
                else if (spaceName.length() > 0) {
                    spacedType = spaceName + "::" + spacedType;
                }
                type = type.getDeclaringClass();
            }
            return spacedType + "*";
        }
    }

    public static String getJNITypeName(final Class<?> type) {
        if (type == byte.class) {
            return "jbyte";
        }
        else if (type == short.class) {
            return "jshort";
        }
        else if (type == int.class) {
            return "jint";
        }
        else if (type == long.class) {
            return "jlong";
        }
        else if (type == float.class) {
            return "jfloat";
        }
        else if (type == double.class) {
            return "jdouble";
        }
        else if (type == boolean.class) {
            return "jboolean";
        }
        else if (type == char.class) {
            return "jchar";
        }
        else if (type == byte[].class) {
            return "jbyteArray";
        }
        else if (type == short[].class) {
            return "jshortArray";
        }
        else if (type == int[].class) {
            return "jintArray";
        }
        else if (type == long[].class) {
            return "jlongArray";
        }
        else if (type == float[].class) {
            return "jfloatArray";
        }
        else if (type == double[].class) {
            return "jdoubleArray";
        }
        else if (type == boolean[].class) {
            return "jbooleanArray";
        }
        else if (type == char[].class) {
            return "jcharArray";
        }
        else if (type == String.class) {
            return "jstring";
        }
        else if (type == void.class) {
            return "void";
        }
        else {
            return "jobject";
        }
    }

    public static String getSignature(final Class<?>... types) {
        final StringBuilder signature = new StringBuilder(2 * types.length);
        for (int i = 0; i < types.length; i++) {
            signature.append(getSignature(types[i]));
        }
        return signature.toString();
    }

    public static String getSignature(final Class<?> type) {
        if (type == byte.class) {
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
        else if (type == boolean.class) {
            return "Z";
        }
        else if (type == char.class) {
            return "C";
        }
        else if (type == void.class) {
            return "V";
        }
        else if (type.isArray()) {
            return type.getName().replace(".", "/");
        }
        else {
            return "L" + type.getName().replace(".", "/") + ";";
        }
    }

    @SuppressWarnings("fallthrough")
    public static String mangle(final String name) {
        final StringBuilder mangledName = new StringBuilder(name.length());
        for (int i = 0; i < name.length(); i++) {
            final char c = name.charAt(i);
            if (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
                mangledName.append(c);
            }
            else if (c == '_') {
                mangledName.append("_1");
            }
            else if (c == ';') {
                mangledName.append("_2");
            }
            else if (c == '[') {
                mangledName.append("_3");
            }
            else if ((c == '.') || (c == '/')) {
                mangledName.append("_");
            }
            else {
                final String code = Integer.toHexString(c);
                mangledName.append("_0");
                switch (code.length()) {
                case 1:
                    mangledName.append("0");
                case 2:
                    mangledName.append("0");
                case 3:
                    mangledName.append("0");
                default:
                    mangledName.append(code);
                }
            }
        }
        return mangledName.toString();
    }

    public static void println(final PrintWriter printWriter, final String x) {
        printWriter.print(x);
        printWriter.append('\n');
    }

    public static void println(final PrintWriter printWriter) {
        printWriter.append('\n');
    }

    public static void print(final PrintWriter printWriter, final String s) {
        printWriter.print(s);
    }

    public static void print(final PrintWriter printWriter, final char c) {
        printWriter.print(c);
    }

    public static void print(final PrintWriter printWriter, final Object obj) {
        printWriter.print(obj);
    }

    public static void print(final PrintWriter printWriter, final char[] s) {
        printWriter.print(s);
    }
}
