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

package ni.generator.cpp;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.lang.reflect.Method;
import java.util.LinkedList;
import java.util.Properties;
import java.util.Arrays;
import java.util.Comparator;

import ni.types.util.Loader;

/**
 *
 * @author Samuel Audet
 */
public class Builder {
    public static final Comparator<Class<?>> CMP_CLASS = new Comparator<Class<?>>() {
            public int compare(Class<?> b1, Class<?> b2) {
                return b1.getName().compareToIgnoreCase(b2.getName());
            }
        };

    public static final Comparator<Class> CMP_CLASSZ = new Comparator<Class>() {
            public int compare(Class b1, Class b2) {
                return b1.getName().compareToIgnoreCase(b2.getName());
            }
        };

    public static final Comparator<Method> CMP_METHOD = new Comparator<Method>() {
            public int compare(Method b1, Method b2) {
                int r = b1.getName().compareToIgnoreCase(b2.getName());
                if (r != 0)
                    return r;
                int p1 = b1.getParameterCount();
                int p2 = b2.getParameterCount();
                r = p2 - p1;
                if (r != 0)
                    return r;
                Class[] c1 = b1.getParameterTypes();
                Class[] c2 = b2.getParameterTypes();
                for (int i = 0; i < p1; ++i) {
                    r = c1[i].getName().compareToIgnoreCase(c2[i].getName());
                    if (r != 0)
                        return r;
                }
                return 0;
            }
        };

    public static final Comparator<String> CMP_STRING = new Comparator<String>() {
            public int compare(String b1, String b2) {
                return b1.compareToIgnoreCase(b2);
            }
        };

    static public Method[] getSortedDeclaredMethods(Class<?> cls) {
        Method[] methods = cls.getDeclaredMethods();
        Arrays.sort(methods, CMP_METHOD);
        return methods;
    }

    static public Class[] getSortedDeclaredClasses(Class<?> cls) {
        Class[] classes = cls.getDeclaredClasses();
        Arrays.sort(classes, CMP_CLASSZ);
        return classes;
    }

    public Builder(final Properties properties) {
        // try to find include paths for jni.h and jni_md.h automatically
        final String[] jnipath = new String[2];
        if ((jnipath[0] != null) && jnipath[0].equals(jnipath[1])) {
            jnipath[1] = null;
        }
        Loader.appendProperty(properties, "compiler.includepath",
                properties.getProperty("path.separator"), jnipath);
    }

    public static LinkedList<File> doGenerate(
            final String moduleName,
            final Class<?>[] classes,
            Properties properties,
            File outputDirectory,
            final String outputName)
            throws IOException, InterruptedException, URISyntaxException
    {
        final LinkedList<File> outputFiles = new LinkedList<File>();
        properties = (Properties)properties.clone();
        if (classes != null) {
            Arrays.sort(classes,CMP_CLASS);
            for (final Class<?> c : classes) {
                Loader.appendProperties(properties, c);
            }
        }
        File sourceFile;
        if (outputDirectory == null) {
            if (classes == null) {
                outputDirectory = new File(properties.getProperty("platform.name"));
                sourceFile = new File(outputName + ".cpp");
            }
            else if (classes.length == 1) {
                final URL resourceURL = classes[0].getResource(classes[0].getSimpleName() + ".class");
                final File packageDir = new File(resourceURL.toURI()).getParentFile();
                outputDirectory = new File(packageDir, properties.getProperty("platform.name"));
                sourceFile = new File(packageDir, outputName + ".cpp");
            }
            else {
                outputDirectory = new File(properties.getProperty("platform.name"));
                sourceFile = new File(outputName + ".cpp");
            }
        }
        else {
            sourceFile = new File(outputDirectory, outputName + ".cpp");
        }
        if (!outputDirectory.exists()) {
            outputDirectory.mkdirs();
        }
        System.out.println("Generating source file: " + sourceFile);
        final Generator generator = new Generator(properties, sourceFile);
        generator.generate(moduleName, classes);
        generator.close();
        return outputFiles;
    }

    public static class UserClassLoader extends URLClassLoader {
        private final LinkedList<String> paths = new LinkedList<String>();

        public UserClassLoader() throws MalformedURLException {
            super(new URL[0]);
            addPaths(System.getProperty("user.dir"));
        }

        public void addPaths(final String... paths) throws MalformedURLException {
            for (final String path : paths) {
                if (path.endsWith("/*") || path.endsWith("/.")) {
                    final String dir = path.substring(0, path.length() - 2);
                    final File files[] = new File(dir).listFiles();
                    for (final File f : files) {
                        addPath(f.getPath());
                    }
                }
                else {
                    addPath(path);
                }
            }
        }

        public void addPath(final String path) throws MalformedURLException {
            this.paths.add(path);
            addURL(new File(path).toURI().toURL());
        }

        public String[] getPaths() {
            return paths.toArray(new String[paths.size()]);
        }
    }

    public static void printHelp() {
        System.err.println("Usage: java -jar ni-gen-cpp.jar [options] <classes>");
        System.err.println();
        System.err.println("where options include:");
        System.err.println();
        System.err.println("    -classpath <path>      Load user classes from path");
        System.err.println("    -d <directory>         Dump all output files in directory");
        System.err.println("    -base <name>           Build the base support file in the specified file");
        System.err.println("    -o <name>              Output everything in a file named after given name");
        System.err.println("    -properties <resource> Load all properties from resource");
        System.err.println("    -propertyfile <file>   Load all properties from file");
        System.err.println("    -D<property>=<value>   Set property to value");
        System.err.println();
    }

    public static void main(final String[] args) throws Exception {
        if (args.length == 0) {
            printHelp();
            System.exit(1);
        }

        final UserClassLoader classLoader = new UserClassLoader();
        File outputDirectory = null;
        String outputName = null;
        String buildBase = null;
        String moduleName = null;
        Properties properties = Loader.getProperties();
        final LinkedList<Class<?>> classes = new LinkedList<Class<?>>();

        for (int i = 0; i < args.length; i++) {
            if ("-help".equals(args[i]) || "--help".equals(args[i])) {
                printHelp();
                System.exit(0);
            }
            else if ("-classpath".equals(args[i]) || "-cp".equals(args[i]) || "-lib".equals(args[i])) {
                classLoader.addPaths(args[++i].split(File.pathSeparator));
            }
            else if ("-d".equals(args[i])) {
                outputDirectory = new File(args[++i]);
            }
            else if ("-base".equals(args[i])) {
                buildBase = args[++i];
            }
            else if ("-module".equals(args[i])) {
                moduleName = args[++i];
            }
            else if ("-o".equals(args[i])) {
                outputName = args[++i];
            }
            else if ("-properties".equals(args[i])) {
                properties = Loader.getProperties(args[++i]);
            }
            else if ("-propertyfile".equals(args[i])) {
                final FileInputStream fis = new FileInputStream(args[++i]);
                properties = new Properties(properties);
                try {
                    properties.load(new InputStreamReader(fis));
                }
                catch (final NoSuchMethodError e) {
                    properties.load(fis);
                }
                fis.close();
            }
            else if (args[i].startsWith("-D")) {
                int equalIndex = args[i].indexOf('=');
                if (equalIndex < 0) {
                    equalIndex = args[i].indexOf(':');
                }
                final String key = args[i].substring(2, equalIndex);
                final String value = args[i].substring(equalIndex + 1);
                if ((key.length() > 0) && (value.length() > 0)) {
                    properties.put(key, value);
                }
            }
            else if (args[i].startsWith("-")) {
                System.err.println("W/Unknown option: " + args[i]);
            }
            else {
                // Loader.loadLibraries = false;
                String className = args[i].replace('/', '.');
                if (className.endsWith(".class")) {
                    className = className.substring(0, className.length() - 6);
                }
                try {
                    final Class<?> c = Class.forName(className, true, classLoader);
                    classes.add(c);
                }
                catch (final ClassNotFoundException e) {
                    System.err.println("Class not found: " + className);
                    System.exit(1);
                }
            }
        }

        LinkedList<File> outputFiles;
        outputFiles = new LinkedList<File>();
        if (buildBase != null) {
            outputFiles.addAll(doGenerate(
                                   moduleName, null, properties,
                                   outputDirectory, buildBase));
        }
        else if (outputName == null) {
            for (final Class<?> c : classes) {
                outputFiles.addAll(doGenerate(
                                       moduleName,
                                       new Class<?>[] { c }, properties,
                                       outputDirectory, Loader.getLibraryName(c)));
            }
        }
        else {
            outputFiles.addAll(doGenerate(
                                   moduleName,
                                   classes.toArray(new Class<?>[classes.size()]),
                                   properties, outputDirectory, outputName));
        }
    }
}
