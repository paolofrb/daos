/**
 * (C) Copyright 2016 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * This file is part of daos
 *
 * src/include/daos/pool_map.h
 */
#ifndef __DAOS_POOL_H__
#define __DAOS_POOL_H__

#include <daos/common.h>

/**
 * pool component types
 */
typedef enum pool_comp_type {
	PO_COMP_TP_UNKNOWN	= 0,
	PO_COMP_TP_ROOT		= 1,
	PO_COMP_TP_RACK		= 10,
	PO_COMP_TP_BLADE	= 20,
	PO_COMP_TP_BOARD	= 30,
	PO_COMP_TP_NODE		= 40,
	PO_COMP_TP_TARGET	= 50,
	/* TODO: more types */
} pool_comp_type_t;

/** pool component states */
typedef enum pool_comp_state {
	PO_COMP_ST_UNKNOWN,
	/** intermediate state for pool map change */
	PO_COMP_ST_NEW,
	/** component is healthy */
	PO_COMP_ST_UP,
	/** component is healthy and integrated in storage pool */
	PO_COMP_ST_UPIN,
	/** component is dead */
	PO_COMP_ST_DOWN,
	/** component is dead, its data has already been rebuilt */
	PO_COMP_ST_DOWNOUT,
} pool_comp_state_t;

/** parent class of all all pool components: target, domain */
struct pool_component {
	/** pool_comp_type_t */
	uint16_t		co_type;
	/** pool_comp_state_t */
	uint8_t			co_status;
	/** padding byte */
	uint8_t			co_padding;
	/** Immutable component ID. */
	uint32_t		co_id;
	/**
	 * e.g. rank in the communication group, only used by PO_COMP_TARGET
	 * for the time being.
	 */
	uint32_t		co_rank;
	/** version it's been added */
	uint32_t		co_ver;
	/** failure sequence */
	uint32_t		co_fseq;
	/** number of children or VOSs */
	uint32_t		co_nr;
};

/** a leaf of pool map */
struct pool_target {
	/** embedded component for myself */
	struct pool_component	 ta_comp;
	/** nothing else for the time being */
};

/**
 * an intermediate component in pool map, a domain can either contains low
 * level domains or just leaf targets.
 */
struct pool_domain {
	/** embedded component for myself */
	struct pool_component	 do_comp;
	/** # all targets within this domain */
	unsigned int		 do_target_nr;
	/**
	 * child domains within current domain, it is NULL for the last
	 * level domain.
	 */
	struct pool_domain	*do_children;
	/**
	 * all targets within this domain
	 * for the last level domain, it points to the first direct targets
	 * for the intermediate domain, it ponts to the first indirect targets
	 */
	struct pool_target	*do_targets;
};

#define do_child_nr		do_comp.co_nr
#define do_cpu_nr		do_comp.co_nr

/**
 * pool component buffer, it's a contiguous buffer which includes portion of
 * or all components of a pool map.
 */
struct pool_buf {
	/** checksum of components */
	uint32_t		pb_csum;
	/** summary of domain_nr and target_nr, buffer size */
	uint32_t		pb_nr;
	uint32_t		pb_domain_nr;
	uint32_t		pb_target_nr;
	/** buffer body */
	struct pool_component	pb_comps[0];
};

struct pool_buf *pool_buf_alloc(unsigned int nr);
void pool_buf_free(struct pool_buf *buf);
int  pool_buf_attach(struct pool_buf *buf, struct pool_component *comps,
		     unsigned int comp_nr);
int  pool_buf_pack(struct pool_buf *buf);
int  pool_buf_unpack(struct pool_buf *buf);

struct pool_map;
int  pool_map_create(struct pool_buf *buf, uint32_t version,
		     struct pool_map **mapp);
void pool_map_destroy(struct pool_map *map);
int  pool_map_extend(struct pool_map *map, uint32_t version,
		     struct pool_buf *buf);
void pool_map_print(struct pool_map *map);

int  pool_map_set_version(struct pool_map *map, uint32_t version);
uint32_t pool_map_get_version(struct pool_map *map);

#define PO_COMP_ID_ALL		(-1)

int pool_map_find_target(struct pool_map *map, uint32_t id,
			 struct pool_target **target_pp);
int pool_map_find_domain(struct pool_map *map, pool_comp_type_t type,
			 uint32_t id, struct pool_domain **domain_pp);

pool_comp_state_t pool_comp_str2state(const char *name);
const char *pool_comp_state2str(pool_comp_state_t state);

pool_comp_type_t pool_comp_abbr2type(char abbr);
pool_comp_type_t pool_comp_str2type(const char *name);
const char *pool_comp_type2str(pool_comp_type_t type);

static inline const char *pool_comp_name(struct pool_component *comp)
{
	return pool_comp_type2str(comp->co_type);
}

#define pool_target_name(target)	pool_comp_name(&(target)->ta_comp)
#define pool_domain_name(domain)	pool_comp_name(&(domain)->do_comp)

#endif /* __DAOS_POOL_H__ */