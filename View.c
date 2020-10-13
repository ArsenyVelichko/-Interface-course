#include "View.h"
#include <assert.h>

View* createView(TextData* textData, HWND hwnd) {
  if (!textData) { return NULL; }

  View* view = malloc(sizeof(View));
  if (!view) { return NULL; }

  HDC hdc = GetDC(hwnd);
  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);

  view->hwnd = hwnd;
  view->xChar = tm.tmAveCharWidth;
  view->yChar = tm.tmHeight + tm.tmExternalLeading;
  view->textData = textData;
  view->vScrollPos = 0;
  view->hScrollPos = 0;

  ReleaseDC(hwnd, hdc);
  return view;
}

void drawView(View* view) {
  if (!view) { return; }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(view->hwnd, &ps);
  
  TextData* td = view->textData;
  int startPos = max(0, view->vScrollPos + ps.rcPaint.top / view->yChar);
  int endPos = min(td->strCount, view->vScrollPos + ps.rcPaint.bottom / view->yChar + 1);

  for (int i = startPos; i < endPos; i++) {
    int x = view->xChar * (-view->hScrollPos);
    int y = view->yChar * (i - view->vScrollPos);

    int len = td->strBegin[i + 1] - td->strBegin[i];
    char* str = td->strings + td->strBegin[i];

    TextOutA(hdc, x, y, str, len);
  }
  EndPaint(view->hwnd, &ps);
}

void resizeView(View* view, int newWidth, int newHeight) {
  if (!view) { return; }
  int prevVPos = view->vScrollPos;
  int prevHPos = view->hScrollPos;

  view->xClient = newWidth;
  view->yClient = newHeight;

  view->vScrollMax = max(0, view->textData->strCount - newHeight / view->yChar);
  view->vScrollPos = min(view->vScrollPos, view->vScrollMax);

  SetScrollRange(view->hwnd, SB_VERT, 0, view->vScrollMax, FALSE);
  SetScrollPos(view->hwnd, SB_VERT, view->vScrollPos, TRUE);

  int maxWidth = view->xChar * view->textData->maxLen;
  view->hScrollMax = max(0, (maxWidth - newWidth) / view->xChar);
  view->hScrollPos = min(view->hScrollPos, view->hScrollMax);

  SetScrollRange(view->hwnd, SB_HORZ, 0, view->hScrollMax, FALSE);
  SetScrollPos(view->hwnd, SB_HORZ, view->hScrollPos, TRUE);

  if (prevVPos != view->vScrollPos || prevHPos != view->hScrollPos) {
    RECT clientRc = {0, 0, newWidth, newHeight};
    InvalidateRect(view->hwnd, &clientRc, TRUE);
  }
}

static int clamp(int v, int lo, int hi) {
  assert(lo < hi);
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

void scrollViewV(View* view, int inc) {
  inc = clamp(inc, -view->vScrollPos, view->vScrollMax - view->vScrollPos);

  if (inc != 0) {
    view->vScrollPos += inc;
    ScrollWindow(view->hwnd, 0, -view->yChar * inc, NULL, NULL);
    SetScrollPos(view->hwnd, SB_VERT, view->vScrollPos, TRUE);
    UpdateWindow(view->hwnd);
  }
}

void scrollViewH(View* view, int inc) {
  inc = clamp(inc, -view->hScrollPos, view->hScrollMax - view->hScrollPos);

  if (inc != 0) {
    view->hScrollPos += inc;
    ScrollWindow(view->hwnd, -view->xChar * inc, 0, NULL, NULL);
    SetScrollPos(view->hwnd, SB_HORZ, view->hScrollPos, TRUE);
  }
}