package ni.niLang.util;

import java.io.FileNotFoundException;
import java.io.IOException;

import ni.niLang.ESerializeMode;
import ni.niLang.IDataTable;
import ni.niLang.IFile;
import ni.niLang.Lib;

public class DataTable {
    protected DataTable() {
    }

    static public IDataTable create(final String name) {
        return Lib.getLang().createDataTable(name);
    }

    static public IDataTable create() {
        return create("");
    }

    static public IDataTable load(final String type, final IDataTable dt, final IFile fp) throws InvalidDataTableException
    {
        if (!Lib.getLang().serializeDataTable(type, ESerializeMode.Read, dt, fp))
            throw new InvalidDataTableException();
        return dt;
    }

    static public IDataTable load(final String type, final IFile fp) throws InvalidDataTableException
    {
        final IDataTable dt = create("");
        load(type, dt, fp);
        return dt;
    }

    static public IDataTable load(final String type, final String aPath) throws InvalidDataTableException, FileNotFoundException
    {
        final IFile fp = Files.urlOpen(aPath);
        return load(type, fp);
    }

    static public IDataTable write(final String type, final IDataTable dt, final IFile fp) throws InvalidDataTableException, IOException
    {
        if (dt == null)
            throw new InvalidDataTableException();
        if (!Lib.getLang().serializeDataTable(type, ESerializeMode.Write, dt, fp))
            throw new IOException("Can't write data table.");
        return dt;
    }

    static public IDataTable write(final String type, final IDataTable dt, final String aPath) throws InvalidDataTableException, IOException
    {
        final IFile fp = Files.fileOpenWrite(aPath);
        try {
            return write(type, dt, fp);
        }
        catch (final InvalidDataTableException e) {
            throw e;
        }
        catch (final IOException e) {
            throw e;
        }
        finally {
            fp.flush();
            fp.dispose();
        }
    }

    static public String toString(final IDataTable dt, final String mode) {
        String ret = "";
        final IFile fp = Lib.getLang().createFileDynamicMemory(256, "");
        try {
            write(mode, dt, fp);
            fp.seekSet(0);
            ret = fp.readString();
        }
        catch (final Exception e) {
        }
        finally {
            fp.dispose();
        }
        return ret;
    }

    static public String toString(final IDataTable dt) {
        return toString(dt, "xml-raw");
    }
}
