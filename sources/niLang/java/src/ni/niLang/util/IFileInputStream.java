package ni.niLang.util;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;

import ni.niLang.IFile;

public class IFileInputStream extends InputStream {
    public final IFile fp;

    public IFileInputStream(final IFile aFP) {
        fp = aFP;
    }

    @Override
    public int read() throws IOException {
        if (fp.getPartialRead())
            return -1;
        return fp.read8();
    }

    @Override
    public int read(final byte b[], final int off, final int len) throws IOException {
        if (fp.getPartialRead())
            return -1;
        return fp.readRaw(ByteBuffer.wrap(b, off, b.length - off), len);
    }

    @Override
    public long skip(final long n) {
        if (!fp.seek(n))
            return 0;
        return n;
    }

    @Override
    public int available() {
        return (int)(fp.getSize() - fp.tell());
    }
}
