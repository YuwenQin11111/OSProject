/*
 *	kernel/sched.gwrr.c
 *
 *	Group Weighted Round Robin (GWRR) Scheduling Class (SCHED_GWRR)
 * 
 *	2012-11-05	CSC456 - Program Assignment # 4 by Yuwen Qin,
 *				Haoliang Wang and Nasrin Mostafazadeh.
 */

/*
 * Update the cpu time and calculate the usage of task specified by rq->curr.
 */
static void update_curr_gwrr(struct rq *rq)
{
	struct task_struct* curr = rq->curr;
	u64 delta_exec;

	if (unlikely(!curr))
		return;
	if (unlikely(!GWRR_TSK(curr)))
		return;
	
	delta_exec = rq -> clock - curr -> se.exec_start;

	if (unlikely((s64)delta_exec < 0) )
		delta_exec = 0;

	curr->se.exec_start = rq->clock;
	curr->se.sum_exec_runtime += delta_exec;
	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));
	cpuacct_charge(curr, delta_exec);
}

/*
 * Add a GWRR task into the running queue.
 */
static void enqueue_gwrr(struct rq *rq, struct task_struct *p, int wakeup)
{
	spin_lock(&grp_lock);

	update_curr_gwrr(rq);
	list_add_tail(&(p->running_queue),&(rq->gwrr_queue));

	spin_unlock(&grp_lock);
}

/*
 * Dequeue the task from rq. If the task's running time exceeds its timeslice
 * move the task to the tail of the group list.
 */
static void dequeue_gwrr(struct rq *rq, struct task_struct* p,  int sleep)
{
	grp_t* tmp_grp=NULL;

	spin_lock(&grp_lock);

	update_curr_gwrr(rq);
	list_for_each_entry(tmp_grp, &group_list, grp_list)
	{
		if (tmp_grp->gid==p->egid)
			break;
	}
	if (tmp_grp->gid==p->egid)
	{
		tmp_grp->running_time = tmp_grp->running_time+p->running_time;
		if (tmp_grp->running_time >= get_weight(p->egid) * DEF_TIMESLICE/10)
		{
			list_move_tail(&(tmp_grp->grp_list), &group_list);
			tmp_grp->running_time = 0;
		}
	}
	p->running_time = 0;
	list_del(&(p->running_queue));

	spin_unlock(&grp_lock);
}

static void requeue_gwrr(struct rq *rq, struct task_struct* p)
{
	dequeue_gwrr(rq,p,0);
	enqueue_gwrr(rq,p,0);
}

static void yield_task_gwrr(struct rq *rq)
{
	requeue_gwrr(rq, rq->curr);
}

static void check_preempt_curr_gwrr(struct rq *rq, struct task_struct *p)
{
	if (p->prio < rq->curr->prio)
		resched_task(rq->curr);
}

/*
 * Pick the group in the head of group_list and pick its task from the running_queue.
 */
static struct task_struct* get_next_gwrr(struct rq *rq)
{
	grp_t* tmp_grp=NULL;
	struct task_struct* tmp_tsk=NULL;
	struct task_struct* next_tsk=NULL;
	struct list_head* gwrr = &(rq->gwrr_queue);

	if (list_empty(gwrr))
		return NULL;

	spin_lock(&grp_lock);

	list_for_each_entry(tmp_grp, &group_list, grp_list)
	{
		list_for_each_entry(tmp_tsk, gwrr, running_queue)
		{
			if ((next_tsk==NULL) && (tmp_tsk->egid == tmp_grp->gid))
			{
				next_tsk = tmp_tsk;
				break;
			}
		}
	}

	spin_unlock(&grp_lock);

	return next_tsk;
}

static struct task_struct* pick_next_task_gwrr(struct rq *rq)
{
	struct task_struct* next_tsk=NULL;

	next_tsk = get_next_gwrr(rq);
	if (next_tsk!=NULL)
	{
		if (GWRR_TSK(next_tsk))
		{
			next_tsk->se.exec_start = rq->clock;
			return next_tsk;
		}
	}
	return NULL;
}

static void put_prev_task_gwrr(struct rq *rq, struct task_struct *p)
{
	update_curr_gwrr(rq);
	p->se.exec_start = 0;
	
}

static void set_curr_task_gwrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock;
}

/*
 * This function is called when the timeslice is end.
 */
static void task_tick_gwrr(struct rq *rq, struct task_struct *p, int queued)
{
	update_curr_gwrr(rq);
	p->running_time++;
	if (!GWRR_TSK(p))
		return;

	if (--p->rt.time_slice)
		return;

	p->rt.time_slice = DEF_TIMESLICE;

	if (p->running_queue.prev != p->running_queue.next) {
		requeue_gwrr(rq, p);
		set_tsk_need_resched(p);
	}
}

static void prio_changed_gwrr(struct rq *rq, struct task_struct *p,
			    int oldprio, int running)
{
	if (running)
	{
		if (oldprio < p->prio)
			resched_task(p);
	} 
	else
		check_preempt_curr_gwrr(rq, p);
}

static void switched_to_gwrr(struct rq *rq, struct task_struct *p,
			     int running)
{
	if (running)
		resched_task(rq->curr);
	else
		check_preempt_curr(rq, p);
}

static const struct sched_class gwrr_sched_class = {
	.next				= 	&idle_sched_class,
	.enqueue_task		= 	enqueue_gwrr,
	.dequeue_task		= 	dequeue_gwrr,
	.yield_task			= 	yield_task_gwrr,
	.check_preempt_curr	= 	check_preempt_curr_gwrr,
	.pick_next_task		= 	pick_next_task_gwrr,
	.put_prev_task		= 	put_prev_task_gwrr,
	.set_curr_task		= 	set_curr_task_gwrr,
	.task_tick			= 	task_tick_gwrr,
	.prio_changed		= 	prio_changed_gwrr,
	.switched_to		= 	switched_to_gwrr,
};
