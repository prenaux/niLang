package ni.niLang.util;

import java.io.IOException;
import java.util.Hashtable;

public class ResourcesClassLoader extends ClassLoader {
    private final Hashtable<String,Class<?>> classes = new Hashtable<String,Class<?>>();

    ResourcesClassLoader() {
        super(ResourcesClassLoader.class.getClassLoader());
    }

    @Override
    public Class<?> loadClass(final String className) throws ClassNotFoundException {
        return findClass(className);
    }

    @Override
    public Class<?> findClass(final String className) {
        Class<?> result = classes.get(className); //checks in cached classes
        if (result != null) {
            return result;
        }

        try {
            // Using Class.forName because findSystemClass doesn't do the job on Android
            return Class.forName(className);
        }
        catch (final Exception e) {
        }

        try {
            final byte[] bytes = loadClassData(className);
            result = defineClass(className, bytes, 0, bytes.length);
            classes.put(className, result);
            return result;
        }
        catch (final IOException ioe) {
            return null;
        }
    }

    private byte[] loadClassData(final String className) throws IOException {
        final String filePath = className.replaceAll("\\.", "/") + ".class";
        if (!Files.urlExists(filePath))
            return null;
        return Files.readResToByteArray(filePath);
    }
}
