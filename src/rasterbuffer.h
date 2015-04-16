#ifndef _RASTERBUFFER_HEAD_
#define _RASTERBUFFER_HEAD_

#include "datalist.h"
#include "rasterheader.h"
#include "bandbuffer.h"

#ifndef _RASTERBUFFER_EXTERN
#define _RASTERBUFFER_EXTERN extern
#endif

typedef struct {
  RasterHeader header;
  DataList* bandlist;
} RasterBuffer;

_RASTERBUFFER_EXTERN RasterBuffer *RasterBuffer_New();
_RASTERBUFFER_EXTERN void RasterBuffer_Free(RasterBuffer *rasterbuffer);
_RASTERBUFFER_EXTERN BandBuffer *RasterBuffer_NewBandBuffer(RasterBuffer *rasterbuffer, BandValueType type);
_RASTERBUFFER_EXTERN BandBuffer *RasterBuffer_GetBand(RasterBuffer *rasterbuffer, int index);

#endif /* rasterbuffer.h */
