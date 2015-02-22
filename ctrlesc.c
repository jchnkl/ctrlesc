#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

// for restoring auto repeat
Display * g_dpy;
XKeyboardState g_keyboard_state;

static void
handler(int signum)
{
  XKeyboardControl kc;

  if (g_keyboard_state.global_auto_repeat) {
    kc.key = XK_Escape;
    kc.auto_repeat_mode = AutoRepeatModeOff;
    XChangeKeyboardControl(g_dpy, KBKey | KBAutoRepeatMode, &kc);
  } else {
    // stub, until i find out how to use XKeyboardState to enable auto repeat
    // for single keys again
  }

  exit(EXIT_SUCCESS);
}

int
main(int argc, char ** argv)
{
  Display * dpy = XOpenDisplay("");
  Window root = DefaultRootWindow(dpy);
  KeyCode kc_escape = XKeysymToKeycode(dpy, XK_Escape);

  Window focus = None;
  Bool want_ctrl = False;

  g_dpy = dpy;
  XGetKeyboardControl(dpy, &g_keyboard_state);

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGINT, &sa, NULL) == -1
      || sigaction(SIGTERM, &sa, NULL) == -1
      // || sigaction(SIGKILL, &sa, NULL) == -1
     ) {
    fprintf(stderr, "Couldn't install signal handler\n");
    exit(EXIT_FAILURE);
  }

  // XKeyboardControl kc;
  // kc.key = kc_escape;
  // kc.auto_repeat_mode = AutoRepeatModeOn;
  // XChangeKeyboardControl(dpy, KBKey | KBAutoRepeatMode, &kc);
  // exit(0);

  XKeyboardControl kc;
  kc.key = kc_escape;
  kc.auto_repeat_mode = AutoRepeatModeOff;
  XChangeKeyboardControl(dpy, KBKey | KBAutoRepeatMode, &kc);

  XGrabKey(dpy, kc_escape, 0, root, False, GrabModeAsync, GrabModeAsync);

  XEvent event;
  while (1) {
    XNextEvent(dpy, &event);

    if (event.type == KeyPress) {
      XKeyEvent * e = &event.xkey;
        fprintf(stderr, "KeyPress: %s\n",
                XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)));


      if (e->keycode == kc_escape) {
        int revert;
        XGetInputFocus(dpy, &focus, &revert);

        XGrabKeyboard(dpy, focus, False, GrabModeAsync, GrabModeAsync, CurrentTime);

        fprintf(stderr, "grabbed on %u\n", // release on window %u, subwindow %u, focus %u\n",
                // XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)),
                // (uint)e->window,
                // (uint)e->subwindow,
                (uint)focus);

      } else if (focus != None) {

        fprintf(stderr, "%s press   on window %u, subwindow %u, focus %u\n",
                XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)),
                (uint)e->window,
                (uint)e->subwindow,
                (uint)focus);

        want_ctrl = True;

        e->window = focus;
        e->state |= ControlMask;
        e->time = CurrentTime;
        e->send_event = True;

        XSendEvent(dpy, focus, True, KeyPressMask, (XEvent *)&event);
      }

    } else if (focus != None && event.type == KeyRelease) {
      XKeyEvent * e = &event.xkey;

      if (e->keycode == kc_escape) {
        if (! want_ctrl) {

          fprintf(stderr, "%s release on window %u, subwindow %u, focus %u\n",
                  XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)),
                  (uint)e->window,
                  (uint)e->subwindow,
                  (uint)focus);

          e->type = KeyPress;
          e->window = focus;
          e->keycode = kc_escape;
          e->time = CurrentTime;
          e->send_event = True;

          XSendEvent(dpy, focus, True, KeyPressMask, (XEvent *)&event);

          e->type = KeyRelease;
          XSendEvent(dpy, focus, True, KeyReleaseMask, (XEvent *)&event);
        }

        focus = None;
        want_ctrl = False;
        XUngrabKeyboard(dpy, CurrentTime);
      }
    }
  }

  return 0;
}

// {
//   Display * dpy = XOpenDisplay("");
//   Window root = DefaultRootWindow(dpy);
//
//   KeyCode kc_escape = 0;
// #ifdef DEBUG
//   int debug = 0;
// #endif
//   int is_escape = False;
//   int not_released = False;
//
//   kc_escape = XKeysymToKeycode(dpy, XK_Escape);
//   XGrabKey(dpy, kc_escape, 0, root, False, GrabModeAsync, GrabModeAsync);
//
// #ifdef DEBUG
//   if (argc == 2 && 0 == strncmp("-d", argv[1], strlen(argv[1]))) {
//     debug = 1;
//   }
// #endif
//
//   XEvent event;
//   while (1) {
//     XNextEvent(dpy, &event);
//
//     if (event.type == KeyPress) {
//       XKeyEvent * e = &event.xkey;
//
//       Window focus;
//       int revert;
//       XGetInputFocus(dpy, &focus, &revert);
//
// #ifdef DEBUG
//       if (debug) {
//         fprintf(stderr, "%s press   on window %u, subwindow %u, focus %u\n",
//                 XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)),
//                 (uint)e->window,
//                 (uint)e->subwindow,
//                 (uint)focus);
//       }
// #endif
//
//       if (e->keycode == kc_escape) {
//         is_escape = True;
//
//         // if (! not_released) { TODO: FIXME:
//         // M_X FOO_X W_X
//           XGrabKeyboard(dpy, focus, False,
//                         GrabModeAsync, GrabModeAsync, CurrentTime);
//         // }
//
//         // not_released = True;
//
//       } else {
//         is_escape = False;
//
//         XUngrabKeyboard(dpy, CurrentTime);
//
//         e->window = focus;
//         e->state |= ControlMask;
//         e->time = CurrentTime;
//         e->send_event = True;
//
//         XSendEvent(dpy, focus, True, KeyPressMask, (XEvent *)&event);
//       }
//
//     } else if (event.type == KeyRelease) {
//       XKeyEvent * e = &event.xkey;
//
//       XUngrabKeyboard(dpy, CurrentTime);
//
//       Window focus;
//       int revert;
//       XGetInputFocus(dpy, &focus, &revert);
//
// #ifdef DEBUG
//       if (debug) {
//         fprintf(stderr, "%s release on window %u, subwindow %u, focus %u\n",
//                 XKeysymToString(XKeycodeToKeysym(dpy, e->keycode, 0)),
//                 (uint)e->window,
//                 (uint)e->subwindow,
//                 (uint)focus);
//       }
// #endif
//
//       e->time = CurrentTime;
//       e->send_event = True;
//
//       if (is_escape) {
//         e->type = KeyPress;
//         e->keycode = kc_escape;
//         XSendEvent(dpy, focus, True, KeyPressMask, (XEvent *)&event);
//         e->type = KeyRelease;
//       }
//
//       XSendEvent(dpy, focus, True, KeyReleaseMask, (XEvent *)&event);
//     }
//   }
//
//   return 0;
// }
