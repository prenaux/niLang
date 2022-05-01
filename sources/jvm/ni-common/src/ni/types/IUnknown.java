package ni.types;

import ni.types.annotations.Name;
import ni.types.annotations.Namespace;
import ni.types.annotations.Platform;
import ni.types.annotations.Static;

@Platform(include = { "stdafx.h" })
@Namespace("ni")
@Name("iUnknown")
public class IUnknown extends RefCountedPointer {
    public static final UUID UUID = new UUID(0xec052841, 0x18a0, 0x46c1, 0xad, 0x8b,
            0xcb, 0xfe, 0x06, 0x57, 0xec, 0x0e);

    @Override
    public void dispose() {
        super.dispose();
    }

    @Name("GetNumRefs")
    public native int getNumRefs();

    @Name("Invalidate")
    public native void invalidate();

    /**
     * Query this interface on the specified object.
     */
    @Static(name = "niJVM_QueryInterface<ni::iUnknown>")
    public static native IUnknown query(Object obj);

    /**
     * Query this interface on the specified IUnknown.
     */
    @Static(name = "niJVM_QueryInterface<ni::iUnknown>")
    public static native IUnknown query(IUnknown obj);

    public boolean equals(final IUnknown other) {
        if ((other == null) || (objPtr == 0)) {
            return true;
        }
        else {
            return objPtr == other.objPtr;
        }
    }
};
