package ni.niLang.util;

import java.lang.reflect.InvocationTargetException;

import ni.types.IUnknown;

public interface IScriptingHostContextHandler {
	public IUnknown evalImpl(final String className) throws ClassNotFoundException, NoSuchMethodException, IllegalAccessException, InvocationTargetException, InstantiationException;
}
