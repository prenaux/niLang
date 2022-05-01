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

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.Arrays;

/**
 *
 * @author Samuel Audet
 */
public class BytePointer extends Pointer {
    public BytePointer(String s, String charsetName)
            throws UnsupportedEncodingException {
        this(s.getBytes(charsetName).length+1);
        putString(s, charsetName);
    }
    public BytePointer(String s) {
        this(s.getBytes().length+1);
        putString(s);
    }
    public BytePointer(byte ... array) {
        this(array.length);
        asBuffer(array.length).put(array);
    }
    public BytePointer(int size) { allocateArray(size); }
    public BytePointer(Pointer p) { super(p); }
    private native void allocateArray(int size);

    @Override public BytePointer position(int position) {
        return (BytePointer)super.position(position);
    }

    public byte[] getStringBytes() {
        // This may be kind of slow, and should be moved to a JNI function.
        byte[] buffer = new byte[16];
        int i = 0, j = position();
        while ((buffer[i] = position(j).get()) != 0) {
            i++; j++;
            if (i >= buffer.length) {
                buffer = Arrays.copyOf(buffer, 2*buffer.length);
            }
        }
        return Arrays.copyOf(buffer, i);
    }
    public String getString(String charsetName)
            throws UnsupportedEncodingException {
        return new String(getStringBytes(), charsetName);
    }
    public String getString() {
        return new String(getStringBytes());
    }

    public BytePointer putString(String s, String charsetName)
            throws UnsupportedEncodingException {
        byte[] bytes = s.getBytes(charsetName);
        asBuffer(bytes.length+1).put(bytes).put((byte)0);
        return this;
    }
    public BytePointer putString(String s) {
        byte[] bytes = s.getBytes();
        asBuffer(bytes.length+1).put(bytes).put((byte)0);
        return this;
    }

    public native byte get();
    public native BytePointer put(byte b);

    @Override public final ByteBuffer asBuffer(int capacity) {
        return (ByteBuffer)super.asBuffer(capacity);
    }
}
