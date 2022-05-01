package ni.common.util;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class Collections {
    public static <T> List<T> List() {
        return new ArrayList<T>();
    }

    public static <T> List<T> List(final T... args) {
        return Arrays.asList(args);
    }

    public static <T> Deque<T> Deqeue() {
        // Using LinkedList because Android doesn't have ArrayDeque
        return new LinkedList<T>();
        //return new ArrayDeque<T>();
    }

    public static <T> ConcurrentLinkedQueue<T> ConcurrentQueue() {
        return new ConcurrentLinkedQueue<T>();
    }

    public static <T> TreeSet<T> TreeSet() {
        return new TreeSet<T>();
    }

    public static <T> TreeSet<T> TreeSet(final T... args) {
        final TreeSet<T> result = new TreeSet<T>();
        result.addAll(Arrays.asList(args));
        return result;
    }

    public static <T> HashSet<T> HashSet() {
        return new HashSet<T>();
    }

    public static <T> HashSet<T> HashSet(final T... args) {
        final HashSet<T> result = new HashSet<T>(args.length);
        result.addAll(Arrays.asList(args));
        return result;
    }

    public static <K, V> TreeMap<K,V> TreeMap() {
        return new TreeMap<K,V>();
    }

    public static <K, V> TreeMap<K,V> TreeMap(final Entry<? extends K,? extends V>... entries)
    {
        final TreeMap<K,V> result = new TreeMap<K,V>();

        for (final Entry<? extends K,? extends V> entry : entries)
            if (entry.value != null)
                result.put(entry.key, entry.value);

        return result;
    }

    public static <K, V> HashMap<K,V> HashMap() {
        return new HashMap<K,V>();
    }

    public static <K, V> HashMap<K,V> HashMap(final Entry<? extends K,? extends V>... entries)
    {
        final HashMap<K,V> result = new HashMap<K,V>(entries.length);

        for (final Entry<? extends K,? extends V> entry : entries)
            if (entry.value != null)
                result.put(entry.key, entry.value);

        return result;
    }

    public static <K, V> ConcurrentHashMap<K,V> ConcurrentHashMap() {
        return new ConcurrentHashMap<K,V>();
    }

    public static <K, V> ConcurrentHashMap<K,V> ConcurrentHashMap(final Entry<? extends K,? extends V>... entries)
    {
        final ConcurrentHashMap<K,V> result = new ConcurrentHashMap<K,V>(entries.length);

        for (final Entry<? extends K,? extends V> entry : entries)
            if (entry.value != null)
                result.put(entry.key, entry.value);

        return result;
    }

    public static <K, V> Entry<K,V> Entry(final K key, final V value) {
        return new Entry<K,V>(key, value);
    }

    public static class Entry<K, V> {
        K key;
        V value;

        public Entry(final K key, final V value) {
            this.key = key;
            this.value = value;
        }
    }

    public static <T> Reversed<T> reversed(final List<T> original) {
        return Reversed.reversed(original);
    }
}
