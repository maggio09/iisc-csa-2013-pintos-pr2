/*****************************************************************************
Created by: Duke
Purpose: Jo dar gaya woh so gaya
Singing lulaby for sleeping threads
*****************************************************************************/
#include "threads/sleep.h"
#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "devices/timer.h"

#define SLEEP_MAGIC 0x2366611 //lulaby verification
static struct list bed_list;
static int64_t last_read_tick;

static bool is_bed (struct bed *bednoptr);
static void empty_bed (struct bed *bednoptr);
bool compare_wakeup (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
void sleepers (void);

void sleepers (void)
{
	struct list_elem *cur;
	struct bed *b;
	for (cur = list_begin (&bed_list); cur != list_end (&bed_list); cur = list_next (cur))
	{
		b = list_entry (cur, struct bed, elem);
		printf("%u\t%d\n", b->thread->tid, b->ticks);
	}
	printf ("\n");
}

bool
compare_wakeup (const struct list_elem *a, const struct list_elem *b, void *aux UNUSED)
{
	struct bed *abed, *bbed;

	ASSERT (a != NULL && b != NULL);

	abed = list_entry (a, struct bed, elem);
	bbed = list_entry (b, struct bed, elem);

	return abed->ticks < bbed->ticks;
}

/* Initialized at begining */
void sleep_list_init(void)
{
	list_init (&bed_list); //beds
	last_read_tick = timer_ticks(); //last roll call
	ASSERT (list_empty (&bed_list)); //no one sleeps when system starts
}

/* Make the thread sleep by getting bed*/
void sleep_insert(int64_t ticks)
{
	struct thread *threadptr;
	struct bed *bednoptr;
	enum intr_level old;
	//No bed for no sleep time
	if (ticks == 0)
	{
		return;
	}

	//Get thread info
	threadptr = thread_current();

	//Thread should be running
	ASSERT (threadptr->status == THREAD_RUNNING);

	bednoptr = &threadptr->bednoptr;
	bednoptr->thread = threadptr;
	bednoptr->ticks = ticks + timer_ticks (); //Wake up time based on system tick
	bednoptr->magic = SLEEP_MAGIC;

	//Critical Section
	
	list_insert_ordered (&bed_list, &bednoptr->elem, compare_wakeup, NULL);
	//sleepers ();
	old = intr_disable ();
	thread_block ();
	intr_set_level(old);
}


/**/


/* Is it bed */
static bool is_bed (struct bed *bednoptr)
{
	return (bednoptr != NULL && bednoptr->magic == SLEEP_MAGIC);
}


/* Not sleeping anymore */
static void empty_bed (struct bed *bednoptr)
{
	enum intr_level old;

	//check for true bed
	ASSERT (is_bed (bednoptr));

	//Taking away the bed
	old = intr_disable();
	list_remove (&bednoptr->elem);
	thread_unblock (bednoptr->thread);
	intr_set_level (old);
}


void sleep_wakeup (void)
{
	struct list_elem *cur, *next;
	struct bed *bednoptr;

	//Return if no beds
	if (list_empty (&bed_list))
	{
		return;
	}

	//Wake up processes whose sleep time is over
	//No beds
	while (!list_empty (&bed_list))
	{
		cur = list_begin (&bed_list);
		bednoptr = list_entry (cur, struct bed, elem);
		if (bednoptr->ticks > timer_ticks ())
		{
			break;
		}
		list_remove (cur);
		empty_bed (bednoptr);
	}
}