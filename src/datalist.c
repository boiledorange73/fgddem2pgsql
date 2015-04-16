#ifndef _DATALIST_C_
#define _DATALIST_C_

#include <string.h> /* memcopy */
#include <stdlib.h> /* malloc */

#ifdef _DATALIST_INTERNAL_
#  ifndef _DATALISTCELL_INTERNAL_
#    define _DATALISTCELL_INTERNAL_
#  endif
#  include "datalistcell.c"
#
#  ifndef _PRINTERROR_INTERNAL_
#    define _PRINTERROR_INTERNAL_
#  endif
#  include "printerror.c"
#else
#  include "printerror.h"
#  include "datalist.h"
#endif

#ifndef _DATALIST_EXTERN
#  ifdef _DATALIST_INTERNAL_
#    define _DATALIST_EXTERN static
#  else
#    define _DATALIST_EXTERN
#  endif
#endif
#include "datalist.h"

static int _DataList_GetPointers(DataList *datalist, int index, DataListCell **ppDataListCell, int *pIndex);
static int _DataList_CopyTo(DataList *datalist, DataListCell *cell, int index, void *dst, int count);

/**
 * Constructs New DataList
 * @param cell_elements Element count in one cell.
 * @param element_bytes Bytes in one element.
 * @return New DataList instance. If malloc error occurres, returns NULL.
 */
DataList *DataList_New(int cell_elements, int element_bytes) {
  DataList *datalist;
  datalist = malloc(sizeof(DataList));
  if( datalist == NULL ) {
    print_perror("malloc");
    return NULL;
  }
  datalist->cell_elements = cell_elements;
  datalist->element_bytes = element_bytes;
  datalist->length = 0;
  datalist->head = DataListCell_New(datalist->cell_elements * datalist->element_bytes);
  if( datalist->head == NULL ) {
    free(datalist);
    return NULL;
  }
  datalist->tail = datalist->head;
  datalist->hix = 0;
  datalist->tix = 0;
  return datalist;
}

/**
 * Destructs DataList
 * @param datalist DataList instance.
 */
void DataList_Free(DataList *datalist) {
  DataListCell_Free(datalist->head);
  free(datalist);
}

/**
 * Clears all elements.
 * NOTE: This function DOES NOT destruct each element.
 * @param datalist DataList instance.
 */
void DataList_Clear(DataList *datalist) {
  datalist->tail = datalist->head;
  datalist->hix = 0;
  datalist->tix = 0;
}

/**
 * Adds copied elements at the end of the list.
 * NOTE: If fails to allocate new cell, returns from halfway.
 * @param datalist DataList instance.
 * @param src Pointer to source data.
 * @param count Element count (NOT bytes) of source data.
 * @return Count of actually copied data.
 */
int DataList_Push(DataList *datalist, void *src, int count) {
  int oldlength;
  oldlength = datalist->length;
  while( count > 0 ) {
    int count_one;
    int bytes;
    /* new cell if needed */
    if( datalist->tix >= datalist->cell_elements ) {
      if( datalist->tail->next != NULL ) {
        datalist->tail = datalist->tail->next;
        datalist->tix = 0;
      }
      else {
        datalist->tail->next = DataListCell_New(datalist->cell_elements * datalist->element_bytes);
        if( datalist->tail->next == NULL ) {
          /* error occurred. */
          return datalist->length - oldlength;
        }
        datalist->tail->next->prev = datalist->tail;
        datalist->tail = datalist->tail->next;
        datalist->tix = 0;
      }
    }
    if( datalist->tail == NULL ) {
      break;
    }
    /* copies */
    if( datalist->tix + count < datalist->cell_elements ) {
      /* within current cell */
      count_one = count;
    }
    else {
      /* over current cell */
      count_one = datalist->cell_elements - datalist->tix;
    }
    bytes = count_one * datalist->element_bytes;
    memcpy(
      (void *)((char *)(datalist->tail->body)+datalist->tix*datalist->element_bytes),
      (const void *)src,
      bytes);
    src = (void *)((char *)src + bytes);
    datalist->tix = datalist->tix + count_one;
    datalist->length += count_one;
    count -= count_one;
  }
  return datalist->length - oldlength;
}

/**
 * Copies latter elements and reduces.
 * @param datalist DataList instance.
 * @param dst Pointer to destination memory.
 * @param count Element count (NOT bytes) to copy.
 * @return Count of actually copied data.
 */
int DataList_Pop(DataList *datalist, void *dst, int count) {
  DataListCell *cell;
  int ix_cell;
  int index = -1;
  int rest;
  int ret;

  cell = datalist->tail;
  ix_cell = datalist->tix;
  rest = count;
  while( rest > 0 && cell != NULL ) {
    if( ix_cell >= rest ) {
      /* within the cell */
      ix_cell = ix_cell - rest;
      rest = 0;
    }
    else {
      /* over the cell */
      rest -= ix_cell;
      ix_cell = datalist->cell_elements;
      cell = cell->prev;
    }
  }
  /* adjusts cell and index */
  if( cell == NULL ) {
    cell = datalist->head;
    ix_cell = 0;
  }
  /* copy */
  ret = _DataList_CopyTo(datalist, cell, ix_cell, dst, count);
  /* truncate */
  datalist->tail = cell;
  datalist->tix = ix_cell;
  datalist->length -= ret;
  return ret;
}

/**
 * Copies elder elements and reduces.
 * @param datalist DataList instance.
 * @param dst Pointer to destination memory.
 * @param count Element count (NOT bytes) to copy.
 * @return Count of actually copied data.
 */
int DataList_Shift(DataList *datalist, void *dst, int count) {
  int oldlength;
  oldlength = datalist->length;
  while( count > 0 && (datalist->head != datalist->tail || datalist->hix < datalist->tix) ) {
    int elements_cell;
    int cell_elements_one;
    int count_one;
    int bytes_one;
    /* copies */
    cell_elements_one = datalist->cell_elements;
    if( datalist->head == datalist->tail ) {
      cell_elements_one = datalist->tix;
    }
    if( datalist->hix + count < cell_elements_one ) {
      /* within current cell */
      count_one = count;
    }
    else {
      /* over current cell */
      count_one = cell_elements_one - datalist->hix;
    }
    bytes_one = count_one * datalist->element_bytes;
    if( dst != NULL ) {
      memcpy(dst, (char *)(datalist->head->body)+datalist->hix*datalist->element_bytes, bytes_one);
      dst = (void *)((char *)dst + bytes_one);
    }
    datalist->hix = datalist->hix + count_one;
    datalist->length -= count_one;
    count -= count_one;
    /* moves */
    if( datalist->head == datalist->tail && datalist->hix >= datalist->tix ) {
      /* empty */
      datalist->hix = 0;
      datalist->tix = 0;
    } 
    else if( datalist->hix >= datalist->cell_elements ) {
      DataListCell *oldcell;
      DataListCell *cell;
      oldcell = datalist->head;
      datalist->head = datalist->head->next;
      datalist->head->prev = NULL;
      for( cell = datalist->tail; cell->next != NULL; cell = cell->next ) {
        /* DOES NOTHING */
      }
      cell->next = oldcell;
      oldcell->prev = cell;
      oldcell->next = NULL;
      datalist->hix = 0;
    }
  }
  return oldlength - datalist->length;
}

/**
 * Inserts copied elements at the start of the list.
 * NOTE: If fails to allocate new cell, returns from halfway.
 * @param datalist DataList instance.
 * @param src Pointer to source data.
 * @param count Element count (NOT bytes) of source data.
 * @return Count of actually copied data.
 */
int DataList_Unshift(DataList *datalist, void *src, int count) {
  char *srctail;
  int ret = 0;
  srctail = ((char *)src) + (count * datalist->element_bytes);
  while( count > 0 ) {
    int copied_elements;
    if( datalist->hix <= 0 ) {
      /* new */
      DataListCell *newhead = NULL;
      if( datalist->tail->next != NULL ) {
        /* brings tail->next to head */
        newhead = datalist->tail->next;
        datalist->tail->next = newhead->next;
      }
      else {
        /* new cell */
        newhead = DataListCell_New(datalist->cell_elements * datalist->element_bytes);
        if( newhead == NULL ) {
          /* error */
          return ret;
        }
      }
      newhead->next = datalist->head;
      datalist->head = newhead;
      datalist->hix = datalist->cell_elements;
    }
    if( datalist->hix >= count ) {
      copied_elements = count;
    }
    else {
      copied_elements = datalist->hix;
    }

    bcopy(
      srctail - (copied_elements * datalist->element_bytes),
      datalist->head->body+((datalist->hix - copied_elements) * datalist->element_bytes),
      copied_elements * datalist->element_bytes
    );
    datalist->hix -= copied_elements;
    datalist->length += copied_elements;
    srctail -= copied_elements * datalist->element_bytes;
    count -= copied_elements;
    ret += copied_elements;
  }
  if( datalist->hix == datalist->cell_elements ) {
    /* bring head to tail */
    DataListCell *cell;
    for( cell = datalist->tail; cell->next != NULL; cell = cell->next ) {
      /* DOES NOTHING */
    }
    cell->next = datalist->head;
    datalist->head = datalist->head->next;
    cell->next->next = NULL;
    datalist->hix = 0;
  }
  return ret;
}

/**
 * Copies elements.
 * @param datalist DataList instance.
 * @param index Starting index in datalist, starting with 0.
 * @param dst Pointer to destination memory.
 * @param count Element count (NOT bytes) to copy.
 * @return Count of actually copied data.
 */
int DataList_CopyTo(DataList *datalist, int index, void *dst, int count) {
  DataListCell *cell;
  int ix_cell;
  if( !(_DataList_GetPointers(datalist, index, &cell, &ix_cell) > 0) ) {
    return 0;
  }
  return _DataList_CopyTo(datalist, cell, ix_cell, dst, count);
}

/**
 * Searches for the first index of matched data.
 * @param datalist DataList instance.
 * @param pv Pointer to test data, whose size equals element_bytes.
 * @return Index of first matches element starting with 0. If no matched data, returns negative.
 */
int DataList_IndexOf(DataList *datalist, void *pv) {
  int ret = 0;
  DataListCell *cell;
  int index;
  cell = datalist->head;
  index = datalist->hix;
  while( cell != NULL ) {
    char *p, *pt;
    int n;
    int count_one;
    if( cell == datalist->tail ) {
      count_one = datalist->tix;
    }
    else {
      count_one = datalist->cell_elements;
    }
    p = (char *)cell->body + index * datalist->element_bytes;
    pt = (char *)cell->body + count_one * datalist->element_bytes;
    for( ; p < pt; p += datalist->element_bytes ) {
      if( memcmp(p, pv, datalist->element_bytes) == 0 ) {
        return ret;
      }
      ret++;
    }
    if( cell == datalist->tail ) {
      // BREAKS
      break;
    }
    index = 0;
    cell = cell->next;
  }

  return -1;
}

/**
 * Gets the pointer to specified element.
 * @param datalist DataList instance.
 * @param index Index of element, starting with 0.
 * @return Pointer to specified element in datalist. If index is out of bound, returns NULL.
 */
void *DataList_Get(DataList *datalist, int index) {
  DataListCell *cell;
  int ix_cell;
  if( !(_DataList_GetPointers(datalist, index, &cell, &ix_cell) > 0) ) {
    return NULL;
  }
  return (char *)cell->body + datalist->element_bytes * ix_cell;
}

/**
 * STATIC: Get Pointers.
 */
static int _DataList_GetPointers(DataList *datalist, int index, DataListCell **ppDataListCell, int *pIndex) {
  DataListCell *cell;
  int cell_step, n;
  int ix;

  if( index >= datalist->length ) {
    return 0;
  }

  /* hix base to 0 base */
  index += datalist->hix;

  cell_step = index / datalist->cell_elements;
  for( cell=datalist->head, n = 0; cell != NULL && n < cell_step; cell = cell->next, n++ ) {
    /* DOES NOTHING */
  }
  if( cell == NULL ) {
    return 0;
  }
  ix = index % datalist->cell_elements;
  if( cell == datalist->tail && ix >= datalist->tix ) {
    return 0;
  }
  *ppDataListCell = cell;
  *pIndex = ix;
  return 1;
}

/**
 * STATIC: Copies
 */
static int _DataList_CopyTo(DataList *datalist, DataListCell *cell, int index, void *dst, int count) {
  int rest;
  rest = count;
  while( rest > 0 ) {
    int cell_elements_one;
    int count_one;
    int bytes_one;
    cell_elements_one = datalist->cell_elements;
    if( cell == datalist->tail ) {
      cell_elements_one = datalist->tix;
    }
    if( index + rest <= cell_elements_one ) {
      /* within the cell */
      count_one = rest;
    }
    else {
      /* over the cell */
      count_one = cell_elements_one - index;
    }
    bytes_one = count_one * datalist->element_bytes;
    if( dst != NULL ) {
      memcpy(dst, (char *)cell->body + index * datalist->element_bytes, bytes_one);
      dst = (void *)((char *)dst + bytes_one);
    }
    rest -= count_one;
    if( cell == datalist->tail ) {
      /* BREAKS */
      break;
    }
    else {
      /* next cell */
      index = 0;
      cell = cell->next;
    }
  }
  return count - rest;
}

#endif /* datalist.c */

/*
// cc datalist.c datalistcell.c printerror.c
#include <stdio.h>
static void dumpall(DataList *datalist) {
  int cells_cnt = 0;
  DataListCell *cell;
  cells_cnt = 0;
  printf("------ Dump List (length: %d)\n", datalist->length);
  for( cell = datalist->head; cell != datalist->tail->next; cell = cell->next ) {
    int n;
    int *p;
    int cnt_one;
    printf("cell %d %p prev %p next %p \n", cells_cnt, cell, cell->prev, cell->next);
    p = (int *)cell->body;
    cnt_one = cell == datalist->tail ? datalist->tix : datalist->cell_elements;
    n = cell == datalist->head ? datalist->hix : 0;
    printf("%d to %d\n", n, cnt_one -1);
    for( ; n < cnt_one ; n++ ) {
      printf("%d ", p[n]);
    }
    printf("\n");
    cells_cnt++;
  }
printf("------\n");
}

int main(int ac, char *av[]) {
  DataList *datalist;
  static int ary[1024];
  int n;
  int ret;
  // Test
  datalist = DataList_New(2, sizeof(int));
  // Pushes for each.
  for( n = 0; n < 10; n++ ) {
    ary[0] = n + 1;
    DataList_Push(datalist, ary, 1);
  }

dumpall(datalist);

  printf("-- shift\n");
  ret = DataList_Shift(datalist, ary, 2);

  printf("ret: %d -", ret);
  for( n = 0; n < ret; n++ ) {
    printf(" %d", ary[n]);
  }
  printf("\n");

dumpall(datalist);

  printf("-- unshift\n");
  for( n = 0; n < 5; n++ ) {
    ary[n] = -n-1;
    printf(" %d", ary[n]);
  }
  DataList_Unshift(datalist, ary, 5);
  printf("\n");

dumpall(datalist);

  int v;
  v = 0;
  printf("  Index Of (%d) -> %d\n", v, DataList_IndexOf(datalist, &v));
  v = 8;
  printf("  Index Of (%d) -> %d\n", v, DataList_IndexOf(datalist, &v));
  v = 10;
  printf("  Index Of (%d) -> %d\n", v, DataList_IndexOf(datalist, &v));
  v = 11;
  printf("  Index Of (%d) -> %d\n", v, DataList_IndexOf(datalist, &v));

  printf("-- pop\n");
  ret = DataList_Pop(datalist, ary, 3);

  printf("ret: %d -", ret);
  for( int n = 0; n < ret; n++ ) {
    printf(" %d", ary[n]);
  }
  printf("\n");

dumpall(datalist);

  // Repushes for each.
  for( n = 7; n < 10; n++ ) {
    ary[0] = n + 1;
    DataList_Push(datalist, ary, 1);
  }

dumpall(datalist);

  printf("-- shift\n");
  ret = DataList_Shift(datalist, ary, 10);

  printf("ret: %d -", ret);
  for( n = 0; n < ret; n++ ) {
    printf(" %d", ary[n]);
  }
  printf("\n");

dumpall(datalist);

  // Repushes for each.
  for( n = 0; n < 5; n++ ) {
    ary[0] = n + 1;
    DataList_Push(datalist, ary, 1);
  }

dumpall(datalist);

  printf("-- pop\n");
  ret = DataList_Pop(datalist, ary, 10);

  printf("ret: %d -", ret);
  for( n = 0; n < ret; n++ ) {
    printf(" %d", ary[n]);
  }
  printf("\n");

dumpall(datalist);

  return 0;
}
*/

