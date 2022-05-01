package ni.types;

/**
 * Thrown when an interface can't be queried on a given object.
 */
public class QueryInterfaceException extends RuntimeException {
    private static final long serialVersionUID = 4157833925554128867L;

    public QueryInterfaceException() {
        super();
    }

    public QueryInterfaceException(final Throwable t) {
        super(t);
    }

    public QueryInterfaceException(final String message) {
        super(message);
    }

    public QueryInterfaceException(final String message, final Throwable t) {
        super(message, t);
    }
}
