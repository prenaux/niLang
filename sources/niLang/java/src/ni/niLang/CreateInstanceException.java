package ni.niLang;

public class CreateInstanceException extends RuntimeException {
    private static final long serialVersionUID = 5397275591452257958L;

    public CreateInstanceException() {
        super();
    }

    public CreateInstanceException(final Throwable t) {
        super(t);
    }

    public CreateInstanceException(final String message) {
        super(message);
    }

    public CreateInstanceException(final String message, final Throwable t) {
        super(message, t);
    }
}
