// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("niUI")

window <- {
  size = null
  position = null
  show_flags = 0
  fullscreen = false
}

misc <- {
  ui_skin = "Default"
  default_config = null
  load_save_image_map = false // Load/Save the default image map (in SDK/data/base/niUI.imagemap)
  drop_target = false
  can_fullscreen = true
  refresh_timer = null        // null lets the application decide the refresh timer
  fullscreen_refresh_timer = null
  no_close_popup = false
  window_show_flags = 0
  window_fullscreen = false
}

historyCommands <- [
  "::gui.toggleDrawOpCapture()"
  "::gGraphics.ClearAllFontCaches()"
]
