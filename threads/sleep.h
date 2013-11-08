/*****************************************************************************
Created by: Duke
Purpose: Jo dar gaya woh so gaya
Singing lulaby for sleeping threads
*****************************************************************************/
#ifndef THREADS_SLEEP_H
#define THREADS_SLEEP_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

//Structure for bed for sleeping threads
struct bed
{
	int64_t ticks;//sleep time
	struct thread *thread;
	struct list_elem elem;
	unsigned magic;
};

//externally called commands
void sleep_list_init (void);
void sleep_insert (int64_t);
void sleep_wakeup (void);

//

#endif