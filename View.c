#include "View.h"
#include <assert.h>
#include "Resource.h"

static HFONT createFont() {
  LOGFONTA lf;

  lf.lfHeight = 15;
  lf.lfWidth = 10;
  lf.lfEscapement = 0;
  lf.lfOrientation = 0;
  lf.lfWeight = FW_NORMAL;
  lf.lfItalic = FALSE;
  lf.lfUnderline = FALSE;
  lf.lfStrikeOut = FALSE;
  lf.lfCharSet = ANSI_CHARSET;
  lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  lf.lfQuality = DEFAULT_QUALITY;
  lf.lfPitchAndFamily = FIXED_PITCH;
  strcpy_s(lf.lfFaceName, sizeof(lf.lfFaceName), "Courier New");
  //strcpy(lf.lfFaceName, "Courier New");

  return CreateFontIndirectA(&lf);
}

View* createView(TextData* textData, BOOL lineBreak, HWND hwnd) {
  if (!textData) { return NULL; }

  View* view = malloc(sizeof(View));
  if (!view) { return NULL; }

  HDC hdc = GetDC(hwnd);
  HFONT prevFont = SelectObject(hdc, createFont());

  TEXTMETRIC tm;
  GetTextMetrics(hdc, &tm);

  view->hwnd = hwnd;
  view->xChar = tm.tmAveCharWidth;
  view->yChar = tm.tmHeight + tm.tmExternalLeading;
  view->textData = textData;
  view->vScrollPos = 0;
  view->hScrollPos = 0;
  view->dataPixelWidth = textData->maxLen * view->xChar;
  view->lineBreak = lineBreak;
  view->font = SelectObject(hdc, prevFont);

  ReleaseDC(hwnd, hdc);
  return view;
}

void drawView(View* view) {
  if (!view) { return; }

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(view->hwnd, &ps);
  SelectObject(hdc, view->font);

  TextData* td = view->textData;
  int startPos = max(0, view->vScrollPos + ps.rcPaint.top / view->yChar);
  int endPos = min(td->strCount, view->vScrollPos + ps.rcPaint.bottom / view->yChar + 1);

  for (int i = startPos; i < endPos; i++) {
    int x = -view->hScrollPos;
    int y = view->yChar * (i - view->vScrollPos);

    char* str = td->strings + td->strBegin[i];
    int charLen = td->strBegin[i + 1] - td->strBegin[i];

    int logicLen = charLen * view->xChar;
    if (view->lineBreak && logicLen > view->xClient) {
    }

    TextOutA(hdc, x, y, str, charLen);
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

  view->hScrollMax = max(0, view->dataPixelWidth - newWidth);
  view->hScrollPos = min(view->hScrollPos, view->hScrollMax);

  SetScrollRange(view->hwnd, SB_VERT, 0, view->vScrollMax, FALSE);
  SetScrollPos(view->hwnd, SB_VERT, view->vScrollPos, TRUE);

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
  if (!view) { return; }
  inc = clamp(inc, -view->vScrollPos, view->vScrollMax - view->vScrollPos);

  if (inc != 0) {
    view->vScrollPos += inc;
    ScrollWindow(view->hwnd, 0, -view->yChar * inc, NULL, NULL);
    SetScrollPos(view->hwnd, SB_VERT, view->vScrollPos, TRUE);
    UpdateWindow(view->hwnd);
  }
}

void scrollViewH(View* view, int inc) {
  if (!view) { return; }
  inc = clamp(inc, -view->hScrollPos, view->hScrollMax - view->hScrollPos);

  if (inc != 0) {
    view->hScrollPos += inc;
    ScrollWindow(view->hwnd, -inc, 0, NULL, NULL);
    SetScrollPos(view->hwnd, SB_HORZ, view->hScrollPos, TRUE);
  }
}

void freeView(View* view) {
  if (view) {
    DeleteObject(view->font);
  }
  free(view);
}