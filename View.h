#pragma once
#include "TextData.h"
#include <Windows.h>

typedef struct {
  HWND hwnd;
  TextData* textData;
  int* lineBegin;
  int xChar, yChar;
  int xClient, yClient;
  int vScrollPos, hScrollPos;
  int vScrollMax, hScrollMax;
  BOOL symbolWrap;
} View;

View* createView(TextData* textData, HWND hwnd);
void freeView(View* view);

void drawView(View* view);
BOOL resizeView(View* view, int newWidth, int newHeight);

void scrollViewV(View* view, int inc);
void scrollViewH(View* view, int inc);

void showView(View* view, BOOL wrapFlag);