#include <stdint.h>

#define _ENDIAN_EXTRERN
#include "endian.h"

static EndianEnum endian = UNKNOWN;

EndianEnum decideEndian() {
  uint16_t ui16;
  char *p;

  if( endian == BIG || endian == LITTLE ) {
    return endian;
  }
  // check
  ui16 = (uint16_t)0x0a0b0c0dU;
  p = (char *)&ui16;
  if( *p == 0x0d ) {
    endian = LITTLE;
  }
  else if( *p == 0x0a ) {
    endian = BIG;
  }
  return endian;
}

