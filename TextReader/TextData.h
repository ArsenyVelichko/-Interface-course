#pragma once

//Structure that stores text and some of its metrics
typedef struct {
  int strCount;
  int maxLen;
  char* strings;
  int* strBegin;
} TextData;

//Constructor of text data
TextData* createTextData(const char* fileName);
//Destructor of text data
void freeTextData(TextData* textData);