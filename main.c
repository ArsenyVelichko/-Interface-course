#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "TextData.h"
#include "View.h"
#include "Resource.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nCmdShow) {
  HWND hwnd;               /* This is the handle for our window */
  MSG messages;            /* Here messages to the application are saved */
  WNDCLASSEX wincl;        /* Data structure for the windowclass */

  /* The Window structure */
  wincl.hInstance = hThisInstance;
  wincl.lpszClassName = szClassName;
  wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
  wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
  wincl.cbSize = sizeof(WNDCLASSEX);

  /* Use default icon and mouse-pointer */
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.lpszMenuName = L"Interface";
  wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;                      /* structure or the window instance */
  /* Use Windows's default colour as the background of the window */
  wincl.hbrBackground = CreateSolidBrush(0x00FFFFFF);

  /* Register the window class, and if it fails quit the program */
  if (!RegisterClassEx(&wincl))
    return 0;

  /* The class is registered, let's create the program*/
  hwnd = CreateWindowEx(
    0,                   /* Extended possibilites for variation */
    szClassName,         /* Classname */
    _T("Code::Blocks Template Windows App"),       /* Title Text */
    WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, /* default window */
    CW_USEDEFAULT,       /* Windows decides the position */
    CW_USEDEFAULT,       /* where the window ends up on the screen */
    544,                 /* The programs width */
    375,                 /* and height in pixels */
    HWND_DESKTOP,        /* The window is a child-window to desktop */
    NULL,                /* No menu */
    hThisInstance,       /* Program Instance handler */
    NULL         /* No Window Creation data */
  );

  /* Make the window visible on the screen */
  ShowWindow(hwnd, nCmdShow);

  /* Run the message loop. It will run until GetMessage() returns 0 */
  while (GetMessage(&messages, NULL, 0, 0)) {
    /* Translate virtual-key messages into character messages */
    TranslateMessage(&messages);
    /* Send message to WindowProcedure */
    DispatchMessage(&messages);
  }

  /* The program return-value is 0 - The value that PostQuitMessage() gave */
  return messages.wParam;
}

static int defineVScrollInc(WPARAM wParam, const View* view) {
  if (!view) { return 0; }

  int vScrollInc;
  switch (LOWORD(wParam)) {
  case SB_TOP:
    vScrollInc = -view->vScrollPos;
    break;

  case SB_BOTTOM:
    vScrollInc = view->vScrollMax - view->vScrollPos;
    break;

  case SB_LINEUP:
    vScrollInc = -1;
    break;

  case SB_LINEDOWN:
    vScrollInc = 1;
    break;

  case SB_PAGEUP:
    vScrollInc = min(-1, -view->yClient / view->yChar);
    break;

  case SB_PAGEDOWN:
    vScrollInc = max(1, view->yClient / view->yChar);
    break;

  case SB_THUMBTRACK:
    vScrollInc = HIWORD(wParam) - view->vScrollPos;
    break;

  default:
    vScrollInc = 0;
  }
  return vScrollInc;
}

static int defineHScrollInc(WPARAM wParam, const View* view) {
  if (!view) { return 0; }

  int hScrollInc;
  switch (LOWORD(wParam)) {
  case SB_LINEUP:
    hScrollInc = -1;
    break;

  case SB_LINEDOWN:
    hScrollInc = 1;
    break;

  case SB_THUMBTRACK:
    hScrollInc = HIWORD(wParam) - view->hScrollPos;
    break;

  default:
    hScrollInc = 0;
  }
  return hScrollInc;
}

static BOOL openFile(HWND hwnd, View** view, TextData** textData) {
  OPENFILENAMEA ofn;
  char szFile[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.lpstrFilter = "Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile[0] = '\0';
  ofn.nMaxFile = sizeof(szFile);
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameA(&ofn)) {
    *textData = createTextData(ofn.lpstrFile);
    *view = createView(*textData, TRUE, hwnd);

    RECT currRc;
    GetClientRect(hwnd, &currRc);
    resizeView(*view, currRc.right, currRc.bottom);
    InvalidateRect(hwnd, &currRc, TRUE);
    return TRUE;
  }
  return FALSE;
}

static BOOL processCommand(HWND hwnd, WPARAM wParam, View** view, TextData** textData) {
  BOOL result = FALSE;
  switch (LOWORD(wParam)) {
  case IDM_OPENFILE:
  {
    result = openFile(hwnd, view, textData);
    break;
  }
  case IDM_LINEBREAK:
  {
    HMENU hMenu = GetMenu(hwnd);
    if (GetMenuState(hMenu, IDM_LINEBREAK, MF_BYCOMMAND) == MF_CHECKED) {
      CheckMenuItem(hMenu, IDM_LINEBREAK, MF_UNCHECKED);
      (*view)->lineBreak = FALSE;

    } else {
      CheckMenuItem(hMenu, IDM_LINEBREAK, MF_CHECKED);
      (*view)->lineBreak = TRUE;
    }
  }
  default:
  {
    break;
  }
  }
  return result;
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  static TextData* textData = NULL;
  static View* view = NULL;

  switch (message)                  /* handle the messages */
  {
  case WM_SIZE:
  {
    resizeView(view, LOWORD(lParam), HIWORD(lParam));
    break;
  }
  case WM_VSCROLL:
  {
    int inc = defineVScrollInc(wParam, view);
    scrollViewV(view, inc);
    break;
  }
  case WM_HSCROLL:
  {
    int inc = defineHScrollInc(wParam, view);
    scrollViewH(view, inc);
    break;
  }
  case WM_PAINT:
  {
    drawView(view);
    break;
  }
  case WM_COMMAND:
  {
    processCommand(hwnd, wParam, &view, &textData);
    break;
  }
  case WM_DESTROY:
  {
    freeTextData(textData);
    freeView(view);
    PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
    break;
  }
  default:                      /* for messages that we don't deal with */
  {
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  }

  return 0;
}
