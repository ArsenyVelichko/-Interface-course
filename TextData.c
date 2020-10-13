#include "TextData.h"
#include <malloc.h>
#include <Windows.h>

static int* splitToStrings(const char* buf, int* strCount) {
  int* strBegin = malloc(2 * sizeof(int));
  if (!strBegin) { return NULL; }
  strBegin[0] = 0;
  *strCount = 1;
  
  int i;
  for (i = 0; buf[i]; i++) {
    if (buf[i] == '\n') {
      int* tmp = realloc(strBegin, (*strCount + 2) * sizeof(int));

      if (tmp) {
        strBegin = tmp;
        strBegin[*strCount] = i;
        (*strCount)++;

      } else {
        free(strBegin);
        return NULL;
      }
    }
  }
  strBegin[*strCount] = i;
  return strBegin;
}

static int maxLen(int* strBegin, int size) {
  int maxLen = 0;
  for (int i = 0; i < size; i++) {
    maxLen = max(strBegin[i + 1] - strBegin[i], maxLen);
  }
  return maxLen;
}

TextData* createTextData(const char* fileName) {
  FILE* source;
  //source = fopen(fileName, "rb");
  //if (!source) { return NULL; }
  errno_t err = fopen_s(&source, fileName, "rb");
  if (err) { return NULL; }

  fseek(source, 0, SEEK_END);
  int len = ftell(source);
  fseek(source, 0, SEEK_SET);

  TextData* textData = malloc(sizeof(TextData));

  if (textData) {
    textData->strings = malloc(len * sizeof(char));
    if (textData->strings) {
      fread(textData->strings, sizeof(char), len, source);
      textData->strings[len - 1] = '\0';
      textData->strBegin = splitToStrings(textData->strings, &textData->strCount);
    }

    if (!textData->strings || !textData->strBegin) {
      free(textData->strings);
      free(textData);
      textData = NULL;
    } else {
      textData->maxLen = maxLen(textData->strBegin, textData->strCount);
    }
  }
  fclose(source);
  return textData;
}

void freeTextData(TextData* textData) {
  if (textData) {
    free(textData->strBegin);
    free(textData->strings);
  }
  free(textData);
}