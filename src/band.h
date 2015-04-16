#ifndef _BAND_HEAD_
#define _BAND_HEAD_

#include <stdio.h>
#include <stdint.h>

#ifndef _BAND_EXTERN
#define _BAND_EXTERN extern
_BAND_EXTERN int BANDVALUETYPEBYTES[];
_BAND_EXTERN char *BANDVALUETYPE_PRINTF[];
#else
_BAND_EXTERN int BANDVALUETYPEBYTES[] = {
  0, 0, 0, 1, 1, 2, 2, 4, 4, -1, 4, 8
};
_BAND_EXTERN char *BANDVALUETYPE_PRINTF[] = {
  NULL, NULL, NULL, "%d", "%u", "%d", "%u", "%d", "%u", NULL, "%.14f", "%.14f"
};
#endif

/**
 * Band Value Type
 */
typedef enum {
  BVT_1BB = 0,
  BVT_2BUI = 1,
  BVT_4BUI = 2,
  BVT_8BSI = 3,
  BVT_8BUI = 4,
  BVT_16BSI = 5,
  BVT_16BUI = 6,
  BVT_32BSI = 7,
  BVT_32BUI = 8,
  BVT_32BF = 10,
  BVT_64BF = 11
} BandValueType;

/**
 * Union of band value.
 */
typedef union {
  /* 8bit char */
  char v8bsi;
  /* 8bit unsigned char */
  unsigned char v8bui;
  /* 16bit integer */
  int16_t v16bsi;
  /* 16bit unsigned integer */
  uint16_t v16bui;
  /* 32bit integer */
  int32_t v32bsi;
  /* 32bit unsigned integer */
  uint32_t v32bui;
  /* 32bit floating point (float) */
  float v32bf;
  /* 64bit floating point (double) */
  double v64bf;
} PixelValueUnion;

/**
* Prints value text.
* @param fp File pointer.
* @param valuetype Type of variable pointed by ptr.
* @param ptr Variable pointer.
* @return Count of printed value. Usually, returns 1.
*/
_BAND_EXTERN size_t print_value(FILE *fp, BandValueType valuetype, void *ptr);

/**
* Prints pixel value text.
* @param fp File pointer.
* @param valuetype Member of union.
* @param value Value union.
* @return Count of printed value. Usually, returns 1.
*/
_BAND_EXTERN size_t print_pixelvalue(FILE *fp, BandValueType valuetype, PixelValueUnion value);

/**
* Prints hexbin text from the array.
* @param fp File pointer.
* @param ptr Pointer pointing the head of array.
* @param bytes Bytes of writing.
*/
_BAND_EXTERN size_t print_hexbin(FILE *fp, void *ptr, size_t bytes);

/**
* Prints hexbin text from one value.
* @param fp File pointer.
* @param valuetype Type of value.
* @param ptr Points the value.
*/
_BAND_EXTERN size_t print_hexbin_value(FILE *fp, BandValueType valuetype, void *ptr);

/**
* Prints hexbin text from one value union.
* @param fp File pointer.
* @param valuetype Type of value.
* @param value Value union.
*/
_BAND_EXTERN size_t print_hexbin_pixelvalue(FILE *fp, BandValueType valuetype, PixelValueUnion value);

/**
* Prints hexbin text from one byte value.
* @param fp File pointer.
* @param value Value.
*/
_BAND_EXTERN size_t print_hexbin_byte(FILE *fp, unsigned char value);

/**
* Prints hexbin text from one uint16_t value.
* @param fp File pointer.
* @param value Value.
*/
_BAND_EXTERN size_t print_hexbin_uint16(FILE *fp, uint16_t value);

/**
* Prints hexbin text from one uint32_t value.
* @param fp File pointer.
* @param value Value.
*/
_BAND_EXTERN size_t print_hexbin_uint32(FILE *fp, uint32_t value);

/**
* Prints hexbin text from one float value.
* @param fp File pointer.
* @param value Value.
*/
_BAND_EXTERN size_t print_hexbin_float(FILE *fp, float value);

/**
* Prints hexbin text from one double value.
* @param fp File pointer.
* @param value Value.
*/
_BAND_EXTERN size_t print_hexbin_double(FILE *fp, double value);

#endif /* band.h */
