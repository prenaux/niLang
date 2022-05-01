package ni;

public class NiException extends Exception {
    private static final long serialVersionUID = 1L;

    public NiException() {
        super();
    }

    public NiException(final String msg) {
        super(msg);
    }

    public NiException(final String msg, final Throwable cause) {
        super(msg, cause);
    }

    public NiException(final Throwable cause) {
        super(cause);
    }
}
