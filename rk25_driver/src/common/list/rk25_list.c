#pragma once
#include "rk25_list.h"

#define RK25_LIST_ALLOCATE_TAG 'TSIL'

PRK25_LC RK25ListInit(ULONG size) {
	if (!size) return NULL;

	PRK25_LC lc = (PRK25_LC)ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(RK25_LC),
		RK25_LIST_ALLOCATE_TAG
	);

	if (!lc) return NULL;

	RtlZeroMemory(lc, sizeof(RK25_LC));
	ExInitializeNPagedLookasideList(
		&lc->las,
		NULL,
		NULL,
		0,
		sizeof(RK25_LD),
		RK25_LIST_ALLOCATE_TAG,
		0
	);

	KeInitializeSpinLock(&lc->lock);
	lc->size = size; // for allocate entry->data
	InitializeListHead(&lc->list.link);
	return lc;
}

BOOLEAN RK25ListAdd(PRK25_LC lc, PVOID data) {
	if (!data) return FALSE;

	KIRQL oldIrql;
	KeAcquireSpinLock(&lc->lock, &oldIrql);

	BOOLEAN added = FALSE;
	PRK25_LD entry = (PRK25_LD)ExAllocateFromNPagedLookasideList(&lc->las);
	if (entry) {
		entry->data = ExAllocatePoolWithTag(NonPagedPool, lc->size, RK25_LIST_ALLOCATE_TAG);
		if (entry->data) {
			RtlCopyMemory(entry->data, data, lc->size);
			InsertTailList(&lc->list.link, &entry->link);
			added = TRUE;
		} else ExFreeToNPagedLookasideList(&lc->las, entry);
	}

	KeReleaseSpinLock(&lc->lock, oldIrql);
	return added;
}

PVOID RK25ListEntryNext(PRK25_LC lc) {
	if (!lc) return NULL;

	PVOID ret = NULL;
	KIRQL oldIrql;
	KeAcquireSpinLock(&lc->lock, &oldIrql);

	if (!IsListEmpty(&lc->list.link)) {
		if (&lc->list.link != lc->next) {
			if (!lc->next)
				lc->next = lc->list.link.Flink;
			else
				lc->next = lc->next->Flink;

			if (lc->next != &lc->list.link) {
				PRK25_LD entry = CONTAINING_RECORD(lc->next, RK25_LD, link);
				ret = entry->data;
			}
		}
	}

	KeReleaseSpinLock(&lc->lock, oldIrql);
	return ret;
}

VOID RK25ListFlush(PRK25_LC lc) {
	if (!lc) return;

	KIRQL oldIrql;
	KeAcquireSpinLock(&lc->lock, &oldIrql);

	lc->next = NULL;

	KeReleaseSpinLock(&lc->lock, oldIrql);
	return;
}

// non exported
VOID RK25ListEntryFree(PRK25_LC lc, PRK25_LD entry) {
	if (!entry) return;
	if (entry->data)
		ExFreePool(entry->data);
	ExFreeToNPagedLookasideList(&lc->las, entry);
}

VOID RK25ListEntryDel(PRK25_LC lc, PVOID data) {
	if (!data) return;

	KIRQL oldIrql;
	KeAcquireSpinLock(&lc->lock, &oldIrql);

	for (PLIST_ENTRY pLink = lc->list.link.Flink;
		pLink != &lc->list.link;
		pLink = pLink->Flink) {
		PRK25_LD entry = CONTAINING_RECORD(pLink, RK25_LD, link);
		if (RtlEqualMemory(entry->data, data, lc->size)) {
			RemoveEntryList(&entry->link);
			RK25ListEntryFree(lc, entry);
			break;
		}
	}
	KeReleaseSpinLock(&lc->lock, oldIrql);
}

VOID RK25ListFree(PRK25_LC lc) {
	KIRQL oldIrql;
	KSPIN_LOCK lock = lc->lock; // lc will be free
	KeAcquireSpinLock(&lock, &oldIrql);

	while (!IsListEmpty(&lc->list.link)) {
		PLIST_ENTRY pLink = RemoveHeadList(&lc->list.link);
		PRK25_LD entry = CONTAINING_RECORD(pLink, RK25_LD, link);
		RK25ListEntryFree(lc, entry);
	}
	ExDeleteNPagedLookasideList(&lc->las);
	ExFreePool(lc);
	KeReleaseSpinLock(&lock, oldIrql);
	return;
}