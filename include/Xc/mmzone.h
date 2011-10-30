#ifndef _XC_MMZONE_H
#define _XC_MMZONE_H

#include <Xc/list.h>
#include <Xc/spinlock.h>
#include <Xc/wait.h>
#include <Xc/atomic.h>
#include <Xc/types.h>
#include <Xc/bitops.h>
#include <Xc/kernel.h>
#include <Xc/page-flags.h>
#include <Xc/nodemask.h>

#define NUMA_NO_NODE (-1)

/* asm/topology.h */
static inline int numa_node_id(void)
{
    return 0;
}
/* end -- asm/topology.h*/

#define MAX_ORDER 11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

enum pageblock_bits {
    PB_migrate,
	PB_migrate_end = PB_migrate + 3 -1,
	NR_PAGEBLOCK_BITS
};

struct node_active_region {
    unsigned long start_pfn;
	unsigned long end_pfn;
	int nid;
};

enum zone_type {
    ZONE_DMA,
	ZONE_NORMAL,
	ZONE_MOVABLE,
	ZONE_HIGHMEM,
	__MAX_NR_ZONES
};

#define LRU_BASE 0
#define LRU_ACTIVE 1
#define LRU_FILE 2

enum lru_list {
    LRU_INACTIVE_ANON = LRU_BASE,
	LRU_ACTIVE_ANON = LRU_BASE + LRU_ACTIVE,
	LRU_INACTIVE_FILE = LRU_BASE + LRU_FILE,
	LRU_ACTIVE_FILE = LRU_BASE + LRU_FILE +LRU_ACTIVE,
	LRU_UNEVICTABLE,
	NR_LRU_LISTS
};

#define PAGE_ALLOC_COSTLY_ORDER 3
#define MIGRATE_UNMOVABLE       0
#define MIGRATE_RECLAIMABLE     1
#define MIGRATE_MOVABLE         2
#define MIGRATE_PCPTYPES        3
#define MIGRATE_RESERVE         3
#define MIGRATE_ISOLATE         4
#define MIGRATE_TYPES           5

struct free_area {
    struct list_head free_list[MIGRATE_TYPES];
	unsigned long nr_free;
};

#define for_each_migratetype_order(order, type)    \
	for (order = 0; order < MAX_ORDER; order ++)   \
        for (type = 0; type < MIGRATE_TYPES; type++)

/* MAX_NR_ZONES is generated by Kbuild */
#define MAX_NR_ZONES  4

#if MAX_NR_ZONES < 2
#define ZONES_SHIFT 0
#elif MAX_NR_ZONES <= 2
#define ZONES_SHIFT 1
#elif MAX_NR_ZONES <= 4
#define ZONES_SHIFT 2
#else
#error ZONES_SHIFT -- too many zones
#endif


#define MAX_ZONES_PER_ZONELIST (MAX_NR_ZONES * MAX_NUMNODES)
/* 1 under UMA while 2 under NUMA */
#define MAX_ZONELISTS 1

struct zone_reclaim_stat {
    unsigned long recent_rotated[2];
	unsigned long recent_scanned[2];
};

enum zone_stat_item {
	/* First 128 byte cacheline (assuming 64 bit words) */
	NR_FREE_PAGES,
	NR_LRU_BASE,
	NR_INACTIVE_ANON = NR_LRU_BASE, /* must match order of LRU_[IN]ACTIVE */
	NR_ACTIVE_ANON,		/*  "     "     "   "       "         */
	NR_INACTIVE_FILE,	/*  "     "     "   "       "         */
	NR_ACTIVE_FILE,		/*  "     "     "   "       "         */
	NR_UNEVICTABLE,		/*  "     "     "   "       "         */
	NR_MLOCK,		/* mlock()ed pages found and moved off LRU */
	NR_ANON_PAGES,	/* Mapped anonymous pages */
	NR_FILE_MAPPED,	/* pagecache pages mapped into pagetables.
			   only modified from process context */
	NR_FILE_PAGES,
	NR_FILE_DIRTY,
	NR_WRITEBACK,
	NR_SLAB_RECLAIMABLE,
	NR_SLAB_UNRECLAIMABLE,
	NR_PAGETABLE,		/* used for pagetables */
	NR_KERNEL_STACK,
	/* Second 128 byte cacheline */
	NR_UNSTABLE_NFS,	/* NFS unstable pages */
	NR_BOUNCE,
	NR_VMSCAN_WRITE,
	NR_WRITEBACK_TEMP,	/* Writeback using temporary buffers */
	NR_ISOLATED_ANON,	/* Temporary isolated pages from anon lru */
	NR_ISOLATED_FILE,	/* Temporary isolated pages from file lru */
	NR_SHMEM,		/* shmem pages (included tmpfs/GEM pages) */
	NR_DIRTIED,		/* page dirtyings since bootup */
	NR_WRITTEN,		/* page writings since bootup */
#ifdef CONFIG_NUMA
	NUMA_HIT,		/* allocated in intended node */
	NUMA_MISS,		/* allocated in non intended node */
	NUMA_FOREIGN,		/* was intended here, hit elsewhere */
	NUMA_INTERLEAVE_HIT,	/* interleaver preferred this zone */
	NUMA_LOCAL,		/* allocation from local node */
	NUMA_OTHER,		/* allocation from other node */
#endif
	NR_ANON_TRANSPARENT_HUGEPAGES,
	NR_VM_ZONE_STAT_ITEMS };
enum zone_watermarks {
    WMARK_MIN,
	WMARK_LOW,
	WMARK_HIGH,
	NR_WMARK
};

struct per_cpu_pages {
    int count;
	int high;
	int batch;
	
	struct list_head lists[MIGRATE_PCPTYPES];
};

struct per_cpu_pageset {
    struct per_cpu_pages pcp;
};
struct page;
struct zone {
	unsigned long watermark[NR_WMARK];
    unsigned long lowmem_reserve[MAX_NR_ZONES];

	struct per_cpu_pageset *pageset;

	spinlock_t    lock;
	int all_unreclaimable;
	struct free_area free_area[MAX_ORDER];

	unsigned long *pageblock_flags;

	spinlock_t    lru_lock;
	struct zone_lru {
        struct list_head list;
	} lru[NR_LRU_LISTS];

    struct zone_reclaim_stat reclaim_stat;

	unsigned long pages_scanned;
	unsigned long flags;

	atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS];

    wait_queue_head_t  *wait_table;
	unsigned long wait_table_hash_nr_entries;
	unsigned long wait_table_bits;

    struct pglist_data *zone_pgdat;

	unsigned long zone_start_pfn;
	unsigned long spanned_pages;
	unsigned long present_pages;

	const char    *name;
};

struct zonelist_cache {
    unsigned short z_to_n[MAX_ZONES_PER_ZONELIST];
	DECLARE_BITMAP(fullzones, MAX_ZONES_PER_ZONELIST);
	unsigned long last_full_zap;
};

struct zoneref {
    struct zone *zone;
	int zone_idx;
};

/* No definition under UMA */
struct zonelist_cache;

struct zonelist {
    struct zonelist_cache *zlcache_ptr;
	struct zoneref _zonerefs[MAX_ZONES_PER_ZONELIST + 1];
};

struct task_struct;

typedef struct pglist_data {
    struct zone node_zones[MAX_NR_ZONES];
	struct zonelist node_zonelists[MAX_ZONELISTS];
	int nr_zones;

	struct page *node_mem_map;

	unsigned long node_start_pfn;
	unsigned long node_present_pages;
	unsigned long node_spanned_pages;

	int node_id;

	wait_queue_head_t kswapd_wait;
	struct task_struct *kswapd;
	int kswapd_max_order;
	enum zone_type classzone_idx;
} pg_data_t;

extern struct pglist_data contig_page_data;
extern struct page *mem_map;
#define NODE_DATA(nid)    (&contig_page_data)

#define zone_idx(zone) ((zone) - (zone)->zone_pgdat->node_zones)

#define for_each_lru(l) for (l = 0; l < NR_LRU_LISTS; l++)
#define for_each_evictable_lru for (l = 0; l <= LRU_ACTIVE_FILE; l++)

static inline int populated_zone(struct zone *zone)
{
    return (!!zone->present_pages);
}

enum memmap_context {
    MEMMAP_EARLY,
	MEMMAP_HOTPLUG,
};

extern int page_group_by_mobility_disabled;

extern int init_currently_empty_zone(struct zone *zone, unsigned long start_pfn, unsigned long size, 
		enum memmap_context context);


extern int movable_zone;

static inline int zone_movable_is_highmem(void)
{
    return movable_zone == ZONE_HIGHMEM;
}

static inline int is_highmem_idx(enum zone_type idx)
{
    return (idx == ZONE_HIGHMEM ||
			(idx == ZONE_MOVABLE && zone_movable_is_highmem()));
}

static inline int get_pageblock_migratetype(struct page *page)
{
    return get_pageblock_flags_group(page, PB_migrate, PB_migrate_end);
}

#define early_pfn_valid(pfn) (1)
#define early_pfn_in_nid(pfn, nid) (1)
#define pfn_valid_within(pfn) (1)

static inline struct zone *zonelist_zone(struct zoneref *zoneref)
{
    return zoneref->zone;
}

static inline int zonelist_zone_idx(struct zoneref *zoneref)
{
    return zoneref->zone_idx;
}

struct zoneref *next_zones_zonelist(struct zoneref *z, enum zone_type highest_zoneidx, nodemask_t *nodes, 
		struct zone **zone);

static inline struct zoneref *first_zones_zonelist(struct zonelist *zonelist, 
		                                           enum zone_type highest_zoneidx, 
		                                           nodemask_t *nodes, struct zone **zone)
{
    return next_zones_zonelist(zonelist->_zonerefs, highest_zoneidx, nodes, zone);
}

#define for_each_zone_zonelist_nodemask(zone, z, zlist, highidx, nodemask)    \
	for (z = first_zones_zonelist(zlist, highidx, nodemask, &zone);           \
			zone;                                                             \
			z = next_zones_zonelist(++z, highidx, nodemask, &zone))

#endif
