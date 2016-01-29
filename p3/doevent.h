#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#include <asm/atomic.h>

#define DE_EMPTY 	0
#define DE_OCCUPIED	1

struct doevent {
	int			eid;
	int			count;
	spinlock_t		lock;		/* Lock for RW doevent */
	wait_queue_head_t	wait_queue;	/* Waiting process list */
	struct list_head	des;		/* Doevent list */
};

struct doevent_list {
	int		count;		/* Number of created doevent. */
	struct doevent	* head;		/* First doevent. */
	spinlock_t	lock;		/* write for RW de_list */
};

extern struct doevent_list	de_list;	/* List of doevent. */

#define __DOEVENT_INITIALIZER(name)						\
{										\
	.eid		= 0,							\
	.count		= DE_EMPTY,						\
	.lock 		= __SPIN_LOCK_UNLOCKED((name)->lock),			\
	.wait_queue 	= __WAIT_QUEUE_HEAD_INITIALIZER((name)->wait_queue), 	\
	.des		= {(&(name)->des), &((name)->des)},						\
}

#define next_de(de) \
	list_entry(rcu_deference((de)->doevents.next), struct doevent, doevents)

#define for_each_doevent(de) \
	for (de = init_de; (de = next_de(de)) != init_de)

void __init doevent_init();


//int 	sys_doeventopen();
//int 	sys_doeventclose(int eid);
//int	sys_doeventwait(int eid);
//int	sys_doeventsig(int eid);
