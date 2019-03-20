/**
 * (C) Copyright 2019 Intel Corporation.
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
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the Apache License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 */
/**
 * dtx: resync DTX status
 */
#define D_LOGFAC	DD_FAC(dtx)

#include <daos/placement.h>
#include <daos/pool_map.h>
#include <daos_srv/vos.h>
#include <daos_srv/pool.h>
#include <daos_srv/dtx_srv.h>
#include <daos_srv/daos_server.h>
#include <daos_srv/container.h>
#include "dtx_internal.h"

struct dtx_resync_entry {
	d_list_t		dre_link;
	struct daos_tx_id	dre_xid;
	daos_unit_oid_t		dre_oid;
	uint32_t		dre_state;
	uint32_t		dre_intent;
	uint64_t		dre_hash;
};

struct dtx_resync_head {
	d_list_t		drh_list;
	int			drh_count;
};

struct dtx_resync_args {
	uuid_t			po_uuid;
	uint64_t		start;
	uint32_t		version;
	struct dtx_resync_head	tables;
};

static int
dtx_check_committable(uuid_t po_uuid, uuid_t co_uuid, struct daos_tx_id *dti,
		      daos_unit_oid_t *oid, struct pl_obj_layout *layout)
{
	struct daos_tx_entry	dte;

	dte.dte_xid = *dti;
	dte.dte_oid = *oid;

	return dtx_check(po_uuid, co_uuid, &dte, layout);
}

static int
dtx_resync_commit(uuid_t po_uuid, uuid_t co_uuid,
		  struct dtx_resync_head *drh, int count, uint32_t version)
{
	struct daos_tx_entry		*dte;
	struct dtx_resync_entry		*dre;
	int				 rc;
	int				 i = 0;

	D_ASSERT(drh->drh_count >= count);

	D_ALLOC_ARRAY(dte, count);
	if (dte == NULL)
		return -DER_NOMEM;

	do {
		dre = d_list_entry(drh->drh_list.next,
				   struct dtx_resync_entry, dre_link);
		d_list_del(&dre->dre_link);
		drh->drh_count--;

		dte[i].dte_xid = dre->dre_xid;
		dte[i].dte_oid = dre->dre_oid;
		D_FREE_PTR(dre);
	} while (++i < count);

	rc = dtx_commit(po_uuid, co_uuid, dte, count, version);
	D_FREE(dte);

	if (rc < 0)
		D_ERROR("Failed to commit the DTX: rc = %d\n", rc);

	return rc > 0 ? 0 : rc;
}

static int
dtx_resync_abort(uuid_t po_uuid, struct ds_cont *cont,
		 struct dtx_resync_head *drh, struct dtx_resync_entry *dre,
		 uint32_t version, bool force)
{
	struct daos_tx_entry	 dte;
	int			 rc;

	/* If we abort multiple non-ready DTXs together, then there is race that
	 * one DTX may become committable when we abort some other DTX. To avoid
	 * complex rollback logic, let's abort the DTXs one by one.
	 */

	if (!force) {
		rc = vos_dtx_lookup_cos(cont->sc_hdl, &dre->dre_oid,
			&dre->dre_xid, dre->dre_hash,
			dre->dre_intent == DAOS_INTENT_PUNCH ? true : false);
		if (rc == 0)
			/* The DTX become committable, should NOT abort it. */
			return 1;

		if (rc != -DER_NONEXIST)
			goto out;
	}

	dte.dte_xid = dre->dre_xid;
	dte.dte_oid = dre->dre_oid;
	rc = dtx_abort(po_uuid, cont->sc_uuid, &dte, 1, version);

out:
	if (rc < 0)
		D_ERROR("Failed to abort the DTX "DF_UOID"/"DF_DTI": rc = %d\n",
			DP_UOID(dre->dre_oid), DP_DTI(&dre->dre_xid), rc);

	d_list_del(&dre->dre_link);
	drh->drh_count--;
	D_FREE_PTR(dre);

	return rc > 0 ? 0 : rc;
}

static int
dtx_placement_handle(struct dtx_resync_args *dra, uuid_t co_uuid)
{
	struct ds_cont			*cont = NULL;
	struct pl_obj_layout		*layout = NULL;
	struct dtx_resync_head		*drh = &dra->tables;
	struct dtx_resync_entry		*dre;
	struct dtx_resync_entry		*next;
	struct daos_oclass_attr		*oc_attr;
	int				 count = 0;
	int				 err = 0;
	int				 rc;

	if (drh->drh_count == 0)
		return 0;

	rc = ds_cont_lookup(dra->po_uuid, co_uuid, &cont);
	if (rc != 0)
		return rc;

	d_list_for_each_entry_safe(dre, next, &drh->drh_list, dre_link) {
		if (layout != NULL) {
			pl_obj_layout_free(layout);
			layout = NULL;
		}

		rc = ds_pool_check_leader(dra->po_uuid, &dre->dre_oid,
					  dra->version, &layout);
		if (rc <= 0) {
			if (rc < 0)
				D_WARN("Not sure about the leader for the DTX "
				       DF_UOID"/"DF_DTI" (ver = %u): rc = %d, "
				       "skip it.\n",
				       DP_UOID(dre->dre_oid),
				       DP_DTI(&dre->dre_xid), dra->version, rc);
			else
				D_DEBUG(DB_TRACE, "Not the leader for the DTX "
					DF_UOID"/"DF_DTI" (ver = %u) skip it\n",
					DP_UOID(dre->dre_oid),
					DP_DTI(&dre->dre_xid), dra->version);

			d_list_del(&dre->dre_link);
			drh->drh_count--;
			D_FREE_PTR(dre);
			continue;
		}

		if (dre->dre_state == DTX_ST_INIT) {
			/* The INIT DTX may become committable when we
			 * scan the DTX tables So re-check its status.
			 */
			rc = vos_dtx_check_committable(cont->sc_hdl,
				&dre->dre_oid, &dre->dre_xid, dre->dre_hash,
				dre->dre_intent == DAOS_INTENT_PUNCH ?
				true : false);
			switch (rc) {
			case DTX_ST_PREPARED:
				break;
			case DTX_ST_INIT:
				/* I am the leader, and if the DTX is in INIT
				 * state, then it must be non-committable case,
				 * unnecessary to check with others. Abort it.
				 */
				rc = dtx_resync_abort(dra->po_uuid, cont, drh,
						      dre, dra->version, true);
				if (rc > 0)
					goto committable;
				if (rc < 0)
					err = rc;
				continue;
			default:
				D_WARN("Not sure about the leader for the DTX "
				       DF_UOID"/"DF_DTI": rc = %d, skip it.\n",
				       DP_UOID(dre->dre_oid),
				       DP_DTI(&dre->dre_xid), rc);
				/* Fall through. */
			case DTX_ST_COMMITTED:
				d_list_del(&dre->dre_link);
				drh->drh_count--;
				D_FREE_PTR(dre);
				continue;
			}
		}

		oc_attr = daos_oclass_attr_find(dre->dre_oid.id_pub);
		D_ASSERT(oc_attr->ca_resil == DAOS_RES_REPL);

		rc = dtx_check_committable(dra->po_uuid, co_uuid, &dre->dre_xid,
					   &dre->dre_oid, layout);
		if (rc == DTX_ST_INIT) {
			rc = dtx_resync_abort(dra->po_uuid, cont,
					      drh, dre, dra->version, false);
			if (rc > 0)
				goto committable;
			if (rc < 0)
				err = rc;
			continue;
		}

		if (rc != DTX_ST_COMMITTED && rc != DTX_ST_PREPARED) {
			/* We are not sure about whether the DTX can be
			 * committed or not, then we have to skip it.
			 */
			D_WARN("Not sure about whether the DTX "DF_UOID
			       "/"DF_DTI" can be committed or not: rc = %d\n",
			       DP_UOID(dre->dre_oid),
			       DP_DTI(&dre->dre_xid), rc);

			d_list_del(&dre->dre_link);
			drh->drh_count--;
			D_FREE_PTR(dre);
			continue;
		}

committable:
		if (++count >= DTX_THRESHOLD_COUNT) {
			rc = dtx_resync_commit(dra->po_uuid, co_uuid,
					       drh, count, dra->version);
			if (rc < 0)
				err = rc;
			count = 0;
		}
	}

	if (count > 0) {
		rc = dtx_resync_commit(dra->po_uuid, co_uuid, drh, count,
				       dra->version);
		if (rc < 0)
			err = rc;
	}

	if (layout != NULL)
		pl_obj_layout_free(layout);

	if (cont != NULL)
		ds_cont_put(cont);

	return err;
}

static int
dtx_placement_check(uuid_t co_uuid, vos_iter_entry_t *ent, void *args)
{
	struct dtx_resync_args		*dra = args;
	struct dtx_resync_entry		*dre;

	/* Ignore new DTX after the rebuild/recovery start */
	if (ent->ie_dtx_sec > dra->start)
		return 0;

	/* We commit the DTXs periodically, there will be not too many DTXs
	 * to be checked when resync. So we can load all those uncommitted
	 * DTXs in RAM firstly, then check the state one by one. That avoid
	 * the race trouble between iteration of active-DTX tree and commit
	 * (or abort) the DTXs (that will change the active-DTX tree).
	 */

	D_ALLOC_PTR(dre);
	if (dre == NULL)
		return -DER_NOMEM;

	dre->dre_xid = ent->ie_xid;
	dre->dre_oid = ent->ie_oid;
	dre->dre_state = ent->ie_dtx_state;
	dre->dre_intent = ent->ie_dtx_intent;
	dre->dre_hash = ent->ie_dtx_hash;
	d_list_add_tail(&dre->dre_link, &dra->tables.drh_list);
	dra->tables.drh_count++;

	return 0;
}

int
dtx_resync(daos_handle_t po_hdl, uuid_t po_uuid, uuid_t co_uuid, uint32_t ver)
{
	struct dtx_resync_args		 dra = { 0 };
	struct dtx_resync_entry		*dre;
	struct dtx_resync_entry		*next;
	int				 rc = 0;
	int				 rc1 = 0;

	uuid_copy(dra.po_uuid, po_uuid);
	dra.start = time(NULL);
	dra.version = ver;
	D_INIT_LIST_HEAD(&dra.tables.drh_list);
	dra.tables.drh_count = 0;

	D_DEBUG(DB_TRACE, "resync DTX scan "DF_UUID" start.\n",
		DP_UUID(po_uuid));

	rc = ds_cont_iter(po_hdl, po_uuid, co_uuid, dtx_placement_check, &dra,
			  VOS_ITER_DTX);
	/* Handle the DTXs that have been scanned even if some failure happend
	 * in above ds_cont_iter() step.
	 */
	rc1 = dtx_placement_handle(&dra, co_uuid);
	d_list_for_each_entry_safe(dre, next, &dra.tables.drh_list, dre_link) {
		d_list_del(&dre->dre_link);
		D_FREE_PTR(dre);
	}

	return rc < 0 ? rc : (rc1 < 0 ? rc1 : 0);
}
