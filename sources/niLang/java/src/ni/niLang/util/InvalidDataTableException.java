package ni.niLang.util;

import ni.NiException;

public class InvalidDataTableException extends NiException {
    private static final long serialVersionUID = 1L;

    public InvalidDataTableException() {
        super();
    }

    public InvalidDataTableException(final Throwable t) {
        super(t);
    }

    public InvalidDataTableException(final String message) {
        super(message);
    }

    public InvalidDataTableException(final String message, final Throwable t) {
        super(message, t);
    }
}
