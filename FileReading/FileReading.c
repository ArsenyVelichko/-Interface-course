#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#define DllExport  __declspec(dllexport)
#define READ_BUFFER_SIZE 1 << 8

DllExport char* readFile(const char* fileName) {
  FILE* source;
  //source = fopen(fileName, "rt");
  //if (!source) { return NULL; }
  errno_t err = fopen_s(&source, fileName, "r");
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