
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_data.h"
#include "hal_device.h"
#include "hal_tasks.h"

/**
 * @addtogroup hal__tasks
 * @{
 */

/** @internal Create a task_id_t for a given task index. */
#define TASK_FLAG(i)    ((task_id_t) FLAG(i))

#if DEBUG
WARNING_SAVE()
WARNING_DISABLE("missing-prototypes")
/** @internal Default task if not defined. This is to simplify debugging and development. */
void default_task(void) { __BKPT(0); }
WARNING_RESTORE()
#endif

#define XTASK(task,fn,doc)     TASK_ID_ ## task = FLAG(TASK_ ## task),
/** @internal Task flags. */
typedef enum e_task_id
{
    TASK_ID_NONE = 0,
    TASK_LIST
} task_id_t;
#undef XTASK

// Declare the task functions.
#define XTASK(task,fn,doc)     void fn (void) __attribute__ (( weak, alias ("default_task") ));
TASK_LIST
#undef XTASK

// Make the task list.
#define XTASK(task,fn,doc)     [TASK_ ## task] = fn,
/** @internal Task function table. */
static const task_fn_t task_fns[TASK_COUNT] =
{
    TASK_LIST
};
#undef XTASK

/** @internal Tasks waiting to execute. */
static volatile task_id_t queued_tasks  = TASK_ID_NONE;

/** @internal Tasks pending execution awaiting higher priority tasks. */
static volatile task_id_t pending_tasks = TASK_ID_NONE;

/** @internal Current running task. */
static volatile task_t    running_task  = TASK_UNDEFINED;

void tasks_queue(task_t task)
{
    task_id_t task_id = TASK_FLAG(task);
    queued_tasks = (task_id_t) (queued_tasks | task_id);
}

task_status_t tasks_status_get(task_t task)
{
    task_id_t task_id = TASK_FLAG(task);
    if (running_task == task)
    {
        return TASK_STATUS_RUNNING;
    }
    else if ((pending_tasks & task_id) != 0)
    {
        return TASK_STATUS_PENDING;
    }
    else if ((queued_tasks & task_id) != 0)
    {
        return TASK_STATUS_QUEUED;
    }
    else
    {
        return TASK_STATUS_IDLE;
    }
}

void tasks_run(void)
{
    FSP_CRITICAL_SECTION_DEFINE;

    // Task execution is a priority round-robin queue.
    while (1)
    {
        // Disable interrupts around modifying the task queue.
        FSP_CRITICAL_SECTION_ENTER;
        // Copy then clear the task queue list.
        pending_tasks = queued_tasks;
        queued_tasks = TASK_ID_NONE;
        // Restore interrupts.
        FSP_CRITICAL_SECTION_EXIT;

        task_id_t current_task = TASK_FLAG(0);

        // Loop through the tasks, executing as we go.
        for (int i = 0; i < TASK_COUNT; i++)
        {
            if ((pending_tasks & current_task) != 0)
            {
                // Clear this task in the queue list.
                pending_tasks &= ~current_task;

                // Run this task.
                running_task = (task_t) i;
                task_fns[i]();
                running_task = TASK_UNDEFINED;
            }

            current_task <<= 1;
            if ((queued_tasks & (current_task - 1)) != 0)
            {
                // A higher priority task has been queued or this tasks has been re-queued.
                // Exit to refresh the task list.
                // But first re-queue any lower priority tasks.
                FSP_CRITICAL_SECTION_ENTER;
                queued_tasks = (task_id_t) (queued_tasks | (pending_tasks & ~(current_task - 1)));
                FSP_CRITICAL_SECTION_EXIT;
                break;
            }
        }
    }
}

/** @} */
