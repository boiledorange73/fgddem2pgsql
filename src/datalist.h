#ifndef _DATALIST_HEAD_
#define _DATALIST_HEAD_

#include "datalistcell.h"

#ifndef _DATALIST_EXTERN
#define _DATALIST_EXTERN extern
#endif

struct _DataList {
  int cell_elements;
  int element_bytes;
  int length;
  DataListCell *head;
  DataListCell *tail;
  int hix;
  int tix;
};

typedef struct _DataList DataList;

_DATALIST_EXTERN DataList *DataList_New(int cell_elements, int element_bytes);
_DATALIST_EXTERN void DataList_Free(DataList *datalist);
_DATALIST_EXTERN void DataList_Clear(DataList *datalist);
_DATALIST_EXTERN int DataList_Push(DataList *datalist, void *src, int count);
_DATALIST_EXTERN int DataList_Pop(DataList *datalist, void *dst, int count);
_DATALIST_EXTERN int DataList_Unshift(DataList *datalist, void *src, int count);
_DATALIST_EXTERN int DataList_Shift(DataList *datalist, void *dst, int count);
_DATALIST_EXTERN int DataList_CopyTo(DataList *datalist, int index, void *dst, int count);
_DATALIST_EXTERN void *DataList_Get(DataList *datalist, int index);
_DATALIST_EXTERN int DataList_IndexOf(DataList *datalist, void *pv);

#endif /* datalist.h */
