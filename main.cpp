#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>

#include <memory>

#include <iostream>

#include <Windows.h>
#pragma comment(lib, "user32.lib")

static HHOOK kLLKbdHook;

static void fillInput(INPUT& input, WPARAM w_param, WORD unicode_key) {
  input.type = INPUT_KEYBOARD;
  input.ki.time = 0;
  input.ki.dwExtraInfo = 0;
  input.ki.wVk = 0;
  input.ki.dwFlags = KEYEVENTF_UNICODE;
  if (w_param == WM_KEYUP) {
    input.ki.dwFlags |= KEYEVENTF_KEYUP;
  }
  input.ki.wScan = unicode_key;
}

static LRESULT CALLBACK ProcessLLKbd(int code, WPARAM w_param, LPARAM l_param) {
  if (code != HC_ACTION) {
    return CallNextHookEx(kLLKbdHook, code, w_param, l_param);
  }

  KBDLLHOOKSTRUCT* kbd_struct = (KBDLLHOOKSTRUCT *)l_param;
  if (kbd_struct && w_param == WM_KEYDOWN) {
    SHORT ctrl_state = GetKeyState(VK_CONTROL);
    SHORT alt_state = GetKeyState(VK_MENU);

    if (ctrl_state < 0 && alt_state < 0) {
      SHORT shift_state = GetKeyState(VK_SHIFT);
      INPUT input;
      switch (kbd_struct->vkCode) {
      case 0x53: // b
        fillInput(input, w_param, 0xDF); //ß
        SendInput(1, &input, sizeof(INPUT));
        return 1;
      case 0x41: // a
        if (shift_state < 0) {
          fillInput(input, w_param, 0xC4); //Ä
        } else {
          fillInput(input, w_param, 0xE4); //ä
        }
        SendInput(1, &input, sizeof(INPUT));
        return 1;
      case 0x4F: // o
        if (shift_state < 0) {
          fillInput(input, w_param, 0xD6); //Ö
        } else {
          fillInput(input, w_param, 0xF6); //ö
        }
        SendInput(1, &input, sizeof(INPUT));
        return 1;
      case 0x55: // u
        if (shift_state < 0) {
          fillInput(input, w_param, 0xDC); //Ü
        } else {
          fillInput(input, w_param, 0xFC); //ü
        }
        SendInput(1, &input, sizeof(INPUT));
        return 1;
      default:
        break;
      }
    }
  }
  return CallNextHookEx(kLLKbdHook, code, w_param, l_param);
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  auto quit_action = std::make_unique<QAction>("Quit");
  QObject::connect(quit_action.get(), &QAction::triggered, qApp, &QCoreApplication::quit);

  auto tray_menu = std::make_unique<QMenu>();
  tray_menu->addAction(quit_action.get());

  auto tray_icon = std::make_unique<QSystemTrayIcon>(QIcon(":/bad.png"));
  tray_icon->setContextMenu(tray_menu.get());
  tray_icon->setVisible(true);

  QApplication::setQuitOnLastWindowClosed(false);

  kLLKbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, ProcessLLKbd, nullptr, 0);

  QObject::connect(qApp, &QApplication::aboutToQuit, []() {
    UnhookWindowsHookEx(kLLKbdHook);
  });
  return a.exec();
}
