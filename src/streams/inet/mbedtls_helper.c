
#include "tkc/utils.h"

void mbedtls_awtk_debug(void* ctx, int level, const char* file, int line, const char* str) {
  const char *p, *basename;

  /* Extract basename from file */
  for (p = basename = file; *p != '\0'; p++)
    if (*p == '/' || *p == '\\') basename = p + 1;

  log_debug("%s:%04d: |%d| %s", basename, line, level, str);
}

