
#include <stdio.h>
#include <stdlib.h> /* malloc */
#include <math.h>

#include "endian.h"

#include "datalist.h"
#include "bandbuffer.h"
#include "band.h"
#include "rasterbuffer.h"

#define _WRITER_EXTERN
#include "writer.h"

#define SNAP(V,SD) (round((V)*(double)(SD))/(double)(SD))


void assignRasterOutputHeader(const RasterHeader *pheader, RasterOutputHeader *poutheader, int snapdenom, double expansion_rate) {
  poutheader->cols = pheader->pmaxx-pheader->pminx + 1;
  poutheader->rows = pheader->pmaxy-pheader->pminy + 1;
  poutheader->uminx = pheader->uminx;
  poutheader->uminy = pheader->uminy;
  poutheader->umaxx = pheader->umaxx;
  poutheader->umaxy = pheader->umaxy;
  /* snap */
  if( snapdenom != 0 ) {
    poutheader->uminx = SNAP(poutheader->uminx, snapdenom);
    poutheader->uminy = SNAP(poutheader->uminy, snapdenom);
    poutheader->umaxx = SNAP(poutheader->umaxx, snapdenom);
    poutheader->umaxy = SNAP(poutheader->umaxy, snapdenom);
  }
  poutheader->cellsize_x = (poutheader->umaxx-poutheader->uminx) / (double)poutheader->cols;
  poutheader->cellsize_y = (poutheader->umaxy-poutheader->uminy) / (double)poutheader->rows;
  if( snapdenom != 0 ) {
    poutheader->cellsize_x = SNAP(poutheader->cellsize_x, snapdenom);
    poutheader->cellsize_y = SNAP(poutheader->cellsize_y, snapdenom);
  }
  /* expansion */
  if( expansion_rate != 0 ) {
    poutheader->cellsize_x =  poutheader->cellsize_x * (1.0+expansion_rate);
    poutheader->cellsize_y =  poutheader->cellsize_y * (1.0+expansion_rate);
  }
  /* lets cellsize (unit/pixel) upsidedown */
  poutheader->cellsize_y = - poutheader->cellsize_y;
  /* */
  poutheader->umaxx = pheader->uminx + poutheader->cellsize_x * (double)poutheader->cols;
  poutheader->uminy = pheader->umaxy + poutheader->cellsize_y * (double)poutheader->rows;
}


size_t writeRasterBuffer_HEXWKB(RasterBuffer *rasterbuffer, FILE *fp, uint32_t srid, int snapdenom, double expansion_rate) {
  int bandnum;
  size_t r, c;
  PixelValueUnion value;
  RasterOutputHeader outputheader;
  size_t ret = 0;
  unsigned char endian_flag = 1;
  if(decideEndian() == BIG) {
    endian_flag = 0;
  }


  assignRasterOutputHeader(&(rasterbuffer->header), &outputheader, snapdenom, expansion_rate);
  ret += print_hexbin_byte(fp, endian_flag);
  ret += print_hexbin_uint16(fp, 0);
  ret += print_hexbin_uint16(fp, rasterbuffer->bandlist->length);
  ret += print_hexbin_double(fp, outputheader.cellsize_x);
  ret += print_hexbin_double(fp, outputheader.cellsize_y);
  ret += print_hexbin_double(fp, outputheader.uminx);
  ret += print_hexbin_double(fp, outputheader.umaxy);
  ret += print_hexbin_double(fp, 0);
  ret += print_hexbin_double(fp, 0);
  ret += print_hexbin_uint32(fp, srid);
  ret += print_hexbin_uint16(fp, outputheader.cols);
  ret += print_hexbin_uint16(fp, outputheader.rows);


  for( bandnum = 0; bandnum < rasterbuffer->bandlist->length; bandnum++ ) {
    int n;
    BandBuffer *bandbuffer;
    bandbuffer = RasterBuffer_GetBand(rasterbuffer, bandnum);
    ret += print_hexbin_byte(fp, 0x4a);
    ret += print_hexbin_pixelvalue(fp, bandbuffer->valuetype, bandbuffer->nodata);
    n = -(bandbuffer->pstartx + bandbuffer->pstarty * outputheader.cols);
    for( r = 0; r < outputheader.rows; r++ ) {
      for( c = 0; c < outputheader.cols; c++ ) {
        void *ptr;
        if( n >= 0 ) {
          ptr = DataList_Get(bandbuffer->datalist, n);
        }
        else {
          ptr = NULL;
        }
        if( ptr != NULL ) {
          ret += print_hexbin_value(fp, bandbuffer->valuetype, ptr);
        }
        else {
          ret += print_hexbin_pixelvalue(fp, bandbuffer->valuetype, bandbuffer->nodata);
        }
        n++;
      }
    }
  }
  return ret;
}


int writeRasterBuffer_Asc(RasterBuffer *rasterbuffer, FILE *fp, int bandnum, int snapdenom, double expansion_rate) {
  RasterOutputHeader outputheader;
  BandBuffer *bandbuffer;
  int n, c, r, nstart;

  bandbuffer = RasterBuffer_GetBand(rasterbuffer, bandnum);
  if( bandbuffer == NULL ) {
    return 0;
  }

  assignRasterOutputHeader(&(rasterbuffer->header), &outputheader, snapdenom, expansion_rate);

  fprintf(fp, "ncols %d\nnrows %d\nxllcorner %.14f\nyllcorner %.14f\ncellsize %.14f\n",
    outputheader.cols, outputheader.rows, outputheader.uminx, outputheader.uminy, outputheader.cellsize_x
  );
  if( bandbuffer->hasnodata ) {
    fprintf(fp, "NODATA_value ");
    print_pixelvalue(fp, bandbuffer->valuetype, bandbuffer->nodata);
    fputs("\n", fp);
  }
  /* data */
  n = -(bandbuffer->pstartx + bandbuffer->pstarty * outputheader.cols);
  for( r = 0; r < outputheader.rows; r++ ) {
    for( c = 0; c < outputheader.cols; c++ ) {
      void *ptr;
      if( c > 0 ) {
        fputs(" ", fp);
      }
      if( n >= 0 ) {
        ptr = DataList_Get(bandbuffer->datalist, n);
      }
      else {
        ptr = NULL;
      }
      if( ptr != NULL ) {
        print_value(fp, bandbuffer->valuetype, ptr);
      }
      else {
        if( bandbuffer->hasnodata ) {
          print_pixelvalue(fp, bandbuffer->valuetype, bandbuffer->nodata);
        }
        else {
          fputs("0", fp);
        }
      }
      n++;
    }
    fputs("\n", fp);
  }
  return outputheader.cols * outputheader.rows;
}

/*
// cc writer.c rasterbuffer.c bandbuffer.c band.c datalist.c datalistcell.c printerror.c -lm
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

  writeRasterBuffer_HEXWKB(rasterbuffer, stdout, 0);

  RasterBuffer_Free(rasterbuffer);
  return 0;
}
*/

