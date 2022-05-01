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

package ni.types.util;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.HashMap;
import java.util.Properties;
import java.util.WeakHashMap;

import ni.types.Pointer;
import ni.types.annotations.Platform;

/**
 *
 * @author Samuel Audet
 */
public class Loader {
    public final static String getPlatformName() {
        final String jvmName = System.getProperty("java.vm.name").toLowerCase();
        String osName = System.getProperty("os.name").toLowerCase();
        String osArch = System.getProperty("os.arch").toLowerCase();
        if (jvmName.startsWith("dalvik") && osName.startsWith("linux")) {
            osName = "android";
        }
        else if (osName.startsWith("mac os x")) {
            osName = "macosx";
        }
        else {
            final int spaceIndex = osName.indexOf(' ');
            if (spaceIndex > 0) {
                osName = osName.substring(0, spaceIndex);
            }
        }
        if (osArch.equals("i386") || osArch.equals("i486") || osArch.equals("i586") || osArch.equals("i686")) {
            osArch = "x86";
        }
        else if (osArch.equals("amd64") || osArch.equals("x86-64")) {
            osArch = "x86_64";
        }
        else if (osArch.startsWith("arm")) {
            osArch = "arm";
        }
        else if (osArch.equals("aarch64")) {
            osArch = "arm64";
        }
        return osName + "-" + osArch;
    }

    public final static Properties getProperties() {
        return getProperties(getPlatformName());
    }

    public final static Properties getProperties(final String name) {
        final Properties p = new Properties();
        p.put("platform.name", name);
        return p;
    }

    public final static void appendProperties(final Properties properties, final Class<?> cls)
    {
        final String platformName = properties.getProperty("platform.name");

        final ni.types.annotations.Properties classProperties =
            cls.getAnnotation(ni.types.annotations.Properties.class);
        Platform[] platforms;
        if (classProperties == null) {
            final Platform platform = cls.getAnnotation(Platform.class);
            if (platform == null) {
                return;
            }
            else {
                platforms = new Platform[] { platform };
            }
        }
        else {
            platforms = classProperties.value();
        }

        String[] define = {}, include = {}, cinclude = {}, includepath = {}, options = {}, linkpath = {}, link = {}, preloadpath = {}, preload = {};
        for (final Platform p : platforms) {
            final String[][] names = { p.value(), p.not() };
            final boolean[] matches = { false, false };
            for (int i = 0; i < names.length; i++) {
                for (final String s : names[i]) {
                    if (platformName.startsWith(s)) {
                        matches[i] = true;
                        break;
                    }
                }
            }
            if (((names[0].length == 0) || matches[0]) && ((names[1].length == 0) || !matches[1])) {
                if (p.define().length > 0) {
                    define = p.define();
                }
                if (p.include().length > 0) {
                    include = p.include();
                }
                if (p.cinclude().length > 0) {
                    cinclude = p.cinclude();
                }
                if (p.includepath().length > 0) {
                    includepath = p.includepath();
                }
                if (p.options().length > 0) {
                    options = p.options();
                }
                if (p.linkpath().length > 0) {
                    linkpath = p.linkpath();
                }
                if (p.link().length > 0) {
                    link = p.link();
                }
                if (p.preloadpath().length > 0) {
                    preloadpath = p.preloadpath();
                }
                if (p.preload().length > 0) {
                    preload = p.preload();
                }
            }
        }

        final String s = properties.getProperty("path.separator");
        appendProperty(properties, "generator.define", "\u0000", define);
        appendProperty(properties, "generator.include", "\u0000", include);
        appendProperty(properties, "generator.cinclude", "\u0000", cinclude);
        appendProperty(properties, "compiler.includepath", s, includepath);
        if (options.length > 0) {
            final String defaultOptions = properties.getProperty("compiler.options");
            properties.setProperty("compiler.options", "");
            for (int i = 0; i < options.length; i++) {
                String o = defaultOptions;
                if (options[i].length() > 0) {
                    o = properties.getProperty("compiler.options." + options[i]);
                }
                appendProperty(properties, "compiler.options", " ", o);
            }
        }
        appendProperty(properties, "compiler.linkpath", s, linkpath);
        appendProperty(properties, "compiler.link", s, link);
        appendProperty(properties, "loader.preloadpath", s, linkpath);
        appendProperty(properties, "loader.preloadpath", s, preloadpath);
        appendProperty(properties, "loader.preload", s, link);
        appendProperty(properties, "loader.preload", s, preload);
    }

    public final static void appendProperty(final Properties properties, final String name,
                                            final String separator, final String... values)
    {
        if ((values == null) || (values.length == 0)) {
            return;
        }
        String value = "";
        for (final String v : values) {
            if ((v == null) || (v.length() == 0)) {
                continue;
            }
            if ((value.length() > 0) && !value.endsWith(separator)) {
                value += separator;
            }
            value += v;
        }
        final String value2 = properties.getProperty(name, "");
        if (value2.length() > 0) {
            value += separator;
        }
        properties.setProperty(name, value + value2);
    }

    public final static String getLibraryName(final Class<?> cls) {
        return "jni" + cls.getSimpleName();
    }

    static WeakHashMap<Class<? extends Pointer>,HashMap<String,Integer>> memberOffsets =
                               new WeakHashMap<Class<? extends Pointer>,HashMap<String,Integer>>();

    static void putMemberOffset(final String typeName, final String member, final int offset) throws ClassNotFoundException
    {
        final Class<?> c = Class.forName(typeName.replace('/', '.'), false, Loader.class.getClassLoader());
        putMemberOffset(c.asSubclass(Pointer.class), member, offset);
    }

    static final synchronized void putMemberOffset(final Class<? extends Pointer> type, final String member, final int offset)
    {
        HashMap<String,Integer> offsets = memberOffsets.get(type);
        if (offsets == null) {
            memberOffsets.put(type, offsets = new HashMap<String,Integer>());
        }
        offsets.put(member, offset);
    }

    public final static int sizeof(final Class<? extends Pointer> type) {
        return memberOffsets.get(type).get("sizeof");
    }


    public static String getProperty(final String name, final String defaultValue) {
        return AccessController.doPrivileged(new PrivilegedAction<String>() {
                public String run() {
                    try {
                        return System.getProperty(name,defaultValue);
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                    return defaultValue;
                }
            });
    }

    public static String getProperty(final String name) {
        return getProperty(name,null);
    }

    public static String setProperty(final String name, final String value) {
        return AccessController.doPrivileged(new PrivilegedAction<String>() {
                public String run() {
                    try {
                        System.setProperty(name,value);
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                    return null;
                }
            });
    }

    public static void load(final String name) {
        AccessController.doPrivileged(new PrivilegedAction<Object>() {
                public Object run() {
                    try {
                        System.load(name);
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                    return null;
                }
            });
    }

    public static void loadLibrary(final String name) {
        AccessController.doPrivileged(new PrivilegedAction<Object>() {
                public Object run() {
                    try {
                        System.loadLibrary(name);
                    }
                    catch (Exception e) {
                        e.printStackTrace();
                    }
                    return null;
                }
            });
    }
}
