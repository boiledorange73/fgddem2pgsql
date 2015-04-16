#ifndef _WRITER_HEAD_
#define _WRITER_HEAD_

#include <stdio.h>
#include <stdint.h> // uint32_t

#include "rasterbuffer.h"

#ifndef _WRITER_EXTERN
#define _WRITER_EXTERN extern
#endif

typedef struct {
  double uminx;
  double uminy;
  double umaxx;
  double umaxy;
  double cellsize_x;
  double cellsize_y;
  uint16_t cols;
  uint16_t rows;
} RasterOutputHeader;

_WRITER_EXTERN int writeRasterBuffer_Asc(RasterBuffer *rasterbuffer, FILE *fp, int bandnum, int snapdenom, double expansion_rate);
_WRITER_EXTERN size_t writeRasterBuffer_HEXWKB(RasterBuffer *rasterbuffer, FILE *fp, uint32_t srid, int snapdenom, double expansion_rate);
_WRITER_EXTERN void assignRasterOutputHeader(const RasterHeader *pheader, RasterOutputHeader *poutheader, int snapdenom, double expansion_rate);

#endif /* rasterbuffer.h */
