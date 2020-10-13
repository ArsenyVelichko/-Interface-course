#pragma once

#include <stdio.h>

typedef struct {
  int strCount;
  int maxLen;
  char* strings;
  int* strBegin;
} TextData;

TextData* createTextData(const char* fileName);
void freeTextData(TextData* textData);