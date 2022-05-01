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

import java.nio.IntBuffer;
import java.util.Arrays;

/**
 *
 * @author Samuel Audet
 */
public class IntPointer extends Pointer {
    public IntPointer(String s) {
        this(s.length()+1);
        putString(s);
    }
    public IntPointer(int ... array) {
        this(array.length);
        asBuffer(array.length).put(array);
    }
    public IntPointer(int size) { allocateArray(size); }
    public IntPointer(Pointer p) { super(p); }
    private native void allocateArray(int size);

    @Override public IntPointer position(int position) {
        return (IntPointer)super.position(position);
    }

    public int[] getStringCodePoints() {
        // This may be kind of slow, and should be moved to a JNI function.
        int[] buffer = new int[16];
        int i = 0, j = position();
        while ((buffer[i] = position(j).get()) != 0) {
            i++; j++;
            if (i >= buffer.length) {
                buffer = Arrays.copyOf(buffer, 2*buffer.length);
            }
        }
        return Arrays.copyOf(buffer, i);
    }
    public String getString() {
        int[] codePoints = getStringCodePoints();
        return new String(codePoints, 0, codePoints.length);
    }
    public IntPointer putString(String s) {
        int[] codePoints = new int[s.length()];
        for (int i = 0; i < codePoints.length; i++) {
            codePoints[i] = s.codePointAt(i);
        }
        asBuffer(codePoints.length+1).put(codePoints).put((int)0);
        return this;
    }

    public native int get();
    public native IntPointer put(int i);

    @Override public final IntBuffer asBuffer(int capacity) {
        return asByteBuffer(capacity).asIntBuffer();
    }
}
