#pragma once
#include "TextData.h"
#include <Windows.h>

typedef struct {
  HWND hwnd;
  HFONT font;
  TextData* textData;
  int xChar, yChar;
  int xClient, yClient;
  int vScrollPos, vScrollMax;
  int hScrollPos, hScrollMax;
  int dataPixelWidth;
  BOOL lineBreak;
} View;

View* createView(TextData* textData, BOOL lineBreak, HWND hwnd);
void freeView(View* view);

void drawView(View* view);
void resizeView(View* view, int newWidth, int newHeight);

void scrollViewV(View* view, int inc);
void scrollViewH(View* view, int inc);