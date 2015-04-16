#include <stdlib.h>
#include <stdint.h> // uint32_t

#include "printerror.h"
#include "datalist.h"
#include "band.h"



#define _BAHDBUFFER_EXTERN
#include "bandbuffer.h"

/**
 * Creates BandBuffer.
 * @param type Type of band element.
 * @return Pointer of bandbuffer. You must use BandBuffer_Free() if you want to expire this.
 */
BandBuffer *BandBuffer_New(BandValueType type) {
  BandBuffer *ret;
  ret = malloc(sizeof(BandBuffer));
  if( ret == NULL ) {
    print_perror("malloc");
    return NULL;
  }
  ret->valuetype = type;
  ret->valuebytes = BANDVALUETYPEBYTES[type];
  ret->hasnodata = 0;
  ret->datalist = DataList_New(1024, sizeof(float));
  if( ret->datalist == NULL ) {
    free(ret);
    return NULL;
  }
  return ret;
}

/**
 * Frees BandBuffer.
 * @param bandbuffer Band buffer.
 */
void BandBuffer_Free(BandBuffer *bandbuffer) {
  if( bandbuffer->datalist != NULL ) {
    DataList_Free(bandbuffer->datalist);
  }
  free(bandbuffer);
}

/*
// cc bandbuffer.c band.c datalist.c datalistcell.c printerror.c
#include <stdio.h>

int main(int ac, char *av[]) {
  BandBuffer *bb;
  int c, r, nstart;
  int pwidth, pheight;

  bb = BandBuffer_New(BVT_32BF);
  // sets nodata value
  bb->nodata.v32bf = (float)-9999.0;
  bb->hasnodata = 1; // do not forget !
  bb->pstartx = 1;
  bb->pstarty = 1;
  pwidth = 4;
  pheight = 3;
  nstart = bb->pstartx + pwidth * bb->pstarty;
  for( r = 0; r < 3; r++ ) {
    for( c = 0; c < 4; c++ ) {
      int n = c + r * pwidth;
      if( n >= nstart ) {
        float v;
        v = r * 10 + c;
        DataList_Push(bb->datalist, &v, 1);
        printf("Push %d %d -> %d %f\n",c, r, n, v);
      }
    }
  }

  for( r = 0; r < 3; r++ ) {
    for( c = 0; c < 4; c++ ) {
      int n = c + r * pwidth;
      if( n >= nstart ) {
        float *pv;
        pv = DataList_Get(bb->datalist, n - nstart);
        if( pv != NULL ) {
          printf("%d %d -> %d %f\n",c, r, n, *pv);
        }
        else {
          printf("%d %d -> %d NULL\n",c, r, n);
        }
      }
    }
  }
  BandBuffer_Free(bb);
  return 0;
}
*/

