#ifndef _RK25_LIST_H_
#define _RK25_LIST_H_

#include <ntddk.h>

typedef struct _RK25_LIST_DATA {
	LIST_ENTRY link;
	PVOID data;
} RK25_LIST_DATA, RK25_LD, *PRK25_LD;

typedef struct _RK25_LIST_CONF {
	struct {
		LIST_ENTRY link;
	} list;
	NPAGED_LOOKASIDE_LIST las;
	ULONG size;
	KSPIN_LOCK lock;
	PLIST_ENTRY next;
} RK25_LIST_CONF, RK25_LC, * PRK25_LC;

/*
* @brief Initialize list head and lookaside
* 
* @return List config
*/
PRK25_LC RK25ListInit(ULONG size);

/*
* @brief Copy data and add to list
*/
BOOLEAN RK25ListAdd(PRK25_LC lc, PVOID data);

/*
* @brief Get data of the next entry list
*/
PVOID RK25ListEntryNext(PRK25_LC lc);

/*
* @brief Set default parameters
*/
VOID RK25ListFlush(PRK25_LC lc);

/*
* @brief Compare memory and remove entry
*/
VOID RK25ListEntryDel(PRK25_LC lc, PVOID data);

/*
* @brief Free all data list and list config
*/
VOID RK25ListFree(PRK25_LC lc);

#endif // !_RK25_LIST_H_
