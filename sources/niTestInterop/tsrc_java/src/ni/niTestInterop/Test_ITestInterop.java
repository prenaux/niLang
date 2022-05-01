package ni.niTestInterop;
import junit.framework.*;

import ni.types.*;
import ni.niLang.Lib;

import java.util.ArrayList;
import java.util.Iterator;

public class Test_ITestInterop extends TestCase {

    ITestInterop testInterop;
    ITestInteropObject testObj;

    class NullTestInteropSink implements ITestInteropSinkImpl {
        public void onTestInteropSink_Void() {
        }
        public void onTestInteropSink_Void_I32(int v) {
        }
        public void onTestInteropSink_Void_I64(long v) {
        }
        public void onTestInteropSink_Void_Ptr(long v) {
        }
        public Object onTestInteropSink_Var_Var(Object v) {
            return null;
        }
        public Vec3f onTestInteropSink_Vec3f_String(String aaszStr) {
            return null;
        }
        public Vec3f onTestInteropSink_Vec3f_CObject(ITestInteropObject apObj) {
            apObj.getFlags();
            return null;
        }
        public Vec3f onTestInteropSink_Vec3f_Object(ITestInteropObject apObj) {
            apObj.getFlags();
            return null;
        }
        public ITestInteropObject onTestInteropSink_Object_CObject(ITestInteropObject apObj) {
            apObj.getFlags();
            return null;
        }
        public ITestInteropObject onTestInteropSink_Object_Object(ITestInteropObject apObj) {
            apObj.getFlags();
            return null;
        }
        public ITestInteropObject onTestInteropSink_Object_Object2(int a, float b, ITestInteropObject apObj) {
            apObj.getFlags();
            return null;
        }
        public ITestInteropObject onTestInteropSink_Object_ObjectVec3f(ITestInteropObject apObj, Vec3f v) {
            apObj.getFlags();
            return null;
        }
    };

    class DefaultTestInteropSink implements ITestInteropSinkImpl {
        public int callCount = 0;
        public int intV = 0;
        public long longV = 0;
        public long ptrV = 0;
        public float floatV = 0;
        public Vec3f vec3V = new Vec3f();

        public void onTestInteropSink_Void() {
            ++callCount;
        }
        public void onTestInteropSink_Void_I32(int v) {
            ++callCount;
            intV += v;
        }
        public void onTestInteropSink_Void_I64(long v) {
            ++callCount;
            longV += v;
        }
        public void onTestInteropSink_Void_Ptr(long v) {
            ++callCount;
            ptrV += v;
        }
        public Object onTestInteropSink_Var_Var(Object v) {
            ++callCount;
            return "Foo";
        }
        public Vec3f onTestInteropSink_Vec3f_String(String aaszStr) {
            ++callCount;
            return new Vec3f().set(1,2,3);
        }
        public Vec3f onTestInteropSink_Vec3f_CObject(ITestInteropObject apObj) {
            apObj.getFlags();
            ++callCount;
            return new Vec3f().set(2,3,4);
        }
        public Vec3f onTestInteropSink_Vec3f_Object(ITestInteropObject apObj) {
            apObj.getFlags();
            ++callCount;
            return new Vec3f().set(3,4,5);
        }
        public ITestInteropObject onTestInteropSink_Object_CObject(ITestInteropObject apObj) {
            apObj.getFlags();
            ++callCount;
            return apObj;
        }
        public ITestInteropObject onTestInteropSink_Object_Object(ITestInteropObject apObj) {
            apObj.getFlags();
            ++callCount;
            return apObj;
        }
        public ITestInteropObject onTestInteropSink_Object_Object2(int a, float b, ITestInteropObject apObj) {
            apObj.getFlags();
            ++callCount;
            intV += a;
            floatV += b;
            return apObj;
        }
        public ITestInteropObject onTestInteropSink_Object_ObjectVec3f(ITestInteropObject apObj, Vec3f v) {
            apObj.getFlags();
            ++callCount;
            vec3V.x += v.x;
            vec3V.y += v.y;
            vec3V.z += v.z;
            return apObj;
        }
    }

    @Override
    protected void setUp() throws java.lang.Exception {
        super.setUp();
        testInterop = niTestInterop.createTestInterop();
        assertTrue(testInterop != null);
        testObj = testInterop.createTestInteropObject(10);
        assertTrue(testObj != null);
    }

    @Override
    protected void tearDown() throws java.lang.Exception {
        testObj.dispose();
        testObj = null;
        super.tearDown();
    }

    public void testBase() {
        System.out.println("getModulePath(niLang): " + Lib.getModulePath("niLang"));
        System.out.println("getModulePath(niTestInterop): " + Lib.getModulePath("niTestInterop"));
    }

    public void testUUID() {
        System.out.println("ITestInterop::UUID: " + ITestInterop.UUID);
        System.out.println("ITestInteropObject::UUID: " + ITestInteropObject.UUID);
    }

    public void testString() {
        String foo = "foo";
        testObj.setString(foo);
        String retFoo = testObj.getString();
        assertEquals(foo,retFoo);
        assertFalse(foo == retFoo); // Reference IS NOT the same even if the string is
    }

    public void testACharStrZ() {
        testObj.setACharStrZ("bar");
        assertEquals("bar",testObj.getACharStrZ());
    }

    public void testI8() {
        testObj.setI8((byte)123);
        assertEquals((byte)123,testObj.getI8());
    }

    public void testI16() {
        testObj.setI16((short)12345);
        assertEquals((short)12345,testObj.getI16());
    }

    public void testI32() {
        testObj.setI32(123456789);
        assertEquals(123456789,testObj.getI32());
    }

    public void testI64() {
        testObj.setI64(123456789123456789L);
        assertEquals(123456789123456789L,testObj.getI64());
    }

    public void testF64() {
        testObj.setF64(123456789);
        assertEquals(123456789.0,testObj.getF64());
    }

    public void testVec2f() {
        testObj.setVec2f(new Vec2f().set(123.0f,456.0f));
        assertEquals(new Vec2f().set(123.0f,456.0f),testObj.getVec2f());
    }

    public void testVec3f() {
        testObj.setVec3f(new Vec3f().set(133.0f,456.0f,789.0f));
        assertEquals(new Vec3f().set(133.0f,456.0f,789.0f),testObj.getVec3f());
    }

    public void testVec4f() {
        testObj.setVec4f(new Vec4f().set(144.0f,456.0f,789.0f,100.0f));
        assertEquals(new Vec4f().set(144.0f,456.0f,789.0f,100.0f),testObj.getVec4f());
    }

    public void testVec2l() {
        testObj.setVec2l(new Vec2i().set(123,456));
        assertEquals(new Vec2i().set(123,456),testObj.getVec2l());
    }

    public void testVec3l() {
        testObj.setVec3l(new Vec3i().set(133,456,789));
        assertEquals(new Vec3i().set(133,456,789),testObj.getVec3l());
    }

    public void testVec4l() {
        testObj.setVec4l(new Vec4i().set(144,456,789,100));
        assertEquals(new Vec4i().set(144,456,789,100),testObj.getVec4l());
    }

    public void testMatrixf() {
        testObj.setMatrixf(new Matrixf().set(1,2,3,4,11,12,13,14,21,22,23,24,31,32,33,34));
        assertEquals(new Matrixf().set(1,2,3,4,11,12,13,14,21,22,23,24,31,32,33,34),
                     testObj.getMatrixf());
    }

    public void testVarNull() {
        testObj.setVar(null);
        assertEquals(null,testObj.getVar());
    }

    public void testVarByte() {
        testObj.setVar((byte)123);
        assertEquals((byte)123,testObj.getVar());
    }

    public void testVarShort() {
        testObj.setVar((short)12345);
        assertEquals((short)12345,testObj.getVar());
    }

    public void testVarInt() {
        testObj.setVar(123);
        assertEquals(123,testObj.getVar());
    }

    public void testVarLong() {
        testObj.setVar(1231234512345L);
        assertEquals(1231234512345L,testObj.getVar());
    }

    public void testVarFloat() {
        testObj.setVar(123.0f);
        assertEquals(123.0f,testObj.getVar());
    }

    public void testVarDouble() {
        testObj.setVar(new Double(123.0));
        assertEquals(new Double(123.0),testObj.getVar());
    }

    public void testVarVec2f() {
        testObj.setVar(new Vec2f().set(123.0f,456.0f));
        assertEquals(new Vec2f().set(123.0f,456.0f),testObj.getVar());
    }

    public void testVarVec3f() {
        testObj.setVar(new Vec3f().set(133.0f,456.0f,789.0f));
        assertEquals(new Vec3f().set(133.0f,456.0f,789.0f),testObj.getVar());
    }

    public void testVarVec4f() {
        testObj.setVar(new Vec4f().set(144.0f,456.0f,789.0f,100.0f));
        assertEquals(new Vec4f().set(144.0f,456.0f,789.0f,100.0f),testObj.getVar());
    }

    public void testVarVec2l() {
        testObj.setVar(new Vec2i().set(123,456));
        assertEquals(new Vec2i().set(123,456),testObj.getVar());
    }

    public void testVarVec3l() {
        testObj.setVar(new Vec3i().set(133,456,789));
        assertEquals(new Vec3i().set(133,456,789),testObj.getVar());
    }

    public void testVarVec4l() {
        testObj.setVar(new Vec4i().set(144,456,789,100));
        assertEquals(new Vec4i().set(144,456,789,100),testObj.getVar());
    }

    public void testVarMatrixf() {
        testObj.setVar(new Matrixf().set(1,2,3,4,11,12,13,14,21,22,23,24,31,32,33,34));
        assertEquals(new Matrixf().set(1,2,3,4,11,12,13,14,21,22,23,24,31,32,33,34),testObj.getVar());
    }

    public void testVarString() {
        String foo = "foo";
        testObj.setVar(foo);
        String retFoo = (String)testObj.getVar();
        assertEquals(foo,retFoo);
        assertFalse(foo == retFoo); // Reference IS NOT the same even if the string is
    }

    public void testIUnknown() {
        ITestInteropObject foo = testInterop.createTestInteropObject(10);
        assertEquals(1,foo.getNumRefs());
        testObj.setTestInteropObject(foo);
        assertEquals(2,foo.getNumRefs());
        testObj.setTestInteropObject(null);
        assertEquals(1,foo.getNumRefs());
        foo.dispose();
    }

    public void testVarIUnknown() {
        ITestInteropObject foo = testInterop.createTestInteropObject(10);
        assertEquals(1,foo.getNumRefs());
        testObj.setVar(foo);
        assertEquals(2,foo.getNumRefs());
        IUnknown retFoo = (IUnknown)testObj.getVar();
        assertEquals(foo,retFoo);
        assertEquals(3,retFoo.getNumRefs());
        assertEquals(3,foo.getNumRefs());
        retFoo.dispose();
        assertEquals(2,foo.getNumRefs());
        testObj.setVar(null);
        assertEquals(1,foo.getNumRefs());
        foo.dispose();
    }

    public void testVarIUnknownQuery() {
        ITestInteropObject foo = testInterop.createTestInteropObject(10);
        assertEquals(1,foo.getNumRefs());
        {
            testObj.setVar(foo);
            assertEquals(2,foo.getNumRefs());
        }
        {
            IUnknown retFoo = (IUnknown)testObj.getVar();
            assertEquals(foo,retFoo);
            assertEquals(3,retFoo.getNumRefs());
            assertEquals(3,foo.getNumRefs());
            {
                ITestInteropObject obtFoo = ITestInteropObject.query(retFoo);
                assertSame(foo,obtFoo); // is the same, because the JNI ensures that IUnknown objects are created only when needed
                assertEquals(3,obtFoo.getNumRefs());
                ITestInteropObject obtFoo2 = ITestInteropObject.query(testObj.getVar());
                assertSame(foo,obtFoo2); // is the same, because the JNI ensures that IUnknown objects are created only when needed
                assertEquals(3,obtFoo2.getNumRefs());
            }
            retFoo.dispose();
            assertEquals(2,foo.getNumRefs());
        }
        {
            testObj.setVar(null);
            assertEquals(1,foo.getNumRefs());
        }
        foo.dispose();
    }

    public void testSink() {
        ITestInteropSink sinkNull = ITestInteropSink.impl(new NullTestInteropSink());
        DefaultTestInteropSink sinkImpl = new DefaultTestInteropSink();
        ITestInteropSink sink = ITestInteropSink.impl(sinkImpl);

        {
            ITestInteropObject testObj2 = testInterop.createTestInteropObject(10);
            assertTrue(testObj2 != null);
            testObj.setVar(testObj2);
            testObj2.dispose();
            testObj2 = null;
            System.gc();
        }

        assertEquals(2,testInterop.getNumTestInteropObjects());
        Vec3f vec3 = new Vec3f().set(1,2,3);
        for (int i = 0; i < 10; ++i) {
            ITestInteropObject nullO = testInterop.forEachObject_OnTestInteropSink_Object_ObjectVec3f(sinkNull,vec3);
            assertEquals(null,nullO);
            System.gc();
        }

        for (int i = 0; i < 10; ++i) {
            ITestInteropObject o = testInterop.forEachObject_OnTestInteropSink_Object_ObjectVec3f(sink,vec3);
            // assertSame(testObj2,o);
            assertEquals(testInterop.getNumTestInteropObjects()*(i+1),sinkImpl.callCount);
            System.gc();
        }

        testObj.setVar(null);
        System.out.println("End: " + testInterop);
        final float scale = (float)testInterop.getNumTestInteropObjects()*10.0f;
        assertEquals(new Vec3f().set(
                         vec3.x * scale,
                         vec3.y * scale,
                         vec3.z * scale),
                     sinkImpl.vec3V);
    }
};
