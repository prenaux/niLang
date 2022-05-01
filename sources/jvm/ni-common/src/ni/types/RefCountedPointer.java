package ni.types;

import java.lang.ref.WeakReference;
import java.util.HashMap;

import ni.types.annotations.Opaque;
import ni.types.annotations.Static;

/**
 * @author Pierre Renaux
 */
@Opaque
public class RefCountedPointer {
    protected long objPtr;
    protected long iidPtr;
    private static RefCountedCache _cache = new RefCountedCache();

    protected static void cache(final long iidPtr, final long objPtr, final Object obj) {
        _cache.cache(iidPtr, objPtr, obj);
    }

    protected static Object obtain(final long iidPtr, final long objPtr) {
        return _cache.obtain(iidPtr, objPtr);
    }

    protected RefCountedPointer() {
        objPtr = 0;
        iidPtr = 0;
    }

    protected RefCountedPointer(final long aIidPtr, final long aObjPtr) {
        // System.out.println("RefCountedPointer::constructor["+objPtr+","+iidPtr+"]");
        objPtr = aObjPtr;
        iidPtr = aIidPtr;
    }

    @Override
    protected void finalize() {
        // System.out.println("RefCountedPointer::finalize");
        dispose();
    }

    /**
     * Release the reference of the object
     */
    public void dispose() {
        // System.out.println("RefCountedPointer::dispose["+objPtr+","+iidPtr+"]");
        if (objPtr != 0) {
            _cache.remove(iidPtr, objPtr);
            IUnknownDispose(iidPtr, objPtr);
            objPtr = 0;
            iidPtr = 0;
        }
    }

    /**
     * @return true if the Pointer is OK, and can be used
     */
    public boolean isOK() {
        return objPtr != 0;
    }

    @Static(call = "niJVM_IUnknownDispose(e,p0,p1)")
    private static native int IUnknownDispose(long iidPtr, long objPtr);

    @Override
    public boolean equals(final Object obj) {
        if ((obj == null) && (objPtr == 0)) {
            return true;
        }
        else if (!(obj instanceof RefCountedPointer)) {
            return false;
        }
        else {
            final RefCountedPointer other = (RefCountedPointer)obj;
            return objPtr == other.objPtr;
        }
    }

    @Override
    public int hashCode() {
        return (int)(objPtr + iidPtr);
    }

    @Override
    public String toString() {
        return getClass().getName() + "[objPtr=0x" + Long.toHexString(objPtr) + ", iidPtr=0x" + Long.toHexString(iidPtr) + "]";
    }

    public long getObjPtr() {
        return objPtr;
    }
}

class RefCountedCache {
    class Key {
        public long objPtr;
        public long iidPtr;

        @Override
        public Key clone() {
            final Key k = new Key();
            k.objPtr = objPtr;
            k.iidPtr = iidPtr;
            return k;
        }

        @Override
        public boolean equals(final Object obj) {
            if ((obj == null) && (objPtr == 0)) {
                return true;
            }
            else if (!(obj instanceof Key)) {
                return false;
            }
            else {
                final Key other = (Key)obj;
                return objPtr == other.objPtr;
            }
        }

        @Override
        public int hashCode() {
            return (int)(objPtr + iidPtr);
        }
    }

    private final Key searchKey = new Key();
    private final HashMap<Key,WeakReference<Object>> _cache = new HashMap<Key,WeakReference<Object>>();

    synchronized void cache(final long iidPtr, final long objPtr, final Object o) {
        // System.out.println("RefCountedCache::cache["+objPtr+","+iidPtr+"]");
        searchKey.iidPtr = iidPtr;
        searchKey.objPtr = objPtr;
        final WeakReference<Object> v = _cache.get(searchKey);
        if (v == null) {
            // System.out.println("RefCountedCache::obtain - added.");
            _cache.put(searchKey.clone(), new WeakReference<Object>(o));
        }
    }

    synchronized void remove(final long iidPtr, final long objPtr) {
        // System.out.println("RefCountedCache::remove["+objPtr+","+iidPtr+"]");
        searchKey.iidPtr = iidPtr;
        searchKey.objPtr = objPtr;
        if (_cache.remove(searchKey) == null) {
            // System.out.println("RefCountedCache::remove - not found.");
        }
    }

    synchronized Object obtain(final long iidPtr, final long objPtr) {
        // System.out.println("RefCountedCache::obtain["+objPtr+","+iidPtr+"]");
        searchKey.iidPtr = iidPtr;
        searchKey.objPtr = objPtr;
        final WeakReference<Object> v = _cache.get(searchKey);
        if (v == null) {
            return null;
        }
        // System.out.println("RefCountedCache::obtain - found.");
        return v.get();
    }
}
