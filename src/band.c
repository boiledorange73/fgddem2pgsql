#include <stdio.h>

#define _BAND_EXTERN
#include "band.h"

/**
* Prints value text.
* @param fp File pointer.
* @param valuetype Type of variable pointed by ptr.
* @param ptr Variable pointer.
* @return Count of printed value. Usually, returns 1.
*/
size_t print_value(FILE *fp, BandValueType valuetype, void *ptr) {
  switch( valuetype ) {
  case BVT_8BSI:
    fprintf(fp, "%d", *((char *)ptr));
    return 1;
  case BVT_8BUI:
    fprintf(fp, "%d", *((unsigned char *)ptr));
    return 1;
  case BVT_16BSI:
    fprintf(fp, "%d", *((int16_t *)ptr));
    return 1;
  case BVT_16BUI:
    fprintf(fp, "%d", *((uint16_t *)ptr));
    return 1;
  case BVT_32BSI:
    fprintf(fp, "%d", *((int32_t *)ptr));
    return 1;
  case BVT_32BUI:
    fprintf(fp, "%d", *((uint32_t *)ptr));
    return 1;
  case BVT_32BF:
    fprintf(fp, "%f", *((float *)ptr));
    return 1;
  case BVT_64BF:
    fprintf(fp, "%f", *((double *)ptr));
    return 1;
  default:
    return 0;
  }
}

/**
* Prints pixel value text.
* @param fp File pointer.
* @param valuetype Member of union.
* @param value Value union.
* @return Count of printed value. Usually, returns 1.
*/
size_t print_pixelvalue(FILE *fp, BandValueType valuetype, PixelValueUnion value) {
  switch( valuetype ) {
  case BVT_8BSI:
    fprintf(fp, "%d", value.v8bsi);
    return 1;
  case BVT_8BUI:
    fprintf(fp, "%u", value.v8bui);
    return 1;
  case BVT_16BSI:
    fprintf(fp, "%d", value.v16bsi);
    return 1;
  case BVT_16BUI:
    fprintf(fp, "%u", value.v16bui);
    return 1;
  case BVT_32BSI:
    fprintf(fp, "%d", value.v32bsi);
    return 1;
  case BVT_32BUI:
    fprintf(fp, "%u", value.v32bui);
    return 1;
  case BVT_32BF:
    fprintf(fp, "%f", value.v32bf);
    return 1;
  case BVT_64BF:
    fprintf(fp, "%f", value.v64bf);
    return 1;
  default:
    return 0;
  }
}

/* decimal to hexadecimal table. */
static char hexchars[] = "0123456789ABCDEF";

/**
 * Prints hexbin text from the array.
 * @param fp File pointer.
 * @param ptr Pointer pointing the head of array.
 * @param bytes Bytes of writing.
 */
size_t print_hexbin(FILE *fp, void *ptr, size_t bytes) {
  char *p;
  size_t n;

  p = (char *)ptr;

  for( n = 0; n < bytes; n++ ) {
    fputc(hexchars[(*p&0xf0)>>4], fp);
    fputc(hexchars[(*p&0x0f)], fp);
    p++;
  }
  return bytes;
}

/**
 * Prints hexbin text from one value.
 * @param fp File pointer.
 * @param valuetype Type of value.
 * @param ptr Points the value.
 */
size_t print_hexbin_value(FILE *fp, BandValueType valuetype, void *ptr) {
  size_t bytes, n;
  switch( valuetype ) {
  case BVT_8BSI:
  case BVT_8BUI:
    return print_hexbin(fp, ptr, 1);
  case BVT_16BSI:
  case BVT_16BUI:
    return print_hexbin(fp, ptr, 2);
  case BVT_32BSI:
  case BVT_32BUI:
  case BVT_32BF:
    return print_hexbin(fp, ptr, 4);
  case BVT_64BF:
    return print_hexbin(fp, ptr, 8);
  default:
    return 0;
  }
}

/**
 * Prints hexbin text from one value union.
 * @param fp File pointer.
 * @param valuetype Type of value.
 * @param value Value union.
 */
size_t print_hexbin_pixelvalue(FILE *fp, BandValueType valuetype, PixelValueUnion value) {
  size_t bytes, n;
  switch( valuetype ) {
  case BVT_8BSI:
    return print_hexbin(fp, &(value.v8bsi), 1);
  case BVT_8BUI:
    return print_hexbin(fp, &(value.v8bui), 1);
  case BVT_16BSI:
    return print_hexbin(fp, &(value.v16bsi), 2);
  case BVT_16BUI:
    return print_hexbin(fp, &(value.v16bui), 2);
  case BVT_32BSI:
    return print_hexbin(fp, &(value.v32bsi), 4);
  case BVT_32BUI:
    return print_hexbin(fp, &(value.v32bui), 4);
  case BVT_32BF:
    return print_hexbin(fp, &(value.v32bf), 4);
  case BVT_64BF:
    return print_hexbin(fp, &(value.v64bf), 8);
  default:
    return 0;
  }
}

/**
 * Prints hexbin text from one byte value.
 * @param fp File pointer.
 * @param value Value.
 */
size_t print_hexbin_byte(FILE *fp, unsigned char value) {
  return print_hexbin(fp, &value, 1);
}

/**
 * Prints hexbin text from one uint16_t value.
 * @param fp File pointer.
 * @param value Value.
 */
size_t print_hexbin_uint16(FILE *fp, uint16_t value) {
  return print_hexbin(fp, &value, 2);
}

/**
 * Prints hexbin text from one uint32_t value.
 * @param fp File pointer.
 * @param value Value.
 */
size_t print_hexbin_uint32(FILE *fp, uint32_t value) {
  return print_hexbin(fp, &value, 4);
}

/**
 * Prints hexbin text from one float value.
 * @param fp File pointer.
 * @param value Value.
 */
size_t print_hexbin_float(FILE *fp, float value) {
  return print_hexbin(fp, &value, 4);
}

/**
 * Prints hexbin text from one double value.
 * @param fp File pointer.
 * @param value Value.
 */
size_t print_hexbin_double(FILE *fp, double value) {
  return print_hexbin(fp, &value, 8);
}

