//
// (C) Copyright 2019 Intel Corporation.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// GOVERNMENT LICENSE RIGHTS-OPEN SOURCE SOFTWARE
// The Government's rights to use, modify, reproduce, release, perform, display,
// or disclose this software are subject to the terms of the Apache License as
// provided in Contract No. 8F-30005.
// Any reproduction of computer software, computer software documentation, or
// portions thereof marked with this legend must also reproduce the markings.
//

package main

import (
	"fmt"
	"os"

	"github.com/pkg/errors"
)

// ShowStorageCommand is the struct representing the command to list storage.
type ShowStorageCommand struct{}

// Execute is run when ShowStorageCommand activates
func (s *ShowStorageCommand) Execute(args []string) error {
	if err := connectHosts(); err != nil {
		return errors.Wrap(err, "unable to connect to hosts")
	}

	fmt.Printf(
		checkAndFormat(conns.ListNvme()),
		"NVMe SSD controller and constituent namespace")

	fmt.Printf(checkAndFormat(conns.ListScm()), "SCM module")

	// exit immediately to avoid continuation of main
	os.Exit(0)
	// never reached
	return nil
}

// KillRankCommand is the struct representing the command to kill server
// identified by rank on given pool identified by uuid.
type KillRankCommand struct {
	Rank     uint32 `short:"r" long:"rank" description:"Rank identifying DAOS server"`
	PoolUUID string `short:"p" long:"pool-uuid" description:"Pool uuid that rank relates to"`
}

// Execute is run when KillRankCommand activates
func (k *KillRankCommand) Execute(args []string) error {
	if err := connectHosts(); err != nil {
		return errors.WithMessage(err, "unable to connect to hosts")
	}

	if err := conns.KillRank(k.PoolUUID, k.Rank); err != nil {
		return errors.WithMessage(err, "Kill Rank failed")
	}

	fmt.Println("Kill Rank succeeding on all active connections!")

	// exit immediately to avoid continuation of main
	os.Exit(0)
	// never reached
	return nil
}
