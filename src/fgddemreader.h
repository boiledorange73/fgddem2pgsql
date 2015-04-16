#ifndef _FGDDEMREADER_HEAD_
#define _FGDDEMREADER_HEAD_

#include <stdio.h>
#include <expat.h>
#include "rasterbuffer.h"

#ifndef _FGDDEMREADER_EXTERN 
#define _FGDDEMREADER_EXTERN extern
#endif

/**
* Reads FGDDEM XML File
* @param fp File pointer.
* @return RasterBuffer pointer. You must call RasterBuffer_Free() if you want to expire this.
*/
_FGDDEMREADER_EXTERN RasterBuffer *readFGDDEMXML(FILE *fp);

#endif /* band.h */
