// Disable warnings produced by external libs
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "../../../external/lib/webview/webview.h"

#pragma GCC diagnostic pop // Enable all warnings

#ifdef __linux__
#include <gtk/gtk.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__)
#import <AppKit/NSWindow.h>
#endif

inline void setup_webview_env() {
  #ifdef __linux__
  setenv("WEBKIT_DISABLE_DMABUF_RENDERER", "1", 1); // This is needed in Wayland (if this isn't set the app crashes on Wayland)
  setenv("GDK_BACKEND", "x11", 1); // This allows the window icon to be hidden in Wayland
  #endif
}

inline void init_window(webview::webview& w, std::string& url) {
  if (url.find("https://") != 0) {
    url = "https://" + url;
  }

  w.set_title("Melodia - Playlist scraper");
  w.set_size(1, 1, WEBVIEW_HINT_NONE);

  // Hide icon in taskbar
  #ifdef __linux__
  auto window_res = w.window();
  if (window_res.has_value()) {
    void* raw_window = window_res.value();
    if (raw_window) {
      GtkWindow* gtk_win = static_cast<GtkWindow*>(raw_window);
      gtk_window_set_skip_taskbar_hint(gtk_win, TRUE);
    }
  }
  #endif
  #if defined(_WIN32)
  auto window_res = w.window();
  if (window_res.has_value()) {
    HWND hwnd = static_cast<HWND>(window_res.value());
    if (hwnd) {
      LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
      SetWindowLongPtr(hwnd, GWL_EXSTYLE, (exStyle | WS_EX_TOOLWINDOW) & ~WS_EX_APPWINDOW);
    }
  }
  #endif
  #if defined(__APPLE__)
  auto window_res = w.window();
  if (window_res.has_value()) {
    void* native_win = window_res.value();
    if (native_win) {
      id window = static_cast<id>(native_win);
      [window setCollectionBehavior:NSWindowCollectionBehaviorTransient | NSWindowCollectionBehaviorIgnoresCycle];
    }
  }
  #endif
}
