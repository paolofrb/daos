#!/usr/bin/python
'''
  (C) Copyright 2018-2019 Intel Corporation.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
  The Governments rights to use, modify, reproduce, release, perform, display,
  or disclose this software are subject to the terms of the Apache License as
  provided in Contract No. B609815.
  Any reproduction of computer software, computer software documentation, or
  portions thereof marked with this legend must also reproduce the markings.
'''

import os
import time
import traceback
import random
import string
from apricot import TestWithoutServers


import ServerUtils
import WriteHostFile
import CheckForPool
from daos_api import DaosPool, DaosServer, DaosContainer
from daos_api import DaosApiError

class RebuildTests(TestWithoutServers):

    """
    Test Class Description:
    This class contains tests for pool rebuild.
    :avocado: recursive
    """

    def test_simple_rebuild(self):
        """
        Test ID: Rebuild-001

        Test Description: The most basic rebuild test.

        Use Cases:
          -- single pool rebuild, single client, various reord/object
             counts

        :avocado: tags=pool,rebuild,rebuildsimple
        """

        # the rebuild tests need to redo this stuff each time so not in setup
        # as it usually would be
        setid = self.params.get("setname", '/run/testparams/setnames/')
        server_group = self.params.get("server_group",
                                       '/server/',
                                       'daos_server')

        basepath = os.path.normpath(self.build_paths['PREFIX']  + "/../")
        tmp = self.build_paths['PREFIX'] + '/tmp'

        self.hostlist = self.params.get("test_machines", '/run/hosts/')
        hostfile = WriteHostFile.WriteHostFile(self.hostlist, tmp)

        try:
            ServerUtils.runServer(hostfile, server_group, basepath)

            # use the uid/gid of the user running the test, these should
            # be perfectly valid
            createuid = os.geteuid()
            creategid = os.getegid()

            # parameters used in pool create that are in yaml
            createmode = self.params.get("mode", '/run/testparams/createmode/')
            createsetid = self.params.get("setname",
                                          '/run/testparams/createset/')
            createsize = self.params.get("size", '/run/testparams/createsize/')

            # initialize a python pool object then create the underlying
            # daos storage
            pool = DaosPool(self.context)
            pool.create(createmode, createuid, creategid,
                        createsize, createsetid, None)

            # want an open connection during rebuild
            pool.connect(1 << 1)

            # get pool status we want to test later
            pool.pool_query()
            if pool.pool_info.pi_ndisabled != 0:
                self.fail("Number of disabled targets reporting incorrectly.\n")
            if pool.pool_info.pi_rebuild_st.rs_errno != 0:
                self.fail("Rebuild error but rebuild hasn't run.\n")
            if pool.pool_info.pi_rebuild_st.rs_done != 1:
                self.fail("Rebuild is running but device hasn't failed yet.\n")
            if pool.pool_info.pi_rebuild_st.rs_obj_nr != 0:
                self.fail("Rebuilt objs not zero.\n")
            if pool.pool_info.pi_rebuild_st.rs_rec_nr != 0:
                self.fail("Rebuilt recs not zero.\n")
            pool_version = pool.pool_info.pi_rebuild_st.rs_version

            # create a container
            container = DaosContainer(self.context)
            container.create(pool.handle)

            # now open it
            container.open()

            # how many objects and records are we creating
            objcount = self.params.get("objcount",
                                       '/run/testparams/numobjects/*')
            reccount = self.params.get("reccount",
                                       '/run/testparams/numrecords/*')
            if objcount == 0:
                reccount = 0

            # which rank to write to and kill
            rank = self.params.get("rank", '/run/testparams/ranks/*')

            # how much data to write with each key
            size = self.params.get("size", '/run/testparams/datasize/')

            saved_data = []
            for i in range(0, objcount):
                obj = None
                for j in range(0, reccount):

                    # make some stuff up and write
                    dkey = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(5))
                    akey = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(5))
                    data = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(size))

                    obj, tx = container.write_an_obj(data, len(data),
                                                        dkey, akey, obj, rank)

                    saved_data.append((obj, dkey, akey, data, tx))

                    # read the data back and make sure its correct
                    data2 = container.read_an_obj(size, dkey, akey,
                                                  obj, tx)
                    if data != data2.value:
                        self.fail("Write data 1, read it back, didn't match\n")

            # kill a server that has
            server = DaosServer(self.context, server_group, rank)
            server.kill(1)

            # temporarily, the exclude of a failed target must be done
            # manually
            pool.exclude([rank])

            while True:
                # get the pool/rebuild status again
                pool.pool_query()
                if pool.pool_info.pi_rebuild_st.rs_done == 1:
                    break
                else:
                    time.sleep(2)

            if pool.pool_info.pi_ndisabled != 1:
                self.fail("Number of disabled targets reporting incorrectly: {}"
                          .format(pool.pool_info.pi_ndisabled))
            if pool.pool_info.pi_rebuild_st.rs_errno != 0:
                self.fail("Rebuild error reported: {}".format(
                    pool.pool_info.pi_rebuild_st.rs_errno))
            if pool.pool_info.pi_rebuild_st.rs_obj_nr != objcount:
                self.fail("Rebuilt objs not as expected: {0} {1}"
                          .format(pool.pool_info.pi_rebuild_st.rs_obj_nr,
                                  objcount))
            if pool.pool_info.pi_rebuild_st.rs_rec_nr != (reccount*objcount):
                self.fail("Rebuilt recs not as expected: {0} {1}"
                          .format(pool.pool_info.pi_rebuild_st.rs_rec_nr,
                                  reccount*objcount))

            # now that the rebuild finished verify the records are correct
            for tup in saved_data:
                data2 = container.read_an_obj(len(tup[3]), tup[1], tup[2],
                                              tup[0], tup[4])
                if tup[3] != data2.value:
                    self.fail("after rebuild data didn't check out")

        except DaosApiError as e:
            print (e)
            print (traceback.format_exc())
            self.fail("Expecting to pass but test has failed.\n")

        finally:
            try:
                ServerUtils.stopServer(hosts=self.hostlist)
                os.remove(hostfile)
                # really make sure everything is gone
                CheckForPool.CleanupPools(self.hostlist)
            finally:
                ServerUtils.killServer(self.hostlist)


    def test_multipool_rebuild(self):
        """
        Test ID: Rebuild-002
        Test Description: Expand on the basic test by rebuilding 2
        pools at once.

        Use Cases:
          -- multipool rebuild, single client, various object and record counds

        :avocado: tags=pool,rebuild,rebuildmulti
        """

        # the rebuild tests need to redo this stuff each time so not in setup
        # as it usually would be
        setid = self.params.get("setname", '/run/testparams/setnames/')
        server_group = self.params.get("server_group",
                                       '/server/',
                                       'daos_server')

        basepath = os.path.normpath(self.build_paths['PREFIX']  + "/../")
        tmp = self.build_paths['PREFIX'] + '/tmp'

        self.hostlist = self.params.get("test_machines", '/run/hosts/')
        hostfile = WriteHostFile.WriteHostFile(self.hostlist, tmp)

        try:
            ServerUtils.runServer(hostfile, server_group, basepath)

            # use the uid/gid of the user running the test, these should
            # be perfectly valid
            createuid = os.geteuid()
            creategid = os.getegid()

            # parameters used in pool create that are in yaml
            createmode = self.params.get("mode", '/run/testparams/createmode/')
            createsetid = self.params.get("setname",
                                          '/run/testparams/createset/')
            createsize = self.params.get("size", '/run/testparams/createsize/')

            # initialize python pool object then create the underlying
            # daos storage, the way the code is now the pools should be
            # on the same storage and have the same service leader
            pool1 = DaosPool(self.context)
            pool2 = DaosPool(self.context)
            pool1.create(createmode, createuid, creategid,
                         createsize, createsetid, None)
            pool2.create(createmode, createuid, creategid,
                         createsize, createsetid, None)

            # want an open connection during rebuild
            pool1.connect(1 << 1)
            pool2.connect(1 << 1)

            # create containers
            container1 = DaosContainer(self.context)
            container1.create(pool1.handle)
            container2 = DaosContainer(self.context)
            container2.create(pool2.handle)

            # now open them
            container1.open()
            container2.open()

            # how many objects and records are we creating
            objcount = self.params.get("objcount",
                                       '/run/testparams/numobjects/*')
            reccount = self.params.get("reccount",
                                       '/run/testparams/numrecords/*')
            if objcount == 0:
                reccount = 0

            # which rank to write to and kill
            rank = self.params.get("rank", '/run/testparams/ranks/*')

            # how much data to write with each key
            size = self.params.get("size", '/run/testparams/datasize/')

            # Putting the same data in both pools, at least for now to simplify
            # checking its correct
            saved_data = []
            for i in range(0, objcount):
                obj = None
                for j in range(0, reccount):

                    # make some stuff up and write
                    dkey = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(5))
                    akey = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(5))
                    data = ''.join(random.choice(string.ascii_uppercase +
                                                 string.digits)
                                   for _ in range(size))

                    obj, tx = container1.write_an_obj(data, len(data),
                                                         dkey, akey, obj, rank)
                    obj, tx = container2.write_an_obj(data, len(data),
                                                         dkey, akey, obj, rank)

                    saved_data.append((obj, dkey, akey, data, tx))

                    # read the data back and make sure its correct
                    # containers
                    data2 = container1.read_an_obj(size, dkey, akey,
                                                   obj, tx)
                    if data != data2.value:
                        self.fail("Wrote data P1, read it back, didn't match\n")

                    # containers
                    data2 = container2.read_an_obj(size, dkey, akey,
                                                   obj, tx)
                    if data != data2.value:
                        self.fail("Wrote data P2, read it back, didn't match\n")

            # kill a server
            server = DaosServer(self.context, server_group, rank)
            server.kill(1)

            # temporarily, the exclude of a failed target must be done
            # manually
            pool1.exclude([rank])
            pool2.exclude([rank])

            # check that rebuild finishes, no errors, progress data as
            # know it to be.  Check pool 1 first then we'll check 2 below.
            while True:
                pool1.pool_query()
                if pool1.pool_info.pi_rebuild_st.rs_done == 1:
                    break
                else:
                    time.sleep(2)

            # check there are no errors and other data matches what we
            # apriori know to be true,
            if pool1.pool_info.pi_ndisabled != 1:
                self.fail("P1 number disabled targets reporting incorrectly: {}"
                          .format(pool1.pool_info.pi_ndisabled))
            if pool1.pool_info.pi_rebuild_st.rs_errno != 0:
                self.fail("P1 rebuild error reported: {}"
                          .format(pool1.pool_info.pi_rebuild_st.rs_errno))
            if pool1.pool_info.pi_rebuild_st.rs_obj_nr != objcount:
                self.fail("P1 rebuilt objs not as expected: {0} {1}"
                          .format(pool1.pool_info.pi_rebuild_st.rs_obj_nr,
                                  objcount))
            if pool1.pool_info.pi_rebuild_st.rs_rec_nr != (reccount*objcount):
                self.fail("P1 rebuilt recs not as expected: {0} {1}"
                          .format(pool1.pool_info.pi_rebuild_st.rs_rec_nr,
                                  reccount*objcount))

            # now that the rebuild finished verify the records are correct
            for tup in saved_data:
                data2 = container1.read_an_obj(len(tup[3]), tup[1], tup[2],
                                               tup[0], tup[4])
                if tup[3] != data2.value:
                    self.fail("after rebuild data didn't check out")

            # now check the other pool
            while True:
                pool2.pool_query()
                if pool2.pool_info.pi_rebuild_st.rs_done == 1:
                    break
                else:
                    time.sleep(2)

            # check there are no errors and other data matches what we
            # apriori know to be true
            if pool2.pool_info.pi_ndisabled != 1:
                self.fail("Number disabled targets reporting incorrectly: {}"
                          .format(pool2.pool_info.pi_ndisabled))
            if pool2.pool_info.pi_rebuild_st.rs_errno != 0:
                self.fail("Rebuild error reported: {}"
                          .format(pool2.pool_info.pi_rebuild_st.rs_errno))
            if pool2.pool_info.pi_rebuild_st.rs_obj_nr != objcount:
                self.fail("Rebuilt objs not as expected: {0} {1}"
                          .format(pool2.pool_info.pi_rebuild_st.rs_obj_nr,
                                  objcount))
            if pool2.pool_info.pi_rebuild_st.rs_rec_nr != (reccount*objcount):
                self.fail("Rebuilt recs not as expected: {0} {1}".
                          format(pool2.pool_info.pi_rebuild_st.rs_rec_nr,
                                 (reccount*objcount)))

            # now that the rebuild finished verify the records are correct
            for tup in saved_data:
                data2 = container2.read_an_obj(len(tup[3]), tup[1], tup[2],
                                               tup[0], tup[4])
                if tup[3] != data2.value:
                    self.fail("after rebuild data didn't check out")

        except DaosApiError as e:
            print (e)
            print (traceback.format_exc())
            self.fail("Expecting to pass but test has failed.\n")

        finally:
            ServerUtils.stopServer(hosts=self.hostlist)
            os.remove(hostfile)
            CheckForPool.CleanupPools(self.hostlist)
            ServerUtils.killServer(self.hostlist)
