#ifndef _DATALISTCELL_C_
#define _DATALISTCELL_C_

#include <stdlib.h> /* malloc, NULL */
#include <string.h> /* memcpy */

#ifdef _DATALISTCELL_INTERNAL_
#  ifndef _PRINTERROR_INTERNAL_
#    define _PRINTERROR_INTERNAL_
#  endif
#  include "printerror.c"
#else
#  include "printerror.h"
#endif

#ifndef _DATALISTCELL_EXTERN
#  ifdef _DATALISTCELL_INTERNAL_
#    define _DATALISTCELL_EXTERN static
#  else
#    define _DATALISTCELL_EXTERN
#  endif
#endif
#include "datalistcell.h"

DataListCell *DataListCell_New(int bytes) {
  DataListCell *datalistcell;
  datalistcell = malloc(sizeof(DataListCell)+bytes);
  if( datalistcell == NULL ) {
    print_perror("malloc");
    return NULL;
  }
  datalistcell->body = (char *)datalistcell + sizeof(DataListCell);
  datalistcell->prev = NULL;
  datalistcell->next = NULL;
  return datalistcell;
}

void DataListCell_Free(DataListCell *datalistcell) {
  while( datalistcell != NULL ) {
    DataListCell *next;
    next = datalistcell->next;
    free(datalistcell);
    datalistcell = next;
  }
}

#endif /* datalistcell.c */
