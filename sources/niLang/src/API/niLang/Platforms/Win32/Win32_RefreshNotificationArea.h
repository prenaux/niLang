#pragma once
#ifndef __WIN32_REFRESHNOTIFICATIONAREA_H_CE4C91EB_256E_4F1F_A460_6D56A502234E__
#define __WIN32_REFRESHNOTIFICATIONAREA_H_CE4C91EB_256E_4F1F_A460_6D56A502234E__

namespace ni { namespace Windows {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

#define FW(x,y) ::FindWindowExW(x, NULL, y, L"")
void RefreshNotificationArea()
{
  HWND hNotificationArea;
  RECT r;

  GetClientRect(
      hNotificationArea = ::FindWindowExW(
          FW(FW(FW(NULL, L"Shell_TrayWnd"), L"TrayNotifyWnd"), L"SysPager"),
          NULL,
          L"ToolbarWindow32",
          // L"Notification Area"), // Windows XP
          L"User Promoted Notification Area"), // Windows 7 and up
      &r);

  for (LONG x = 0; x < r.right; x += 5)
    for (LONG y = 0; y < r.bottom; y += 5)
      niWin32API(SendMessage)(
          hNotificationArea,
          WM_MOUSEMOVE,
          0,
          (y << 16) + x);
}
#undef FW

/**@}*/
/**@}*/
}} // namespace ni { namespace Windows {
#endif // __WIN32_REFRESHNOTIFICATIONAREA_H_CE4C91EB_256E_4F1F_A460_6D56A502234E__
