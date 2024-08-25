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
  extern ni::tHStringNN _hstr_##X
#define _HDecl(X)                               \
  extern ni::tHStringNN _hstr_##X

#endif

#define _HC(X) _hstr_##X

_HDecl(_all);
_HDecl(_pedantic);
_HDecl(_experimental);
_HDecl(_explicit);
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
_HDecl(col);
_HDecl(locals);
_HDecl(Import);
_HDecl(NewImport);
_HDecl(method);
_HDecl(interface);
_HDecl(Vec2);
_HDecl(Vec3);
_HDecl(Vec4);
_HDecl(Matrix);
_HDecl(UUID);
_HDecl(__imports);
_HDecl_(NOT_ITOSTRING,"[not iToString]");
_HDecl(e);
_HDecl(error_code);
_HDecl(error_code_unknown_typedef);
_HDecl(error_code_cant_load_module_def);
_HDecl(error_code_cant_find_interface_def);
_HDecl(error_code_cant_find_method_def);
_HDecl(error_code_method_def_invalid_ret_type);
_HDecl(error_code_cant_find_type_uuid);
_HDecl(error_code_lint_call_error);
_HDecl(resolved_type);
_HDecl(QueryInterface);
_HDecl_(typestr_null, "null");
_HDecl_(typestr_int, "int");
_HDecl_(typestr_float, "float");
_HDecl_(typestr_string, "string");
_HDecl_(typestr_table, "table");
_HDecl_(typestr_array, "array");
_HDecl_(typestr_function, "function");
_HDecl_(typestr_userdata, "userdata");
_HDecl_(typestr_vm, "vm");
_HDecl_(typestr_iunknown, "iunknown");
_HDecl_(typestr_closure, "closure");
_HDecl_(typestr_nativeclosure, "nativeclosure");
_HDecl_(typestr_functionproto, "functionproto");
_HDecl_(typestr_indexed_property, "indexed_property");
_HDecl_(typestr_iterator, "iterator");
_HDecl_(typestr_enum, "enum");
_HDecl_(typestr_property, "property");
_HDecl_(typestr_invalid, "invalid");

#endif // __sq_HSTRING_H__
