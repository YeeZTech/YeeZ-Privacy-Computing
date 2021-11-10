#include "stbox/stx_common.h"
#include "stbox_t.h"
#include <cstdarg>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

uint64_t stbox_common_version() { return 1; }
namespace stbox {
int printf(const char *fmt, ...) {
  char buf[BUFSIZ] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  ocall_print_string(buf);
  return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

// TODO bugs to fix
// int printf(const char *fmt, ...) {
//// get buffer size
// va_list ap;
// va_start(ap, fmt);
// uint32_t len = snprintf(NULL, 0, fmt, ap);
// va_end(ap);

//// allocate and outprint buffer
// char *buf = (char *)malloc(len + 1);
// memset(buf, 0, len + 1);
// va_start(ap, fmt);
// len = vsnprintf(buf, len + 1, fmt, ap);
// va_end(ap);
// ocall_print_string(buf);
// free(buf);
// return len;
//}

int sprintf(char *buf, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

} // namespace stbox
