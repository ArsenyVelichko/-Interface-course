#include "View.h"
#include <assert.h>
#include "Resource.h"

View* createView(TextData* textData, HWND hwnd) {
  if (!textData) { return NULL; }

  View* view = calloc(1, sizeof(View));
  if (!view) { return NULL; }

  //Get text metrics
  HDC hdc = GetDC(hwnd);
  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);

  //Set appropriate window and data
  view->hwnd = hwnd;
  //Set size of characters and position of the scrolls
  view->xChar = tm.tmAveCharWidth;
  view->yChar = tm.tmHeight + tm.tmExternalLeading;
  view->textData = textData;

  ReleaseDC(hwnd, hdc);
  return view;
}


void drawView(View* view) {
  if (!view) { return; }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(view->hwnd, &ps);

  TextData* td = view->textData;
  //Calculation of range of lines requiring redrawing
  int startPos = view->vScrollPos + ps.rcPaint.top / view->yChar;
  int endPos = min(view->vScrollMax, view->vScrollPos + ps.rcPaint.bottom / view->yChar + 1);

  //Line by line output
  for (int i = startPos; i < endPos; i++) {
    int x = -view->hScrollPos;
    int y = view->yChar * (i - view->vScrollPos);

    int charLen = view->lineBegin[i + 1] - view->lineBegin[i];
    TextOutA(hdc, x, y, td->strings + view->lineBegin[i], charLen);
  }
  EndPaint(view->hwnd, &ps);
}

static int topCharIndex(View* view) {
  if (!view->lineBegin) {
    return view->textData->strBegin[view->vScrollPos];
  } else {
    return view->lineBegin[view->vScrollPos];
  }
}

static void shrinkToFit(View* view) {
  TextData* td = view->textData;
  int maxCharLen = view->xClient / view->xChar;

  //find position of upper string
  int topChar = topCharIndex(view);

  //Calculate number of lines corresponding to current width of client window
  view->vScrollMax = 0;
  for (int i = 0; i < td->strCount; i++) {
    int strLen = td->strBegin[i + 1] - td->strBegin[i] - 1;
    int subStrNum = max(0, strLen - 1) / maxCharLen + 1;
    view->vScrollMax += subStrNum;
  }

  //Allocate the required memory space for indexes of line beginnings
  int* tmp = realloc(view->lineBegin, (view->vScrollMax + 1) * sizeof(int));
  if (!tmp) { return; }
  view->lineBegin = tmp;

  //Allocate the required memory space for indexes of line beginnings
  BOOL newUpFound = FALSE;
  for (int i = 0, j = 0; i < td->strCount; i++) {
    int strLen = max(1, td->strBegin[i + 1] - td->strBegin[i] - 1);
    for (int k = 0; k < strLen; k += maxCharLen, j++) {
      view->lineBegin[j] = td->strBegin[i] + k;

      if (!newUpFound && view->lineBegin[j] > topChar) {
        view->vScrollPos = j - 1;
        newUpFound = TRUE;
      }
    }
  }
  view->lineBegin[view->vScrollMax] = td->strBegin[td->strCount];
  if (!newUpFound) { view->vScrollPos = view->vScrollMax - 1; }
}

BOOL resizeView(View* view, int newWidth, int newHeight) {
  if (!view) { return FALSE; }
  if (!newWidth || !newHeight) { return FALSE; }

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

//TODO: Replace by Binary search
static int findOrderedPlace(int elem, int* arr, int size) {
  for (int i = 0; i < size; i++) {
    if (elem < arr[i]) {
      return i - 1;
    }
  }
  return size;
}

void showView(View* view, BOOL wrapFlag) {
  if (!view) { return; }

  view->symbolWrap = wrapFlag;
  if (!wrapFlag) {
    int topChar = topCharIndex(view);
    free(view->lineBegin);

    TextData* td = view->textData;
    view->hScrollMax = td->maxLen * view->xChar;
    //save position on index of upper string
    view->vScrollPos = findOrderedPlace(topChar, td->strBegin, td->strCount + 1);

    view->lineBegin = td->strBegin;
    view->vScrollMax = td->strCount;

  } else {
    view->lineBegin = NULL;
    view->hScrollMax = 0;
  }

  RECT currRc;
  GetClientRect(view->hwnd, &currRc);
  resizeView(view, currRc.right, currRc.bottom);
  InvalidateRect(view->hwnd, NULL, TRUE);
}

void freeView(View* view) {
  if (view && view->symbolWrap) {
    free(view->lineBegin);
  }
  free(view);
}
