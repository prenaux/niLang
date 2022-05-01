package ni.niLang.util;

import java.lang.reflect.InvocationTargetException;

import ni.common.lang3.reflect.ConstructorUtils;


public class NewInstances {
    private NewInstances() {
        /* Utility class */
    }

    /**
     * Set the true to reload the .class file every time the class is loaded.
     * <p>
     * Note that this works only with classes that can be loaded by the
     * ResourcesClassLoader.
     */
    public static boolean ALWAYS_RELOAD = false;

    private static ResourcesClassLoader singleResourceClassLoader = null;

    /**
     * Load a class using the resource class loader if possible.
     *
     * @param className is the name of the class to load.
     * @return the loaded class or null if the class can't be found.
     * @throws ClassNotFoundException
     */
    static public Class<?> loadClass(final String className) throws ClassNotFoundException
    {
        Class<?> cls = null;
        // Using resource class loader
        if (ALWAYS_RELOAD) {
            cls = (new ResourcesClassLoader()).loadClass(className);
        }
        else {
            if (singleResourceClassLoader == null) {
                singleResourceClassLoader = new ResourcesClassLoader();
            }
            cls = singleResourceClassLoader.loadClass(className);
        }
        // Using the bootstrap class loader
        if (cls == null) {
            try {
                cls = NewInstances.class.getClassLoader().loadClass(className);
            }
            catch (final ClassNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        return cls;
    }

    /**
     * Creates a new instance of the specified class, from a class name.
     *
     * @param className is the name of the class to instantiate.
     * @param args are the arguments to pass to the constructor.
     * @return the new instance.
     * @throws ClassNotFoundException
     * @throws NoSuchMethodException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    static public Object newInstance(final String className, final Object... args) throws ClassNotFoundException, NoSuchMethodException, IllegalAccessException, InvocationTargetException, InstantiationException
    {
        final Class<?> cls = loadClass(className);
        if (cls == null)
            throw new ClassNotFoundException();
        return newInstance(cls, args);
    }

    /**
     * Creates a new instance of the specified class, from a class object.
     *
     * @param cls is the class object of the class to instantiate.
     * @param args are the arguments to pass to the constructor.
     * @return the new instance.
     * @throws NoSuchMethodException
     * @throws IllegalAccessException
     * @throws InvocationTargetException
     * @throws InstantiationException
     */
    static public Object newInstance(final Class<?> cls, final Object... args) throws NoSuchMethodException, IllegalAccessException, InvocationTargetException, InstantiationException
    {
        return ConstructorUtils.invokeConstructor(cls, args);
    }
}
