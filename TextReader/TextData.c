#include "TextData.h"
#include <malloc.h>
#include <Windows.h>

typedef char* (*FuncPointer) (const char* fileName);

static void splitToStrings(TextData* td) {
  td->strBegin = malloc(2 * sizeof(int));
  if (!td->strBegin) { return; }
  td->strBegin[0] = 0;
  td->strCount = 1;
  td->maxLen = 0;

  int i;
  for (i = 0; td->strings[i]; i++) {
    if (td->strings[i] == '\n') {
      int* tmp = realloc(td->strBegin, (td->strCount + 2) * sizeof(int));

      if (tmp) {
        td->strBegin = tmp;
        td->strBegin[td->strCount] = i + 1;
        td->maxLen = max(i - td->strBegin[td->strCount - 1], td->maxLen);
        td->strCount++;

      } else {
        free(td->strBegin);
        td->strBegin = NULL;
        return;
      }
    }
  }
  td->strBegin[td->strCount] = i;
  td->maxLen = max(i - td->strBegin[td->strCount - 1], td->maxLen);
}

TextData* createTextData(const char* fileName) {
  HINSTANCE hDll = LoadLibraryA("FileReading");
  if (!hDll) { return NULL; }
  FuncPointer readFile = (FuncPointer)GetProcAddress(hDll, "readFile");
  if (!readFile) { return NULL; }

  TextData* textData = malloc(sizeof(TextData));
  if (textData) {
    textData->strings = readFile(fileName);

    if (textData->strings) {
      splitToStrings(textData);
    }

    if (!textData->strings || !textData->strBegin) {
      free(textData->strings);
      free(textData);
      textData = NULL;
    }
  }
  return textData;
}

void freeTextData(TextData* textData) {
  if (textData) {
    free(textData->strBegin);
    free(textData->strings);
  }
  free(textData);
}
