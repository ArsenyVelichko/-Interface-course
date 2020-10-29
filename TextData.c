#include "TextData.h"
#include <malloc.h>
#include <Windows.h>

#define READ_BUFFER_SIZE 1 << 16

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

static char* readTextFile(const char* fileName) {
  FILE* source;
  //source = fopen(fileName, "rt");
  //if (!source) { return NULL; }
  errno_t err = fopen_s(&source, fileName, "rt");
  if (err) { return NULL; }

  char buf[READ_BUFFER_SIZE];
  char* data = malloc(sizeof(char));
  if (!data) { return NULL; }
  size_t dataSize = 1;

  while (!feof(source)) {
    size_t nRead = fread(buf, sizeof(char), sizeof(buf), source);
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