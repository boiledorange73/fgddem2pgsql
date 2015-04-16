
#include <stdlib.h> // malloc
#include <math.h>

#include "printerror.h"
#include "datalist.h"
#include "bandbuffer.h"

#define _RASTERBUFFER_EXTERN
#include "rasterbuffer.h"

RasterBuffer *RasterBuffer_New() {
  RasterBuffer *rasterbuffer;
  rasterbuffer = malloc(sizeof(RasterBuffer));
  if( rasterbuffer == NULL ) {
    print_perror("malloc");
    return NULL;
  }
  rasterbuffer->bandlist = DataList_New(sizeof(BandBuffer *), 16);
  if( rasterbuffer->bandlist == NULL ) {
    free(rasterbuffer);
    return NULL;
  }
  return rasterbuffer;
}

void RasterBuffer_Free(RasterBuffer *rasterbuffer) {
  if( rasterbuffer->bandlist != NULL ) {
    int n;
    BandBuffer **pbandbuffer;
    for( n = 0; n < rasterbuffer->bandlist->length; n++ ) {
      pbandbuffer = DataList_Get(rasterbuffer->bandlist, n);
      if( pbandbuffer != NULL && *pbandbuffer != NULL ) {
        BandBuffer_Free(*pbandbuffer);
      }
    }
    DataList_Free(rasterbuffer->bandlist);
  }
  free(rasterbuffer);
}

BandBuffer *RasterBuffer_NewBandBuffer(RasterBuffer *rasterbuffer, BandValueType type) {
  BandBuffer *bandbuffer;
  bandbuffer = BandBuffer_New(type);

  if( bandbuffer == NULL ) {
    return NULL;
  }
  DataList_Push(rasterbuffer->bandlist, &bandbuffer, 1);
  return bandbuffer;
}

BandBuffer *RasterBuffer_GetBand(RasterBuffer *rasterbuffer, int index) {
  BandBuffer **pbandbuffer;
  if( rasterbuffer->bandlist == NULL ) {
    return NULL;
  }
  pbandbuffer = DataList_Get(rasterbuffer->bandlist, index);
  if( pbandbuffer == NULL ) {
    return NULL;
  }
  return *pbandbuffer;
}

/*
// cc writer.c rasterbuffer.c bandbuffer.c band.c datalist.c datalistcell.c printerror.c -lm
#include "writer.h"
int main(int ac, char *av[]) {
  RasterBuffer *rasterbuffer;
  BandBuffer *bandbuffer;
  int c, r, n;
  float value;
  rasterbuffer = RasterBuffer_New();
  rasterbuffer->header.uminx = 135.0;
  rasterbuffer->header.umaxx = 135.1;
  rasterbuffer->header.uminy = 35.0;
  rasterbuffer->header.umaxy = 35.1;
  rasterbuffer->header.pminx = 0;
  rasterbuffer->header.pminy = 0;
  rasterbuffer->header.pmaxx = 8;
  rasterbuffer->header.pmaxy = 8;
  bandbuffer = RasterBuffer_NewBandBuffer(rasterbuffer, BVT_32BF);
  bandbuffer->nodata.v32bf = (float)-9999.0;
  bandbuffer->hasnodata = 1;
  bandbuffer->pstartx = 1;
  bandbuffer->pstarty = 2;

  n = 0;
  for( r = 0; r < 6; r++ ) {
    for( c = 0; c < 9; c++ ) {
      value = r * 10 + c;
      DataList_Push(bandbuffer->datalist, &value, 1);
    }
  }

  writeRasterBuffer_Asc(rasterbuffer, stdout, 0, 0);

  RasterBuffer_Free(rasterbuffer);
  return 0;
}
*/

