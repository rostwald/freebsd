/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013 Mellanox Technologies, Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef	_LINUX_GFP_H_
#define	_LINUX_GFP_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/rwlock.h>

#include <linux/page.h>

#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_extern.h>
#include <vm/vm_kern.h>

#define	__GFP_NOWARN	0
#define	__GFP_HIGHMEM	0
#define	__GFP_ZERO	M_ZERO
#define	__GFP_NORETRY	0
#define	__GFP_RECLAIM   0
#define	__GFP_RECLAIMABLE   0

#define	__GFP_IO	0
#define	__GFP_NO_KSWAPD	0
#define	__GFP_WAIT	M_WAITOK
#define	__GFP_DMA32     0

#define	GFP_NOWAIT	M_NOWAIT
#define	GFP_ATOMIC	(M_NOWAIT | M_USE_RESERVE)
#define	GFP_KERNEL	M_WAITOK
#define	GFP_USER	M_WAITOK
#define	GFP_HIGHUSER	M_WAITOK
#define	GFP_HIGHUSER_MOVABLE	M_WAITOK
#define	GFP_IOFS	M_NOWAIT
#define	GFP_NOIO	M_NOWAIT
#define	GFP_DMA32	0
#define	GFP_TEMPORARY	M_NOWAIT

static inline void *
page_address(struct page *page)
{

	if (page->object != kmem_object && page->object != kernel_object)
		return (NULL);
	return ((void *)(uintptr_t)(VM_MIN_KERNEL_ADDRESS +
	    IDX_TO_OFF(page->pindex)));
}

static inline unsigned long
linux_get_page(gfp_t mask)
{

	return kmem_malloc(kmem_arena, PAGE_SIZE, mask);
}

#define	get_zeroed_page(mask)	linux_get_page((mask) | M_ZERO)
#define	__get_free_page(mask)	linux_get_page((mask))

static inline void
__free_hot_cold_page(vm_page_t page)
{
	vm_object_t object;

	/* XXX unsafe */
	if ((object = page->object))
		VM_OBJECT_WLOCK(object);

	vm_page_lock(page);
	if (page->hold_count) {
		MPASS(page->hold_count == 1);
		page->hold_count--;
	}
	if (page->wire_count) {
		MPASS(page->wire_count == 1);
		vm_page_unwire(page, PQ_INACTIVE);
	}
	if (pmap_page_is_mapped(page))
		pmap_remove_all(page);
	vm_page_free(page);
	vm_page_unlock(page);
	if (object)
		VM_OBJECT_WUNLOCK(object);
}

static inline void
__free_pages_ok(struct page *m, unsigned int order)
{
	size_t size;

	if (m == NULL)
		return;
	size = PAGE_SIZE << order;
	kmem_free(kmem_arena, (vm_offset_t)page_address(m), size);
}

static inline void
__free_pages(struct page *page, unsigned int order)
{
	if (order == 0)
		__free_hot_cold_page(page);
	else
		__free_pages_ok(page, order);
}

static inline void
free_pages(uintptr_t addr, unsigned int order)
{
	if (addr == 0)
		return;
	__free_pages(virt_to_page((void *)addr), order);
}


#define __free_page(page) __free_pages((page), 0)
#define free_page(addr) free_pages((addr), 0)


/*
 * Alloc pages allocates directly from the buddy allocator on linux so
 * order specifies a power of two bucket of pages and the results
 * are expected to be aligned on the size as well.
 */
static inline struct page *
alloc_pages(gfp_t gfp_mask, unsigned int order)
{
	unsigned long page;
	size_t size;

	size = PAGE_SIZE << order;
	page = kmem_alloc_contig(kmem_arena, size, gfp_mask,
	    0, ~(vm_paddr_t)0, size, 0, VM_MEMATTR_DEFAULT);
	if (page == 0)
		return (NULL);
        return (virt_to_page(page));
}

static inline uintptr_t __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
	struct page *page;

	page = alloc_pages(gfp_mask, order);
	if (page == NULL)
		return (0);
	return ((uintptr_t)page_address(page));
}

#define alloc_pages_node(node, mask, order)     alloc_pages(mask, order)

#define kmalloc_node(chunk, mask, node)         kmalloc(chunk, mask)

/*
 * XXX this actually translates to wired
 *
 * PG_reserved is set for special pages, which can never be swapped out. Some
 * of them might not even exist (eg empty_bad_page)...
 *
 */
#define	SetPageReserved(page)	do { } while (0)	/* NOP */
#define	ClearPageReserved(page)	do { } while (0)	/* NOP */

#endif	/* _LINUX_GFP_H_ */
