#include <stdio.h>
#include <string.h>
#include <ctype.h> // isspace
#include <expat.h>

#include "printerror.h"
#include "datalist.h"
#include "rasterbuffer.h"

#define _FGDDEMREADER_EXTERN
#include "fgddemreader.h"

#define WKSTR_SIZE (1024)
static unsigned char wkstr[WKSTR_SIZE+1];

#define ERR_NOERROR (0x0000)
#define ERR_PARSERERROR (0x0001)
#define ERR_TOO_LONG_TUPPLE (0x0002)
#define ERR_INVALID_TUPPLE_VALUE (0x0003)
#define ERR_INVALID_LOW_VALUE (0x0004)
#define ERR_INVALID_HIGH_VALUE (0x0005)
#define ERR_INVALID_LOWERCORNER_VALUE (0x0006)
#define ERR_INVALID_UPPERCORNER_VALUE (0x0007)
#define ERR_INVALID_STARTPOINT_VALUE (0x0008)

/* Internal Buffers */
static RasterBuffer *rasterbuffer;
static DataList *stringbuffer;
static BandBuffer *bandbuffer;
static volatile int errorcode;
static volatile int tupple_status;

/*
 * (INTERNAL) Case insentive comparision between 2 strings.
 * @param s1 String #1
 * @param s2 String #2
 * @return -1 if s1 < s2, 1 if s1 > s2, 0 is s1 = s2
 */
static int string_caseinsentive_compare(const char *s1, const char *s2) {
	while (1) {
		int ch1, ch2;
		ch1 = *(s1++);
		if (ch1 >= 'a' && ch1 <= 'z') {
			ch1 = ch1 + ('A' - 'a');
		}
		ch2 = *(s2++);
		if (ch2 >= 'a' && ch2 <= 'z') {
			ch1 = ch1 + ('A' - 'a');
		}
		if (ch1 < ch2) {
			return -1;
		}
		else if (ch1 > ch2) {
			return 1;
		}
		else if (ch1 == '\0') {
			return 0;
		}
	}
}

/**
 * (INTERNAL)  Prints the error message along with errorcode (internal variable).
 */
static void print_fgddem_error() {
  switch( errorcode ) {
  case ERR_NOERROR:
    print_error("No Error.");
    break;
  case ERR_PARSERERROR:
    print_error("Parser Error.");
    break;
  case ERR_TOO_LONG_TUPPLE:
    print_error("Too long tuppleList value.");
    break;
  case ERR_INVALID_TUPPLE_VALUE:
    print_error("Invalid tuppleList value.");
    break;
  case ERR_INVALID_LOW_VALUE:
    print_error("Invalid low value.");
    break;
  case ERR_INVALID_HIGH_VALUE:
    print_error("Invalid high value.");
    break;
  case ERR_INVALID_LOWERCORNER_VALUE:
    print_error("Invalid lowerCorner value.");
    break;
  case ERR_INVALID_UPPERCORNER_VALUE:
    print_error("Invalid upperCorner value.");
    break;
  case ERR_INVALID_STARTPOINT_VALUE:
    print_error("Invalid startPoint value.");
    break;
  default:
    print_error("Unknown error.");
    break;
  }
}

/**
 * (INTERNAL) Gets a couple of double values from string buffer.
 * @param stringbuffer String buffer.
 * @param p1 Pointer of double variable to be set.
 * @param p2 Pointer of double variable to be set.
 * @return Count of variable which are really set by someone.
 */
static int extractDoubleCouple(DataList *stringbuffer, double *p1, double *p2) {
  int ret;
  ret = DataList_CopyTo(stringbuffer, 0, wkstr, WKSTR_SIZE);
  if( ret <= 0 ) {
    // error
    return 0;
  }
  wkstr[ret] = '\0';
  ret = sscanf((char *)wkstr, "%lf%lf", p1, p2);
  return ret;
}

/**
 * (INTERNAL) Gets a couple of uint16_t (16 bits unsigned integer) values from string buffer.
 * @param stringbuffer String buffer.
 * @param p1 Pointer of uint16_t variable to be set.
 * @param p2 Pointer of uint16_t variable to be set.
 * @return Count of variable which are really set by someone.
 */
static int extractUint16Couple(DataList *stringbuffer, uint16_t* p1, uint16_t *p2) {
  int ret;
  ret = DataList_CopyTo(stringbuffer, 0, wkstr, WKSTR_SIZE);
  if( ret <= 0 ) {
    // error
    return 0;
  }
  wkstr[ret] = '\0';
  ret = sscanf((char *)wkstr, "%hu%hu", p1, p2);
  return ret;
}

/**
 * (INTERNAL) Reads tupples (dem value) as possible.
 * @param fin True if final process.
 * @return Count of tupples read successfully.
 */
static int process_tupple(int fin) {
  static const unsigned char ch_ret = '\n';
  int ret = 0;
  int ix;

  if( fin ) {
    DataList_Push(stringbuffer, (void *)&ch_ret, 1);
  }
  while( (ix = DataList_IndexOf(stringbuffer, (void *)&ch_ret)) >= 0 ) {
    if( ix == 0 ) {
      DataList_Shift(stringbuffer, NULL, 1);
    }
    else if( ix >= WKSTR_SIZE ) {
      errorcode = ERR_TOO_LONG_TUPPLE;
      return -1;
    }
    else {
      int len;
      int st = 0;
      unsigned char *p, *t, *p1;
      len = DataList_Shift(stringbuffer, wkstr, ix);
      wkstr[len] = '\0';
      for( p = wkstr, t = wkstr + ix; *p != ',' && p < t; p++ ) {
        // DOES NOTHING
      }
      p++; // skips ','
      if( p < t ) {
        float v;
        if( sscanf((char *)p, "%f", &v) != 1 ) {
          errorcode = ERR_INVALID_TUPPLE_VALUE;
          return -1;
        }
        DataList_Push(bandbuffer->datalist, &v, 1);
        ret++;
      }
    }
  }
  return ret;
}

/**
 * Called by the parser when the parser gets the entity.
 * @param userData Not used.
 * @param s Entity string.
 * @param len Length of entity.
 */
static void XMLCALL on_character(void *userData, const XML_Char *s, int len) {
  if( errorcode != ERR_NOERROR ) {
    return;
  }

  DataList_Push(stringbuffer, (void *)s, len * sizeof(XML_Char));
  if( tupple_status > 0 ) {
    process_tupple(0);
  }
}

/**
 * Called by the parser when the parser gets the starting element.
 * @param userData Not used.
 * @param name Element name string.
 * @param atts Attributes.
 */
static void XMLCALL on_elementstart(void *userData, const XML_Char *name, const XML_Char *atts[]) {
  if( errorcode != ERR_NOERROR ) {
    return;
  }

  DataList_Clear(stringbuffer);
  if( strcmp("gml:tupleList", (const char *)name) == 0 ) {
    tupple_status = 1;
  }
  else {
    tupple_status = 0;
  }
}

/**
 * (INTERNAL) Called by the parser when the parser gets the ending element.
 * @param userData Not used.
 * @param name Element name string.
 */
static void XMLCALL on_elementend(void *userData, const XML_Char *name) {
  if( errorcode != ERR_NOERROR ) {
    return;
  }

  if( name != NULL ) {
    if( tupple_status == 1 ) {
      tupple_status = 0;
    }
    else if( strcmp("gml:low", (const char *)name) == 0 ) {
      if( extractUint16Couple(stringbuffer, &(rasterbuffer->header.pminx), &(rasterbuffer->header.pminy)) != 2 ) {
        errorcode = ERR_INVALID_LOW_VALUE;
        return;
      }
    }
    else if( strcmp("gml:high", (const char *)name) == 0 ) {
      if( extractUint16Couple(stringbuffer, &(rasterbuffer->header.pmaxx), &(rasterbuffer->header.pmaxy)) != 2 ) {
        errorcode = ERR_INVALID_HIGH_VALUE;
        return;
      }
    }
    else if( strcmp("gml:lowerCorner", (const char *)name) == 0 ) {
      if( extractDoubleCouple(stringbuffer, &(rasterbuffer->header.uminy), &(rasterbuffer->header.uminx)) != 2 ) {
        errorcode = ERR_INVALID_LOWERCORNER_VALUE;
        return;
      }
    }
    else if( strcmp("gml:upperCorner", (const char *)name) == 0 ) {
      if( extractDoubleCouple(stringbuffer, &(rasterbuffer->header.umaxy), &(rasterbuffer->header.umaxx)) != 2 ) {
        errorcode = ERR_INVALID_UPPERCORNER_VALUE;
        return;
      }
    }
    else if( strcmp("gml:startPoint", (const char *)name) == 0 ) {
      if( extractUint16Couple(stringbuffer, &(bandbuffer->pstartx), &(bandbuffer->pstarty)) != 2 ) {
        errorcode = ERR_INVALID_STARTPOINT_VALUE;
        return;
      }
    }
  }
  DataList_Clear(stringbuffer);
}

/**
 * (INTERNAL) Converts Shift JIS character into '-'.
 * @param data NOT USED.
 * @param s Shift JIS text.
 * return UTF character code.
 *
 */
static int fromsjis(void *data, const char *s) {
	return '_';
}

/**
 * (INTERNAL) Called by the parser when the parser gets the xml declaration element
 *  with unknown charset (attribute) value.
 * @param encodingHandlerData Not used.
 * @param name Charset string.
 * @param info Pointer of XML Encoding Information, this function sets values of members.
 */
static int on_unknown_encoding(void *encodingHandlerData, const XML_Char *name, XML_Encoding *info) {
	if (string_caseinsentive_compare(name, "SHIFT_JIS") == 0) {
		int n;
		for (n = 0x00; n <= 0xFF; n++) {
			info->map[n] = n;
		}
		for (n = 0x80; n <= 0x9F; n++) {
			info->map[n] = -2; /* -2 */
		}
		for (n = 0xE0; n <= 0xFC; n++) {
			info->map[n] = -2; /* -2 */
		}
		info->convert = fromsjis;
		return 1;
	}
	else {
		return 0;
	}
}

/**
* Reads FGDDEM XML File
* @param fp File pointer.
* @return RasterBuffer pointer. You must call RasterBuffer_Free() if you want to expire this.
*/
RasterBuffer *readFGDDEMXML(FILE *fp) {
  static char buff[BUFSIZ];
  size_t len;
  XML_Parser parser = NULL;

  rasterbuffer = NULL;
  stringbuffer = NULL;
  bandbuffer = NULL;
  errorcode = ERR_NOERROR;

  rasterbuffer = RasterBuffer_New();
  if( rasterbuffer == NULL ) {
    goto ON_ERROR;
  }
  stringbuffer = DataList_New(1024, sizeof(char));
  if( stringbuffer == NULL ) {
    goto ON_ERROR;
  }
  bandbuffer = RasterBuffer_NewBandBuffer(rasterbuffer, BVT_32BF);
  bandbuffer->hasnodata = 1;
  bandbuffer->nodata.v32bf = (float)-9999.0;

  if ((parser = XML_ParserCreate(NULL)) == NULL) {
    print_error("Could not create parser.");
    goto ON_ERROR;
  }
  XML_SetElementHandler(parser, on_elementstart, on_elementend);
  XML_SetCharacterDataHandler(parser, on_character);
  XML_SetUnknownEncodingHandler(parser, on_unknown_encoding, NULL);

  while( !feof(fp) ) {
    len = fread(buff, sizeof(char), BUFSIZ, fp);
    if( ferror(fp) ) {
      print_error("An error occurred in fread().");
      goto ON_ERROR;
    }
    if( XML_Parse(parser, buff, (int)len, feof(fp)) == XML_STATUS_ERROR ) {
      print_errorln(XML_ErrorString(XML_GetErrorCode(parser)));
	  errorcode = ERR_PARSERERROR;
	  goto ON_ERROR;
    }
    if( errorcode ) {
      goto ON_ERROR;
    }
  }

  DataList_Free(stringbuffer);
  return rasterbuffer;

ON_ERROR:
  print_fgddem_error();
  if( stringbuffer != NULL ) {
    DataList_Free(stringbuffer);
    stringbuffer = NULL;
  }
  if( rasterbuffer != NULL ) {
    RasterBuffer_Free(rasterbuffer);
    rasterbuffer = NULL;
  }
  return NULL;
}

