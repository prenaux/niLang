package ni.niUI.widget;

import java.lang.reflect.InvocationTargetException;

import ni.niLang.util.IScriptingHostContextHandler;
import ni.niLang.util.NewInstances;
import ni.niUI.IWidgetSink;
import ni.niUI.IWidgetSinkImpl;
import ni.types.IUnknown;

public class WidgetScriptingHostContextHandler implements
		IScriptingHostContextHandler {
	
	public static final String CONTEXT_NAME = "UIWidget";

	@Override
	public IUnknown evalImpl(String className) throws ClassNotFoundException,
			NoSuchMethodException, IllegalAccessException,
			InvocationTargetException, InstantiationException {
		
		final Object o = NewInstances.newInstance(className);
        return IWidgetSink.impl((IWidgetSinkImpl)o);
	}

}
