#ifndef _ENDIAN_HEAD_
#define _ENDIAN_HEAD_

#ifndef _ENDIAN_EXTERN
#define _ENDIAN_EXTERN extern
#endif

typedef enum {
  UNKNOWN = 0,
  BIG = 1,
  LITTLE = 2
} EndianEnum;

_ENDIAN_EXTERN EndianEnum decideEndian();

#endif /* endian.h */