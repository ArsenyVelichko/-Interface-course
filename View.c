#include "View.h"
#include <assert.h>
#include "Resource.h"

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
  int startPos = view->vScrollPos + ps.rcPaint.top / view->yChar;
  int endPos = min(view->vScrollMax, view->vScrollPos + ps.rcPaint.bottom / view->yChar + 1);

  for (int i = startPos; i < endPos; i++) {
    int x = -view->hScrollPos;
    int y = view->yChar * (i - view->vScrollPos);

    int charLen = view->lineBegin[i + 1] - view->lineBegin[i];
    TextOutA(hdc, x, y, td->strings + view->lineBegin[i], charLen);
  }
  EndPaint(view->hwnd, &ps);
}

static void shrinkToFit(View* view) {
  TextData* td = view->textData;
  int maxCharLen = view->xClient / view->xChar;

  view->vScrollMax = 0;
  for (int i = 0; i < td->strCount; i++) {
    int strLen = td->strBegin[i + 1] - td->strBegin[i] - 1;
    int subStrNum = (strLen - 1) / maxCharLen + 1;
    view->vScrollMax += subStrNum;
  }
  
  int* tmp = realloc(view->lineBegin, (view->vScrollMax + 1) * sizeof(int));
  if (!tmp) { return; }
  view->lineBegin = tmp;

  for (int i = 0, j = 0; i < td->strCount; i++) {
    int strLen = max(1, td->strBegin[i + 1] - td->strBegin[i] - 1);
    for (int k = 0; k < strLen; k += maxCharLen) {
      view->lineBegin[j++] = td->strBegin[i] + k;
    }
  }
  view->lineBegin[view->vScrollMax] = td->strBegin[td->strCount];
}

BOOL resizeView(View* view, int newWidth, int newHeight) {
  if (!view) { return FALSE; }

  TextData* td = view->textData;
  view->xClient = newWidth;
  view->yClient = newHeight;

  BOOL isInvalid = FALSE;
  if (view->symbolWrap) {
    shrinkToFit(view);
    isInvalid = TRUE;
  }

  int strOccupied = newHeight / view->yChar;
  int maxPos = max(0, view->vScrollMax - strOccupied);
  if (view->vScrollPos > maxPos) {
    isInvalid = TRUE;
    view->vScrollPos = maxPos;
  }

  maxPos = max(0, view->hScrollMax - newWidth);
  if (view->hScrollPos > maxPos) {
    isInvalid = TRUE;
    view->hScrollPos = maxPos;
  }

  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  si.nMin = 0;
  si.nMax = view->vScrollMax;
  si.nPos = view->vScrollPos;
  si.nPage = strOccupied + 1;
  SetScrollInfo(view->hwnd, SB_VERT, &si, TRUE);

  si.nMax = view->hScrollMax;
  si.nPos = view->hScrollPos;
  si.nPage = view->xClient;
  SetScrollInfo(view->hwnd, SB_HORZ, &si, TRUE);
  return isInvalid;
}

static int clamp(int v, int lo, int hi) {
  assert(lo <= hi);
  return (v < lo) ? lo : (hi < v) ? hi : v;
}

void scrollViewV(View* view, int inc) {
  if (!view) { return; }

  int strOccupied = view->yClient / view->yChar;
  int maxPos = max(0, view->vScrollMax - strOccupied);
  inc = clamp(inc, -view->vScrollPos, maxPos - view->vScrollPos);

  if (inc != 0) {
    view->vScrollPos += inc;
    ScrollWindow(view->hwnd, 0, -view->yChar * inc, NULL, NULL);
    SetScrollPos(view->hwnd, SB_VERT, view->vScrollPos, TRUE);
    UpdateWindow(view->hwnd);
  }
}

void scrollViewH(View* view, int inc) {
  if (!view) { return; }

  int maxPos = max(0, view->hScrollMax - view->xClient);
  inc = clamp(inc, -view->hScrollPos, maxPos - view->hScrollPos);

  if (inc != 0) {
    view->hScrollPos += inc;
    ScrollWindow(view->hwnd, -inc, 0, NULL, NULL);
    SetScrollPos(view->hwnd, SB_HORZ, view->hScrollPos, TRUE);
  }
}

void showView(View* view, BOOL wrapFlag) {
  if (!view) { return; }

  view->symbolWrap = wrapFlag;
  RECT currRc;
  GetClientRect(view->hwnd, &currRc);
  if (!wrapFlag) {
    TextData* td = view->textData;
    view->hScrollMax = td->maxLen * view->xChar;
    view->lineBegin = td->strBegin;
    view->vScrollMax = td->strCount;

  } else {
    view->lineBegin = NULL;
    view->hScrollMax = 0;
  }

  resizeView(view, currRc.right - currRc.left, currRc.bottom - currRc.top);
  InvalidateRect(view->hwnd, NULL, TRUE);
}

void freeView(View* view) {
  if (view && view->symbolWrap) {
    free(view->lineBegin);
  }
  free(view);
}