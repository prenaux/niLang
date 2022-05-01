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

import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import ni.types.annotations.Opaque;
import ni.types.util.Loader;

/**
 *
 * @author Samuel Audet
 */
@Opaque
public class Pointer {
    public Pointer() {
        // Make sure out top enclosing class is initialized and the
        // associated native loaded. This code is actually quite efficient.
        //        Class cls = getClass();
        //        String className = cls.getName();
        //        int topIndex = className.indexOf('$');
        //        if (topIndex > 0 && Tools.loadedLibraries.
        //                get(className.substring(0, topIndex)) == null) {
        //            Tools.load(cls);
        //        }
    }

    public Pointer(final Pointer p) {
        if (p == null) {
            address = 0;
            position = 0;
        }
        else {
            address = p.address;
            position = p.position;
        }
    }

    void init(final long allocatedAddress, final long deallocatorAddress) {
        address = allocatedAddress;
        deallocator(new ReferenceDeallocator(this, allocatedAddress, deallocatorAddress));
    }

    public interface Deallocator {
        void deallocate();
    }

    static class ReferenceDeallocator extends DeallocatorReference implements Deallocator {
        ReferenceDeallocator(final Pointer p, final long allocatedAddress, final long deallocatorAddress) {
            super(p, null);
            this.deallocator = this;
            this.allocatedAddress = allocatedAddress;
            this.deallocatorAddress = deallocatorAddress;
        }

        private long allocatedAddress;
        private long deallocatorAddress;

        @Override
        public void deallocate() {
            if ((allocatedAddress != 0) && (deallocatorAddress != 0)) {
                //                System.out.println("deallocating 0x" + Long.toHexString(allocatedAddress) +
                //                        " 0x" + Long.toHexString(deallocatorAddress));
                deallocate(allocatedAddress, deallocatorAddress);
                allocatedAddress = deallocatorAddress = 0;
            }
        }

        private native void deallocate(long allocatedAddress, long deallocatorAddress);
    }

    static class DeallocatorReference extends PhantomReference<Pointer> {
        DeallocatorReference(final Pointer p, final Deallocator deallocator) {
            super(p, referenceQueue);
            this.deallocator = deallocator;
        }

        static DeallocatorReference head = null;
        DeallocatorReference prev = null, next = null;
        Deallocator deallocator;

        synchronized final void add() {
            if (head == null) {
                head = this;
            }
            else {
                next = head;
                next.prev = head = this;
            }
        }

        synchronized final void remove() {
            if ((prev == this) && (next == this)) {
                return;
            }
            if (prev == null) {
                head = next;
            }
            else {
                prev.next = next;
            }
            if (next != null) {
                next.prev = prev;
            }
            prev = next = this;
        }

        @Override
        public void clear() {
            super.clear();
            deallocator.deallocate();
        }
    }

    static final ReferenceQueue<Pointer> referenceQueue = new ReferenceQueue<Pointer>();

    protected long address;
    protected int position;
    private Deallocator deallocator;

    public boolean isNull() {
        return address == 0;
    }

    public void setNull() {
        address = 0;
    }

    public int position() {
        return position;
    }

    public Pointer position(final int position) {
        this.position = position;
        return this;
    }

    protected Deallocator deallocator() {
        return deallocator;
    }

    protected Pointer deallocator(final Deallocator deallocator) {
        this.deallocator = deallocator;

        DeallocatorReference r = null;
        while ((r = (DeallocatorReference)referenceQueue.poll()) != null) {
            r.clear();
            r.remove();
        }

        if (deallocator instanceof DeallocatorReference) {
            r = (DeallocatorReference)deallocator;
        }
        else {
            r = new DeallocatorReference(this, deallocator);
        }
        r.add();
        return this;
    }

    public void deallocate() {
        deallocator.deallocate();
        address = 0;
    }

    private native ByteBuffer asDirectBuffer(int capacity);

    public ByteBuffer asByteBuffer(final int capacity) {
        if (isNull()) {
            return null;
        }
        int valueSize = 1;
        try {
            final Class<? extends Pointer> c = getClass();
            if (c != Pointer.class) {
                valueSize = Loader.sizeof(c);
            }
        }
        catch (final NullPointerException e) { /* default to 1 byte */
        }

        final int arrayPosition = position;
        position = valueSize * arrayPosition;
        final ByteBuffer b = asDirectBuffer(valueSize * capacity).order(ByteOrder.nativeOrder());
        position = arrayPosition;
        return b;
    }

    public Buffer asBuffer(final int capacity) {
        return asByteBuffer(capacity);
    }

    @Override
    public boolean equals(final Object obj) {
        if ((obj == null) && isNull()) {
            return true;
        }
        else if (!(obj instanceof Pointer)) {
            return false;
        }
        else {
            final Pointer other = (Pointer)obj;
            return (address == other.address) && (position == other.position);
        }
    }

    @Override
    public int hashCode() {
        return (int)address;
    }

    @Override
    public String toString() {
        return getClass().getName() + "[address=0x" + Long.toHexString(address) +
                ",position=" + position + ",deallocator=" + deallocator + "]";
    }
}
