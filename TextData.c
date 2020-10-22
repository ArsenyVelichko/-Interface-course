#include "TextData.h"
#include <malloc.h>
#include <Windows.h>

#define READ_BUFFER_SIZE 1 << 16

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
        strBegin[*strCount] = i + 1;
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
    maxLen = max(strBegin[i + 1] - strBegin[i] - 1, maxLen);
  }
  return maxLen;
}

static char* readTextFile(const char* fileName) {
  FILE* source;
  //source = fopen(fileName, "rt");
  //if (!source) { return NULL; }
  errno_t err = fopen_s(&source, fileName, "rt");
  if (err) { return NULL; }

  char buf[READ_BUFFER_SIZE];
  char* data = malloc(sizeof(char));
  if (!data) { return NULL; }
  int dataSize = 1;

  while (!feof(source)) {
    int nRead = fread(buf, sizeof(char), sizeof(buf), source);
    char* tmp = realloc(data, (dataSize + nRead) * sizeof(char));

    if (tmp) {
      data = tmp;
      memcpy(data + dataSize - 1, buf, nRead * sizeof(char));
      dataSize += nRead;

    } else {
      free(data);
      return NULL;
    }
  }
  data[dataSize - 1] = '\0';
  fclose(source);
  return data;
}

TextData* createTextData(const char* fileName) {
  TextData* textData = malloc(sizeof(TextData));
  if (textData) {
    textData->strings = readTextFile(fileName);

    if (textData->strings) {
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
  return textData;
}

void freeTextData(TextData* textData) {
  if (textData) {
    free(textData->strBegin);
    free(textData->strings);
  }
  free(textData);
}