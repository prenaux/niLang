package ni.android.util;

import ni.niLang.IFile;
import ni.niLang.IURLFileHandler;
import ni.niLang.IURLFileHandlerImpl;
import ni.niLang.Lib;

public class URLFileHandlerPrefixed implements IURLFileHandlerImpl {
	final IURLFileHandler mHandler;
	final String mPrefix;

	URLFileHandlerPrefixed(IURLFileHandler handler, final String prefix) {
		mHandler = handler;
		mPrefix = prefix;
   	Lib.getLang().setGlobalInstance("URLFileHandler."+prefix, IURLFileHandler.impl(this));
	}

	String replacePrefix(String aURL) {
		return aURL.replace(mPrefix+"://",mPrefix +"/");
	}

	@Override
	public IFile urlopen(final String aURL) {
		return mHandler.urlopen(replacePrefix(aURL));
	}

	@Override
	public boolean urlexists(final String aURL) {
		return mHandler.urlexists(replacePrefix(aURL));
	}

	public static void create(IURLFileHandler handler, String name) {
		Lib.getLang().setGlobalInstance("URLFileHandler."+ name, IURLFileHandler.impl(new URLFileHandlerPrefixed(handler,name)));
	}
}
