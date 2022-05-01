package ni.app;

import java.util.ArrayList;

import ni.common.lang3.StringUtils;
import ni.common.util.Strings;
import ni.niLang.IFile;
import ni.niLang.IURLFileHandler;
import ni.niLang.IURLFileHandlerImpl;

public class URLFileHandler implements IURLFileHandlerImpl {

	final private String SEPARATOR = "://";
	final private String ROOT = ":/";
	final ArrayList<String> _secondaryPaths = new ArrayList<String>();
	final ArrayList<String> _secondaryRootPaths = new ArrayList<String>();
	final ArrayList<IURLFileHandler> _filesHandlers = new ArrayList<IURLFileHandler>();
	private String mRoot = "";

	static private URLFileHandler _resourceHandler = null;
	static public URLFileHandler getInstance()
	{
		if (_resourceHandler == null) {
			_resourceHandler = new URLFileHandler();
		}
		return _resourceHandler;
	}

	URLFileHandler() {
	}

	public void addSource(final IURLFileHandler handler) {
		if (!_filesHandlers.contains(handler)) {
			_filesHandlers.add(0,handler);
		}
	}

	public boolean removeSource(final IURLFileHandler handler)
	{
		if (_filesHandlers.contains(handler)) {
			_filesHandlers.remove(handler);
			return true;
		}
		return false;
	}

	public void addSourceDir(final String dir) {
		_secondaryPaths.add(0,dir);
	}

	public void setRootDir(final String root) {
		mRoot = (root == null) ? StringUtils.EMPTY : root;
	}

	public void addRootSourceDir(final String dir) {
		_secondaryRootPaths.add(0,dir);
	}

	@Override
	public IFile urlopen(final String aURL)
	{
		final String baseFileName = StringUtils.contains(aURL, SEPARATOR) ? Strings.after(aURL, SEPARATOR) : aURL;
		final boolean isAbsolute = StringUtils.contains(baseFileName, ROOT);
		final String root = isAbsolute ? StringUtils.EMPTY : mRoot;
		for (final IURLFileHandler h : _filesHandlers)
		{
			final String _path = root.concat(baseFileName);
			IFile fp = h.urlopen(_path);
			if (fp != null) {
				return fp;
			}

			// Do not check secondary dirs
			// if the path is absolute...
			if (isAbsolute) {
				continue;
			}

			// Check paths which are relative
			// to the root dir (mRoot);
			for (final String s : _secondaryRootPaths)
			{
				final boolean _isAbsolute = StringUtils.contains(s, ROOT);
				final String _root = _isAbsolute ? StringUtils.EMPTY : root;
				final String path = _root.concat(s).concat(baseFileName);
				fp = h.urlopen(path);
				if (fp != null) {
					return fp;
				}
			}

			// Check paths which are related
			// the top dir of the application;
			for (final String s : _secondaryPaths)
			{
				final String path = s.concat(baseFileName);
				fp = h.urlopen(path);
				if (fp != null) {
					return fp;
				}
			}
		}
		return null;
	}

	@Override
	public boolean urlexists(final String aURL)
	{
		final String baseFileName = StringUtils.contains(aURL, SEPARATOR) ? Strings.after(aURL, SEPARATOR) : aURL;
		final boolean isAbsolute = StringUtils.contains(baseFileName, ROOT);
		final String root = isAbsolute ? StringUtils.EMPTY : mRoot;
		for (final IURLFileHandler h : _filesHandlers)
		{
			final String _path = root.concat(baseFileName);
			if (h.urlexists(_path)) {
				return true;
			}

			for (final String s : _secondaryPaths)
			{
				final boolean _isAbsolute = StringUtils.contains(s, ROOT);
				final String _root = _isAbsolute ? StringUtils.EMPTY : root;
				final String path = _root.concat(s).concat(baseFileName);
				if (h.urlexists(path)) {
					return true;
				}
			}
		}
		return false;
	}
};