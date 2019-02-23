/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   utils.h
 * Author: gabriel
 *
 * Created on November 10, 2018, 11:28 AM
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TINY_INDEX_MASK 0x3F
#define SMALL_INDEX_MASK 0xFFF
#define INDEX_INDEX_MASK 0xFFFFFF

    typedef unsigned char TinyId;
    typedef unsigned short SmallId;
    typedef unsigned int Id;
    typedef unsigned char TinyIndex;
    typedef unsigned short SmallIndex;
    typedef unsigned int Index;

#define VOID_TINY_ID ((TinyId) -1)
#define VOID_SMALL_ID ((SmallId) -1)
#define VOID_ID ((Id) -1)
#define VOID_TINY_INDEX ((TinyIndex) -1)
#define VOID_SMALL_INDEX ((SmallIndex) -1)
#define VOID_INDEX ((Index) -1)

#define HAS_ID(strct, id) (strct.ids[strct.indexes[id]] == id)

#define SET_STRUCT_FOR_DOD(index_type, max) \
    index_type indexes[max]; \
    index_type next_free_index; \
    index_type total; \
    index_type ids[max]

#define SET_ID_INDEX(strct, id, index) \
do { \
    strct.indexes[id] = index; \
} while (0)

#define INC_ID_INDEX(strct, id) \
do { \
    strct.indexes[id]++; \
} while (0)

#define GET_NAME( var ) #var

#define GET_NEXT_ID(strct, id, index, max) \
do \
{ \
  if( strct.total == max - 1 ) \
  { \
    fprintf( stderr, "%s limit is exceeding the maximum of %d\n", #strct , max ); \
    exit( EXIT_FAILURE ); \
  } \
  id = strct.next_free_index; \
  strct.next_free_index = strct.indexes[id]; \
  index = strct.indexes[id] = strct.total++; \
  strct.ids[index] = id; \
} while (0)

#define INIT_STRUCT_FOR_DOD_FREE_LIST(strct, length) \
do { \
    for(unsigned long i = 0ul; i < (unsigned long) length; i++ ) { \
        strct.indexes[i] = i + 1ul; \
    } \
} while(0)

#define ARRAY_LENGTH(arr) sizeof(arr) / sizeof(arr[0])
#define SHIFT_ONE_POSITION(arr, i, limit, type) memcpy(&arr[i + 1], &arr[i], (limit - i - 1) * sizeof(type))

#define EXIT_IF_HAS_NOT_ID( strct, id ) \
do \
{ \
  if( !HAS_ID( strct, id ) ) \
  { \
    fprintf( stderr, "Trying to access invalid id: %d for: %s\n", id , #strct ); \
    exit( EXIT_FAILURE ); \
  } \
} \
while( 0 )

#define DELETE_DOD_ELEMENT_BY_ID(strct, id, index, last) \
do \
{ \
  GET_INDEX_FROM_ID(strct, id, index); \
  last = --strct.total; \
  strct.indexes[id] = strct.next_free_index; \
  strct.next_free_index = id; \
  if(strct.ids[index] == strct.ids[last]) { \
    break; \
  } \
  strct.ids[index] = strct.ids[last]; \
  strct.indexes[strct.ids[index]] = index; \
} while(0)

#define GET_INDEX_FROM_ID(strct, id, index) \
do \
{ \
  EXIT_IF_HAS_NOT_ID( strct, id ); \
  index = strct.indexes[id]; \
} \
while( 0 )

double utils_random();

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */

