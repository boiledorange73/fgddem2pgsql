#include <stdio.h>

#define _PRINTERROR_EXTERN
#include "printerror.h"

void print_perror(const char *msg) {
  perror(msg);
}

void print_error(const char *msg) {
  fputs(msg, stderr);
}

void print_errorln(const char *msg) {
  fputs(msg, stderr);
  fputs("\n", stderr);
}

