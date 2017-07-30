/**
pfodUtils for Arduino

(c)2013 Forward Computing and Control Pty. Ltd.
www.forward.com.au
This code may be freely used for both private and commercial use.
Provide this copyright is maintained.
*/
#include "Arduino.h"
#include "pfodParserUtils.h"

// will always put '\0\ at dest[maxLen]
// return the number of char copied excluding the terminating null
size_t strncpy_safe(char* dest, const char* src, size_t maxLen) {
  size_t rtn = 0;
  if (src == NULL) {
    dest[0] = '\0';
  } else {
    strncpy(dest, src, maxLen);
    rtn = strlen(src);
    if ( rtn > maxLen) {
      rtn = maxLen;
    }
  }
  dest[maxLen] = '\0';
  return rtn;
}


// maxLen includes null at end
void getProgStr(const __FlashStringHelper *ifsh, char*str,
                int maxLen) {
  const char *p = (const char *)ifsh;
  int i = 0;
  while (i < maxLen - 1) {
    unsigned char c = pgm_read_byte(p++);
    str[i++] = c;
    if (c == 0) {
      break;
    }
  }
  str[i] = '\0'; // terminate
}


