#pragma once
#include "TextData.h"
#include <Windows.h>

typedef struct {
  HWND hwnd;
  TextData* textData;
  int xChar, yChar;
  int xClient, yClient;
  int vScrollPos, vScrollMax;
  int hScrollPos, hScrollMax;
} View;

View* createView(TextData* textData, HWND hwnd);
void drawView(View* view);
void resizeView(View* view, int newWidth, int newHeight);
void scrollViewV(View* view, int inc);
void scrollViewH(View* view, int inc);