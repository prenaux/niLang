package ni.samples.HelloUI;

import static ni.common.math.Vec.Vec4f;
import ni.common.util.Log;
import ni.types.MessageID;
import ni.niLang.Lib;
import ni.niUI.EUIMessage;
import ni.niUI.EWidgetButtonCmd;
import ni.niUI.EWidgetDockStyle;
import ni.niUI.EWidgetStyle;
import ni.niUI.IWidget;
import ni.niUI.IWidgetCommand;
import ni.niUI.widget.WidgetSink;

public class HelloWidget extends WidgetSink {

	private int _clickCount = 0;

	public void onSinkAttached(IWidget aW) {
        IWidget w = aW.getUIContext().createWidget(
                "Button",
                aW,
                Vec4f(0,0,200,200),
                EWidgetStyle.HoldFocus,
                Lib.hstr("ID_Button"));
            w.setText(Lib.hstr("HelloUI Narf."));
            w.setDockStyle(EWidgetDockStyle.SnapCenter);
	}

	public boolean onCommand(IWidgetCommand cmd) {
        Log.d("onCommand.");
		return false;
	}

	@Override
	public boolean onWidgetSink(final IWidget w, final int msg, final Object a, final Object b) {
		if (msg != EUIMessage.ContextAfterDraw &&
			msg != EUIMessage.ContextBeforeDraw &&
			msg != EUIMessage.ContextUpdate &&
			msg != EUIMessage.NCPaint &&
			msg != EUIMessage.Paint)
		{
			Log.d("MSG: " + MessageID.toString(msg) + "(" + a + "," + b + ")");
		}
		return super.onWidgetSink(w, msg, a, b);
	}

    // Called when interacting with our button
    public void ID_Button(IWidgetCommand cmd) {
        if (cmd.getID() == EWidgetButtonCmd.Clicked) {
            Log.d("Clicked the button.");
            IWidget w = IWidget.query(cmd.getSender());
            w.setText(Lib.hstr("HelloUI, clickCount: " + (++_clickCount)));
        }
    }
}
