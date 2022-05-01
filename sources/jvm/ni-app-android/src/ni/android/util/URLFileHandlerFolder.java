package ni.android.util;

import ni.niLang.IFile;
import ni.niLang.IURLFileHandler;
import ni.niLang.IURLFileHandlerImpl;
import ni.niLang.Lib;

public class URLFileHandlerFolder implements IURLFileHandlerImpl {
	final IURLFileHandler mHandler;
	final String mFolderName;

	URLFileHandlerFolder(IURLFileHandler handler, final String folderName) {
		mHandler = handler;
		mFolderName = folderName;
	}

	@Override
	public IFile urlopen(final String aURL) {
    IFile fp = mHandler.urlopen(aURL);
    if (fp == null) {
      fp = mHandler.urlopen(mFolderName + "/" +aURL);
    }
		return fp;
	}

	@Override
	public boolean urlexists(final String aURL) {
    if (!mHandler.urlexists(aURL)) {
      return mHandler.urlexists(mFolderName + "/" +aURL);
    }
		return true;
	}

	public static void create(IURLFileHandler handler, String name) {
		Lib.getLang().setGlobalInstance("URLFileHandler."+ name, IURLFileHandler.impl(new URLFileHandlerFolder(handler,name)));
	}
}
