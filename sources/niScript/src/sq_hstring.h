#if !defined __sq_HSTRING_H__ || defined __MODULE_HSTRING_TABLE_IMPL__
#undef  __sq_HSTRING_H__
#define __sq_HSTRING_H__

#undef _HDecl_
#undef _HDecl
#undef _HC

#if defined __MODULE_HSTRING_TABLE_IMPL__
#pragma message("-- HString Table implementation --")

#define _HDecl_(X,STRING) ni::tHStringPtr _hstr_##X = _H(STRING)
#define _HDecl(X) _HDecl_(X,#X)

#else

#define _HDecl_(X,STRING)                       \
  extern ni::tHStringPtr _hstr_##X
#define _HDecl(X)                               \
  extern ni::tHStringPtr _hstr_##X

#endif

#define _HC(X) _hstr_##X

_HDecl(_all);
_HDecl(_pedantic);
_HDecl(_experimental);
_HDecl(__debug_name);
_HDecl(__lint);
_HDecl(no_debug_name);
_HDecl(IDENTIFIER);
_HDecl(STRING_LITERAL);
_HDecl(INTEGER);
_HDecl(FLOAT);
_HDecl(NEWSLOT);
_HDecl(this);
_HDecl(compilecontext);
_HDecl_(AT_ITERATOR_AT,"@ITERATOR@");
_HDecl_(AT_INDEX_AT,"@INDEX@");
_HDecl(func);
_HDecl(src);
_HDecl(line);
_HDecl(locals);
_HDecl(Import);
_HDecl(NewImport);
_HDecl(method);
_HDecl(Vec2);
_HDecl(Vec3);
_HDecl(Vec4);
_HDecl(Matrix);
_HDecl(UUID);
_HDecl(__imports);
_HDecl_(NOT_ITOSTRING,"[not iToString]");
_HDecl(e);

#endif // __sq_HSTRING_H__
