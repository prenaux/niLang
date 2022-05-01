package ni.niLang.util;

import ni.niLang.IMessageDesc;

public class Strings extends ni.common.util.Strings {
    protected Strings() {
    }

    static public String fromMessage(final Object receiver, final IMessageDesc m) {
        if (receiver != null) {
            return fromMessage(receiver, m.getID(), m.getA(), m.getB());
        }
        else {
            return fromMessage(m.getID(), m.getA(), m.getB());
        }
    }

    static public String fromMessage(final IMessageDesc m) {
        return fromMessage(null, m);
    }
}
