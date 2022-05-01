package ni.generator.cpp;

import java.util.ListIterator;
import java.util.LinkedList;
import java.util.Comparator;

public class LinkedListRegister<E> extends LinkedList<E> {

    public final Comparator<E> comparator;

    LinkedListRegister(final Comparator<E> comparator) {
        this.comparator = comparator;
    }

    public int getIndex(final E e) {
        int i = this.indexOf(e);
        if (i < 0)
            throw new RuntimeException("Can't find element.");
        return i;
    }

    public void register(final E e) {
        ListIterator<E> itr = listIterator();
        while(true) {
            if (itr.hasNext() == false) {
                itr.add(e);
                return;
            }

            E elementInList = itr.next();
            int cmp = comparator.compare(elementInList,e);
            if (cmp == 0) {
                // already added
                return;
            } else if (cmp > 0) {
                itr.previous();
                itr.add(e);
                return;
            }
        }
    }
}
