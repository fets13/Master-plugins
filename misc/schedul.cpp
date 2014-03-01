/**
 * Ydle - v0.5 (http://www.ydle.fr)
 * Home automotion project lowcost and DIY, all questions and information on http://www.ydle.fr
 *
 * @package Master
 * @license CC by SA
 **/


#include "schedul.h"
#include <sched.h>
#include <stdio.h>


// ----------------------------------------------------------------------------
/**
 Routine: scheduler_realtime()
 Inputs:


 Outputs:

 switch to 'real time'
 */
// ----------------------------------------------------------------------------
void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &p) == -1) {
		perror("Failed to switch to realtime scheduler.");
	}
}

// ----------------------------------------------------------------------------
/**
 Routine: scheduler_standard()
 Inputs:


 Outputs:

 switch to normal
 */
// ----------------------------------------------------------------------------
void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &p) == -1) {
		perror("Failed to switch to normal scheduler.");
	}
}
