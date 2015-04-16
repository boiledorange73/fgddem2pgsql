#ifndef _PRINTERROR_HEAD_
#define _PRINTERROR_HEAD_

#ifndef _PRINTERROR_EXTERN
#define _PRINTERROR_EXTERN extern
#endif

_PRINTERROR_EXTERN void print_perror(const char *msg);
_PRINTERROR_EXTERN void print_error(const char *msg);
_PRINTERROR_EXTERN void print_errorln(const char *msg);

#endif /* printerror.h */
