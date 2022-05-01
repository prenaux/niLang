package ni.android.util;

import java.util.concurrent.atomic.AtomicInteger;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.widget.Toast;

public class Dialogs {
    protected Dialogs() {
    }

    static public int RESID_Logo48 = 0;

    /**
     * Shows a message with an OK callback.
     * <p>
     * It is guaranteed to run in the context's UI thread.
     */
    static public void alertOk(final Context context, final String message, final Runnable onOK)
    {
        MainThread.run(context, new Runnable() {
            @Override
            public void run() {
                final AlertDialog.Builder alertBuilder = new AlertDialog.Builder(context);
                alertBuilder
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(final DialogInterface dialog, final int which)
                                    {
                                        if (onOK != null)
                                            onOK.run();
                                        dialog.dismiss();
                                    }
                                })
                        .setMessage(message);
                alertBuilder.show();
            }
        });
    }

    /**
     * Shows a message with an OK and Cancel callback.
     * <p>
     * It is guaranteed to run in the context's UI thread.
     */
    static public void alertOkCancel(final Context context, final String message, final Runnable onOK, final Runnable onCancel)
    {
        MainThread.run(context, new Runnable() {
            @Override
            public void run() {
                final AlertDialog.Builder alertBuilder = new AlertDialog.Builder(context);
                alertBuilder
                        .setNegativeButton(android.R.string.cancel,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(final DialogInterface dialog, final int which)
                                    {
                                        if (onCancel != null)
                                            onCancel.run();
                                        dialog.dismiss();
                                    }
                                })
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(final DialogInterface dialog, final int which)
                                    {
                                        if (onOK != null)
                                            onOK.run();
                                        dialog.dismiss();
                                    }
                                })
                        .setMessage(message);
                alertBuilder.show();
            }
        });
    }

    static public void makeToast(final Context context, final String message) {
        MainThread.run(context, new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    static public void makeLongToast(final Context context, final String message) {
        MainThread.run(context, new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, message, Toast.LENGTH_LONG).show();
            }
        });
    }

    static public void notify(final Context context, final String title,
            final String message)
    {
        final Notification notification =
                new Notification(RESID_Logo48, message, System.currentTimeMillis());
        // This contentIntent is a noop.
        final PendingIntent contentIntent = PendingIntent.getService(context, 0, new Intent(), 0);
        notification.setLatestEventInfo(context, title, message, contentIntent);
        notification.flags = Notification.FLAG_AUTO_CANCEL;

        // Get a unique notification id from the application.
        final int notificationId = NotificationIdFactory.create();
        final NotificationManager nm = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        nm.notify(notificationId, notification);
    }

    /**
     * Creates unique id's to identify the notifications created by the android
     * scripting service and the trigger service.
     */
    public static final class NotificationIdFactory {
        private static final AtomicInteger mNextId = new AtomicInteger(0);

        public static int create() {
            return mNextId.incrementAndGet();
        }

        private NotificationIdFactory() {
        }
    }
}
