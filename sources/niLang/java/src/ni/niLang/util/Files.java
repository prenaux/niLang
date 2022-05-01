package ni.niLang.util;

import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;

import ni.niLang.EFileOpenMode;
import ni.niLang.EFileAttrFlags;
import ni.niLang.IFile;
import ni.niLang.Lib;

public class Files extends ni.common.util.Files {
    protected Files() {
    }

    /**
     * Returns a directory where user files can be written to safely.
     */
    static public String getHomeDir() {
        return Lib.getLang().getProperty("ni.dirs.home");
    }

    static public String getHomePath(final String aPath) {
        return getHomeDir() + aPath;
    }

    /**
     * Check whether the specified file name can be found by the file system.
     *
     * @param aPath is the name of the resource to look for.
     * @param aBasePath aBasePath is an additional directory where the resource
     *            will be searched for, only the directory part of the path is
     *            used.
     * @return true if the resource can be found, false if not.
     */
    static public boolean fileExists(final String aPath) {
        return Lib.getRootFS().fileExists(aPath,EFileAttrFlags.AllFiles) != 0;
    }

    /**
     * Check whether the specified directory name can be found by the file system.
     *
     * @param aPath is the name of the resource to look for.
     * @param aBasePath aBasePath is an additional directory where the resource
     *            will be searched for, only the directory part of the path is
     *            used.
     * @return true if the resource can be found, false if not.
     */
    static public boolean dirExists(final String aPath) {
        return Lib.getRootFS().fileExists(aPath,EFileAttrFlags.AllDirectories) != 0;
    }

    /**
     * Open a read-only file using the resource manager.
     *
     * @param aPath is the name of the resource to open.
     * @param aBasePath is an additional directory where the resource will be
     *            searched for, only the directory part of the path is used.
     * @return The opened file.
     * @throws FileNotFoundException
     */
    static public IFile fileOpenRead(final String aPath, final String aBasePath) throws FileNotFoundException
    {
        final IFile fp = Lib.getRootFS().fileOpen(aPath, EFileOpenMode.Write);
        if (fp == null)
            throw new FileNotFoundException();
        return fp;
    }

    /**
     * Open a read-only file using the resource manager.
     *
     * @param aPath
     * @return The opened file.
     * @throws FileNotFoundException
     */
    static public IFile fileOpenRead(final String aPath) throws FileNotFoundException {
        return fileOpenRead(aPath, null);
    }

    /**
     * Open a read-only file using the resource manager.
     *
     * @param aPath
     * @return The opened file input stream.
     * @throws FileNotFoundException
     */
    static public IFileInputStream fileOpenInputStream(final String aPath) throws FileNotFoundException
    {
        return new IFileInputStream(fileOpenRead(aPath, null));
    }

    /**
     * Open a write-only file.
     *
     * @param aPath the path where to write the file.
     * @return The opened filed.
     * @throws IOException if the file can't be created or overwritten.
     */
    static public IFile fileOpenWrite(final String aPath) throws IOException {
        final IFile fp = Lib.getRootFS().fileOpen(aPath, EFileOpenMode.Write);
        if (fp == null)
            throw new IOException();
        return fp;
    }

    /**
     * Open an OutputStream using the resource manager.
     *
     * @param aPath
     * @return The opened file output stream.
     * @throws IOException if the file can't be created or overwritten.
     */
    static public IFileOutputStream fileOpenOutputStream(final String aPath) throws IOException
    {
        return new IFileOutputStream(fileOpenWrite(aPath));
    }

    /**
     * Read the file in the specified byte array, from the current position to
     * the end of the file.
     *
     * @param fp is the file to read from.
     * @param ba is the byte array to read to.
     * @param readSize is the number of bytes to read.
     * @throws InterruptedIOException if the read failed because the
     *             {@link IFile#read} method fails (aka doesn't read the
     *             expected number of bytes).
     */
    static public void readToByteArray(final IFile fp, final byte[] ba, final int readSize) throws InterruptedIOException
    {
        if (readSize > ba.length)
            throw new BufferOverflowException();
        final int read = fp.readRaw(ByteBuffer.wrap(ba), readSize);
        if (read != readSize)
            throw new InterruptedIOException();
    }

    /**
     * Read the file in a byte array, from the current position to the end of
     * the file.
     *
     * @param fp is the file to read from.
     * @return A new byte array containing the data read.
     * @throws EOFException if there is nothing to read (0 bytes left)
     * @throws InterruptedIOException if the read failed because the
     *             {@link IFile#read} method fails (aka doesn't read the
     *             expected number of bytes).
     */
    static public byte[] readToByteArray(final IFile fp) throws EOFException, InterruptedIOException
    {
        final long readSize = fp.getSize() - fp.tell();
        if (readSize <= 0)
            throw new EOFException();
        final byte[] data = new byte[(int)readSize];
        if (fp.readRaw(ByteBuffer.wrap(data), (int)readSize) != readSize)
            throw new InterruptedIOException();
        return data;
    }

    /**
     * Read a whole resource file in a byte array.
     *
     * @param aPath is the path of the resource to read from.
     * @return A new byte array containing the data read.
     * @throws FileNotFoundException {@link #openRead(String)}
     * @throws EOFException {@link #readToByteArray(IFile)}
     * @throws InterruptedIOException {@link #readToByteArray(IFile)}
     */
    static public byte[] readFileToByteArray(final String aPath) throws FileNotFoundException, EOFException, InterruptedIOException
    {
        final IFile fp = fileOpenRead(aPath);
        try {
            return readToByteArray(fp);
        }
        finally {
            fp.invalidate();
            fp.dispose();
        }
    }

    static public ByteArrayInputStream readFileToInputStream(final String path) throws FileNotFoundException, EOFException, InterruptedIOException
    {
        return new ByteArrayInputStream(readFileToByteArray(path));
    }

    static public int writeByteArray(final IFile fp, final byte b[], final int off, final int len) throws IOException
    {
        ByteBuffer bb;
        if (off != 0) {
            bb = ByteBuffer.allocate(len);
            bb.put(b, off, len);
            bb.rewind();
        }
        else {
            bb = ByteBuffer.wrap(b, 0, len);
        }
        return fp.writeRaw(bb, len);
    }

    static public int writeByteArray(final String aPath, final byte b[], final int off, final int len) throws IOException
    {
        final IFile fp = fileOpenWrite(aPath);
        try {
            return writeByteArray(fp, b, off, len);
        }
        finally {
            fp.invalidate();
            fp.dispose();
        }
    }

    /**
     * Check whether the specified resource name can be found by the resource
     * manager.
     *
     * @param aPath is the name of the resource to look for.
     * @return true if the resource can be found, false if not.
     */
    static public boolean urlExists(final String aPath) {
    	return Lib.getLang().urlexists(aPath);
    }


    /**
     * Open a read-only file using the resource manager.
     *
     * @param aPath is the name of the resource to open.
     * @return The opened file.
     * @throws FileNotFoundException
     */
    static public IFile urlOpen(final String aPath) throws FileNotFoundException
    {
        final IFile fp = Lib.getLang().urlopen(aPath);
        if (fp == null)
            throw new FileNotFoundException();
        return fp;
    }

    /**
     * Open a read-only file using the resource manager.
     *
     * @param aPath
     * @return The opened file input stream.
     * @throws FileNotFoundException
     */
    static public IFileInputStream urlOpenInputStream(final String aPath) throws FileNotFoundException
    {
        return new IFileInputStream(urlOpen(aPath));
    }

    /**
     * Read a whole resource file in a byte array.
     *
     * @param aPath is the path of the resource to read from.
     * @return A new byte array containing the data read.
     * @throws FileNotFoundException {@link #openRead(String)}
     * @throws EOFException {@link #readToByteArray(IFile)}
     * @throws InterruptedIOException {@link #readToByteArray(IFile)}
     */
    static public byte[] readResToByteArray(final String aPath) throws FileNotFoundException, EOFException, InterruptedIOException
    {
        final IFile fp = urlOpen(aPath);
        try {
            return readToByteArray(fp);
        }
        finally {
            fp.invalidate();
            fp.dispose();
        }
    }

    static public ByteArrayInputStream readToInputStream(final IFile fp) throws EOFException, InterruptedIOException
    {
        return new ByteArrayInputStream(readToByteArray(fp));
    }

    static public ByteArrayInputStream readResToInputStream(final String path) throws FileNotFoundException, EOFException, InterruptedIOException
    {
        return new ByteArrayInputStream(readResToByteArray(path));
    }
}
