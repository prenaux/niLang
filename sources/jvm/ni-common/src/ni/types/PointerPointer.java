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

package ni.types;

/**
 *
 * @author Samuel Audet
 */
public class PointerPointer extends Pointer {
    public PointerPointer(Pointer  ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(byte[]   ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(short[]  ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(int[]    ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(long[]   ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(float[]  ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(double[] ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(char[]   ... array) { this(array.length); put(array); position(0); }
    public PointerPointer(int size) { allocateArray(size); }
    public PointerPointer(Pointer p) { super(p); }
    private native void allocateArray(int size);

    // this is just to keep references to Pointer objects we create
    private Pointer[] pointerArray;

    @Override public PointerPointer position(int position) {
        return (PointerPointer)super.position(position);
    }

    public PointerPointer put(Pointer ... array) {
        for (int i = 0; i < array.length; i++) {
            position(i).put(array[i]);
        }
        return this;
    }
    public PointerPointer put(byte[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new BytePointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(short[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new ShortPointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(int[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new IntPointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(long[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new LongPointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(float[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new FloatPointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(double[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new DoublePointer(array[i]);
        }
        return put(pointerArray);
    }
    public PointerPointer put(char[] ... array) {
        pointerArray = new Pointer[array.length];
        for (int i = 0; i < array.length; i++) {
            pointerArray[i] = new CharPointer(array[i]);
        }
        return put(pointerArray);
    }

    public native Pointer get();
    public native PointerPointer put(Pointer p);
}
