/**
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
 * The Government's rights to use, modify, reproduce, release, perform, display,
 * or disclose this software are subject to the terms of the LGPL License as
 * provided in Contract No. B609815.
 * Any reproduction of computer software, computer software documentation, or
 * portions thereof marked with this legend must also reproduce the markings.
 *
 * (C) Copyright 2016 Intel Corporation.
 */
/**
 * This file is part of dmg, a simple test case of dmg.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <daos_mgmt.h>
#include <daos_event.h>

typedef struct {
	daos_rank_t		ranks[8];
	daos_rank_list_t	svc;
	daos_handle_t		eq;
	bool			async;
} test_arg_t;

/** create/destroy pool on all tgts */
static void
pool_create_all(void **state)
{
	test_arg_t	*arg = *state;
	uuid_t		 uuid;
	char		 uuid_str[64];
	daos_event_t	 ev;
	daos_event_t	*evp;
	int		 rc;

	if (arg->async) {
		rc = daos_event_init(&ev, arg->eq, NULL);
		assert_int_equal(rc, 0);
	}

	arg->svc.rl_nr.num_out = 0;

	/** create container */
	print_message("creating pool %ssynchronously ... ",
		      arg->async ? "a" : "");
	rc = dmg_pool_create(0 /* mode */, 0 /* uid */, 0 /* gid */,
			     "srv_grp" /* grp */, NULL /* tgts */,
			     "pmem" /* dev */, 0 /* minimal size */,
			     &arg->svc /* svc */, uuid,
			     arg->async ? &ev : NULL);
	assert_int_equal(rc, 0);

	if (arg->async) {
		/** wait for container creation */
		rc = daos_eq_poll(arg->eq, 1, DAOS_EQ_WAIT, 1, &evp);
		assert_int_equal(rc, 1);
		assert_ptr_equal(evp, &ev);
		assert_int_equal(ev.ev_error, 0);
	}

	uuid_unparse_lower(uuid, uuid_str);
	print_message("success uuid = %s\n", uuid_str);

	/** destroy container */
	print_message("destroying pool %ssynchronously ... ",
		      arg->async ? "a" : "");
	rc = dmg_pool_destroy(uuid, "srv_grp", 1, arg->async ? &ev : NULL);
	assert_int_equal(rc, 0);

	if (arg->async) {
		/** for container destroy */
		rc = daos_eq_poll(arg->eq, 1, DAOS_EQ_WAIT, 1, &evp);
		assert_int_equal(rc, 1);
		assert_ptr_equal(evp, &ev);
		assert_int_equal(ev.ev_error, 0);

		rc = daos_event_fini(&ev);
		assert_int_equal(rc, 0);
	}
	print_message("success\n");
}

static int
async_enable(void **state) {
	test_arg_t	*arg = *state;

	arg->async = true;
	return 0;
}

static int
async_disable(void **state) {
	test_arg_t	*arg = *state;

	arg->async = false;
	return 0;
}

static const struct CMUnitTest tests[] = {
	{ "DMG1: create/destroy pool on all tgts",
	  pool_create_all, async_disable, NULL},
	{ "DMG2: create/destroy pool on all tgts (async)",
	  pool_create_all, async_enable, NULL},
};

static int
setup(void **state) {
	test_arg_t	*arg;
	int		 rc;

	arg = malloc(sizeof(test_arg_t));
	if (arg == NULL)
		return -1;

	rc = dmg_init();
	if (rc)
		return rc;

	rc = daos_eq_create(&arg->eq);
	if (rc)
		return rc;

	arg->svc.rl_nr.num = 8;
	arg->svc.rl_nr.num_out = 0;
	arg->svc.rl_ranks = arg->ranks;

	*state = arg;
	return 0;
}

static int
teardown(void **state)
{
	test_arg_t	*arg = *state;
	int		 rc;

	rc = daos_eq_destroy(arg->eq, 0);
	if (rc)
		return rc;

	rc = dmg_fini();
	if (rc)
		return rc;

	free(arg);
	return 0;
}

int
main(int argc, char **argv)
{
	int rc;

	rc = cmocka_run_group_tests_name("DMG pool tests", tests,
					 setup, teardown);

	if (rc > 0)
		/** some test failed, report failure */
		return -1;

	if (rc < 0)
		return rc;

	return 0;
}