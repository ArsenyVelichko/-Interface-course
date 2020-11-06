#pragma once
#include "TextData.h"
#include <Windows.h>

//Structure that stores all necessary parameters for data visualization
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

//Constructor of view
View* createView(TextData* textData, HWND hwnd);
//Destructor of view
void freeView(View* view);

//Redraws view in invalid area
void drawView(View* view);
//Modifies views parameters to a new size
BOOL resizeView(View* view, int newWidth, int newHeight);

//Functions for scrolling views
void scrollViewV(View* view, int inc);
void scrollViewH(View* view, int inc);

//Shows view in the appropriate mode(without or with wrapping)
void showView(View* view, BOOL wrapFlag);