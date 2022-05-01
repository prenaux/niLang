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

import java.nio.CharBuffer;
import java.util.Arrays;

/**
 *
 * @author Samuel Audet
 */
public class CharPointer extends Pointer {
    public CharPointer(final String s) {
        this(s.toCharArray().length + 1);
        putString(s);
    }

    public CharPointer(final char... array) {
        this(array.length);
        asBuffer(array.length).put(array);
    }

    public CharPointer(final int size) {
        allocateArray(size);
    }

    public CharPointer(final Pointer p) {
        super(p);
    }

    private native void allocateArray(int size);

    @Override
    public CharPointer position(final int position) {
        return (CharPointer)super.position(position);
    }

    public char[] getStringChars() {
        // This may be kind of slow, and should be moved to a JNI function.
        char[] buffer = new char[16];
        int i = 0, j = position();
        while ((buffer[i] = position(j).get()) != 0) {
            i++;
            j++;
            if (i >= buffer.length) {
                buffer = Arrays.copyOf(buffer, 2 * buffer.length);
            }
        }
        return Arrays.copyOf(buffer, i);
    }

    public String getString() {
        return new String(getStringChars());
    }

    public CharPointer putString(final String s) {
        final char[] chars = s.toCharArray();
        asBuffer(chars.length + 1).put(chars).put((char)0);
        return this;
    }

    public native char get();

    public native CharPointer put(char c);

    @Override
    public final CharBuffer asBuffer(final int capacity) {
        return asByteBuffer(capacity).asCharBuffer();
    }
}
