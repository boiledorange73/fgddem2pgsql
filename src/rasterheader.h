#ifndef _RASTERHEADER_HEAD_
#define _RASTERHEADER_HEAD_

#include <stdint.h> // uint32_t


#ifndef _RASTERHEADER_EXTERN
#define _RASTERHEADER_EXTERN extern
#endif

typedef struct {
  double uminx;
  double uminy;
  double umaxx;
  double umaxy;
  uint16_t pminx;
  uint16_t pminy;
  uint16_t pmaxx;
  uint16_t pmaxy;
} RasterHeader;

/*
#define RasterHeader_CalcNumCols(rh) ((rh).pmaxx-(rh).pminx+1)
#define RasterHeader_CalcNumRows(rh) ((rh).pmaxy-(rh).pminy+1)
#define RasterHeader_CalcUWidth(rh) ((rh).umaxx-(rh).uminx)
#define RasterHeader_CalcUHeight(rh) ((rh).umaxy-(rh).uminy)
#define RasterHeader_CalcCellSizeX(rh) (RasterHeader_CalcUWidth(rh)/(double)RasterHeader_CalcNumCols(rh))
#define RasterHeader_CalcCellSizeY(rh) (RasterHeader_CalcUHeight(rh)/(double)RasterHeader_CalcNumRows(rh))
*/

#endif /* rasterheader.h */
