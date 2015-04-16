#ifndef _BANDBUFFER_HEAD_
#define _BANDBUFFER_HEAD_

#include <stdint.h>

#ifndef _BANDBUFFER_EXTERN
#define _BANDBUFFER_EXTERN extern
#endif

#include "band.h"
#include "datalist.h"

typedef struct {
  /* Value type */
  BandValueType valuetype;
  /* Bytes for one value. */
  size_t valuebytes;
  /* Starting (col, row) position. */
  uint16_t pstartx;
  uint16_t pstarty;
  /* 0 if this band does not hvae nodata, otherwise this band has nodata. */
  int hasnodata;
  /* NoData value. */
  PixelValueUnion nodata;
  /* List of data */
  DataList *datalist;
} BandBuffer;

/**
 * Creates BandBuffer.
 * @param type Type of band element.
 * @return Pointer of bandbuffer. You must use BandBuffer_Free() if you want to expire this.
 */
_BANDBUFFER_EXTERN BandBuffer *BandBuffer_New(BandValueType type);

/**
 * Frees BandBuffer.
 * @param bandbuffer Band buffer.
 */
_BANDBUFFER_EXTERN void BandBuffer_Free(BandBuffer *bandbuffer);

#endif /* band.h */
