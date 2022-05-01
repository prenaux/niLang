#ifndef __WIN32_DC_26053814_H__
#define __WIN32_DC_26053814_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni { namespace Windows {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

// Used so we always remember to close the handle.
// The class interface matches that of ScopedStdioHandle in  addition to an
// IsValid() method since invalid handles on windows can be either NULL or
// INVALID_HANDLE_VALUE (-1).
//
// Example:
//   AutoHandle hfile(CreateFile(...));
//   if (!hfile.Get())
//     ...process error
//   ReadFile(hfile.Get(), ...);
//
// To sqirrel the handle away somewhere else:
//   secret__handle = hfile.Take();
//
// To explicitly close the handle:
//   hfile.Close();
class AutoHandle {
 public:
  AutoHandle() : _handle(NULL) {
  }

  explicit AutoHandle(HANDLE h) : _handle(NULL) {
    Set(h);
  }

  ~AutoHandle() {
    Close();
  }

  // Use this instead of comparing to INVALID_HANDLE_VALUE to pick up our NULL
  // usage for errors.
  bool IsValid() const {
    return _handle != NULL;
  }

  void Set(HANDLE new_handle) {
    Close();

    // Windows is inconsistent about invalid handles, so we always use NULL
    if (new_handle != INVALID_HANDLE_VALUE)
      _handle = new_handle;
  }

  HANDLE Get() {
    return _handle;
  }

  operator HANDLE() { return _handle; }

  HANDLE Take() {
    // transfers ownership away from this object
    HANDLE h = _handle;
    _handle = NULL;
    return h;
  }

  void Close() {
    if (_handle) {
      if (!::CloseHandle(_handle)) {
        niAssertUnreachable("Can't close handle.");
      }
      _handle = NULL;
    }
  }

 private:
  HANDLE _handle;
  niClassNoCopyAssign(AutoHandle);
};

// Like AutoHandle but for HDC.  Only use this on HDCs returned from
// CreateCompatibleDC.  For an HDC returned by GetDC, use ReleaseDC instead.
class AutoHDC {
 public:
  AutoHDC() : _hdc(NULL) { }
  explicit AutoHDC(HDC h) : _hdc(h) { }

  ~AutoHDC() {
    Close();
  }

  HDC Get() {
    return _hdc;
  }

  void Set(HDC h) {
    Close();
    _hdc = h;
  }

  operator HDC() { return _hdc; }

 private:
  void Close() {
    if (_hdc)
      DeleteDC(_hdc);
  }

  HDC _hdc;
  niClassNoCopyAssign(AutoHDC);
};

// Like AutoHandle but for GDI objects.
template<class T>
class AutoGDIObject {
 public:
  AutoGDIObject() : _object(NULL) {}
  explicit AutoGDIObject(T object) : _object(object) {}

  ~AutoGDIObject() {
    Close();
  }

  T Get() {
    return _object;
  }

  void Set(T object) {
    if (_object && object != _object)
      Close();
    _object = object;
  }

  AutoGDIObject& operator=(T object) {
    Set(object);
    return *this;
  }

  operator T() { return _object; }

 private:
  void Close() {
    if (_object)
      DeleteObject(_object);
  }

  T _object;
  niClassNoCopyAssign(AutoGDIObject);
};

// Typedefs for some common use cases.
typedef AutoGDIObject<HBITMAP> AutoBitmap;
typedef AutoGDIObject<HRGN> AutoHRGN;
typedef AutoGDIObject<HFONT> AutoHFONT;

// Like AutoHandle except for HGLOBAL.
template<class T>
class AutoHGlobal {
 public:
  explicit AutoHGlobal(HGLOBAL glob) : _glob(glob) {
    _data = static_cast<T*>(GlobalLock(_glob));
  }
  ~AutoHGlobal() {
    GlobalUnlock(_glob);
  }

  T* get() { return _data; }

  size_t Size() const { return GlobalSize(_glob); }

  T* operator->() const  {
    assert(_data != 0);
    return _data;
  }

 private:
  HGLOBAL _glob;

  T* _data;

  niClassNoCopyAssign(AutoHGlobal);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}} // namespace ni { namespace Windows {
#endif // __WIN32_DC_26053814_H__
