package ni.niLang.util;

import java.io.IOException;
import java.io.OutputStream;

import ni.niLang.IFile;

public class IFileOutputStream extends OutputStream {
    public final IFile fp;

    IFileOutputStream(final IFile aFP) {
        fp = aFP;
    }

    @Override
    public void write(final int b) throws IOException {
        fp.write8((byte)b);
    }

    @Override
    public void write(final byte b[], final int off, final int len) throws IOException {
        final int wrote = Files.writeByteArray(fp, b, off, len);
        if (wrote != len)
            throw new IOException("Can't write to iFile.");
    }

    @Override
    public void flush() {
        fp.flush();
    }

    @Override
    public void close() {
        fp.flush();
    }
}
