/*
 * ssd_bm_parallel.cpp
 *
 *  Created on: Apr 22, 2012
 *      Author: niv
 */

#include <new>
#include <assert.h>
#include <stdio.h>
#include <stdexcept>
#include <algorithm>
#include "../ssd.h"

using namespace ssd;

Block_manager_parallel::Block_manager_parallel(Ssd& ssd, FtlParent& ftl)
: Block_manager_parent(ssd, ftl)
{}

void Block_manager_parallel::register_write_outcome(Event const& event, enum status status) {
	Block_manager_parent::register_write_outcome(event, status);
}

void Block_manager_parallel::register_erase_outcome(Event const& event, enum status status) {
	assert(event.get_event_type() == ERASE);
	if (status == FAILURE) {
		return;
	}
	Block_manager_parent::register_erase_outcome(event, status);
	Address a = event.get_address();

	// if there is no free pointer for this block, set it to this one.
	if (!has_free_pages(free_block_pointers[a.package][a.die])) {
		free_block_pointers[a.package][a.die] = find_free_unused_block(a.package, a.die);
	}

	check_if_should_trigger_more_GC(event.get_current_time());
}

Address Block_manager_parallel::write(Event const& write) {
	Address result;
	bool can_write = Block_manager_parent::can_write(write);
	if (!can_write) {
		return result;
	}
	return get_free_block_pointer_with_shortest_IO_queue();
}
