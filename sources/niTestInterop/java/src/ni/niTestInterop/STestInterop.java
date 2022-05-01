package ni.niTestInterop;

import ni.types.*;
import ni.types.annotations.*;

@Platform(include={"stdafx.h"})
@Namespace("ni")
@Name("sTestInterop")
public class STestInterop extends Pointer {
    public native @Unsigned int mnInt();
    public native void mnInt(int v);
    public native float mfFloat();
    public native void mfFloat(float v);
    public native Vec3f mvVec3();
    public native void mvVec3(Vec3f v);
}
