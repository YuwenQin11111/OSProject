#include <linux/doevent.h>
#include <linux/kernel.h>
#include <linux/sched.h>

struct doevent_list	de_list;

void __init doevent_init() {
        de_list.count = 0; 
        de_list.head  = NULL;
	//__SPIN_LOCK_UNLOCKED(de_list.lock);
	spin_lock_init(&de_list.lock);
}


asmlinkage int sys_doeventopen()
{
	//struct doevent_list	de_list;
	struct doevent * de = kmalloc(sizeof(struct doevent), GFP_KERNEL);
	int		rval;

	if (de == NULL) {
		printk("sys_doeventopen: error to malloc!\n");
		return(-1);
	}

	*de = (struct doevent) __DOEVENT_INITIALIZER(de);
	spin_lock_init(&de->lock);

	/* get RW lock of de_list */
	spin_lock_irq(&de_list.lock);	

	/* critical section */
	de_list.count ++;
	de->eid = de_list.count;
	de->count = DE_OCCUPIED;

	/* add de to de_list */
	if (de_list.head == NULL)
		de_list.head = de;
	else
		list_add_tail(&de->des, &(de_list.head->des));

	rval = de->eid;			/* we don't want to read de when return */

	/* release de_list lock */
	spin_unlock_irq(&de_list.lock);

	return(rval);
}

asmlinkage int sys_doeventclose(int eid)
{
	struct doevent	* de = NULL;
	int		rval;	

	spin_lock_irq(&de_list.lock);				/* get lock */
	if (de_list.head == NULL)				/* make sure there is de in de_list */
		return(-1);

	list_for_each_entry(de, &(de_list.head->des), des) {
		if (de!= NULL && de->eid == eid)
			break;
	}

	if (de == NULL || de->eid != eid) {			/* test search result */
		printk("ERROR: invalid eid %d\n", eid);
		return(-1);
	}
	spin_unlock_irq(&de_list.lock);
	
	spin_lock_irq(&de->lock);				/* will RW on de, first get lock */
	wake_up_all(&de->wait_queue);				/* wait waiting processes */
	rval = de->count;
	spin_unlock_irq(&de->lock);				/* release lock */

	/* change de_list.head */
	spin_lock_irq(&de_list.lock);				/* reget lock of de_list */
	if (de->des.next == &de->des)
		de_list.head = NULL;
	else if (de_list.head == de)
		de_list.head = list_entry(de->des.next, struct doevent, des);
	list_del(&de->des);
	kfree(de);						/* free memory of de */
	spin_unlock_irq(&de_list.lock);				/* release lock of de_list */

	return(rval-1);
}

asmlinkage int sys_doeventwait(int eid)
{
	struct doevent	* de = NULL;

	spin_lock_irq(&de_list.lock);				/* get lock of de_list */
	if (de_list.head == NULL)				/* make sure ther is de in de_list */
		return(-1);

	list_for_each_entry(de, &de_list.head->des, des) {
		if (de != NULL && de->eid == eid)
			break;
	}

	if (de == NULL || de->eid != eid) {
		printk("ERROR: invalid eid %d\n", eid);
		return(-1);
	}
	spin_unlock_irq(&de_list.lock);				/* release lock of de_list */
	
	printk("process %d try to get lock %d ...\n", current->pid, eid);

	spin_lock_irq(&de->lock);				/* get lock of de */
	if (de->count == DE_EMPTY)
		de->count = DE_OCCUPIED;
	else {
		DECLARE_WAITQUEUE(wait, current);		
		add_wait_queue_exclusive(&de->wait_queue, &wait);	/* add process to wait queue */
		current->state = TASK_UNINTERRUPTIBLE;
		de->count ++;
		spin_unlock_irq(&de->lock);				/* release lock of de */
		schedule();						/* yield */

		/* if come here, process is waken up */
		printk("process %d back and get lock!\n", current->pid);
		return(0);
	}
	printk("process %d get lock %d!\n", current->pid, eid);
		
	spin_unlock_irq(&de->lock);
	
	return(0);
}

asmlinkage int sys_doeventsig(int eid)
{
	struct doevent * de = NULL;

	spin_lock_irq(&de_list.lock);				/* get lock of de_list */
	if (de_list.head == NULL)				/* make sure there is de in de_list */
		return(-1);

	list_for_each_entry(de, &(de_list.head->des), des) {
		if (de != NULL && de->eid == eid)
			break;
	}

	if (de == NULL || de->eid != eid) {			/* check result */
		printk("ERROR: invalid eid %d\n", eid);
		return(-1);
	}
	spin_unlock_irq(&de_list.lock);				/* release lock of de_list */
	
	spin_lock_irq(&de->lock);				/* get lock of de */
	de->count = DE_OCCUPIED;				/* clear count of waiting process */
	wake_up_all(&de->wait_queue);				/* wake up all waiting process */
	//de->wait_queue = (struct wait_queue_head_t) __WAIT_QUEUE_HEAD_INITIALIZER((de)->wait_queue);
	de->wait_queue.task_list = (struct list_head) {&(de->wait_queue.task_list), \
						&(de->wait_queue.task_list)};
	spin_unlock_irq(&de->lock);				/* release lock of de */

	return(0);
}

