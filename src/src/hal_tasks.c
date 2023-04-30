
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

/** @internal Create a flag. */
#define FLAG(i)         (1U << (i))
/** @internal Create a task_id_t for a given task index. */
#define TASK_FLAG(i)    ((task_id_t) FLAG(i))

#if DEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
// Default task if not defined. This is to simplify debugging and development.
void default_task(void) { __BKPT(0); }
#pragma GCC diagnostic pop
#endif

#define XTASK(task)     TASK_ID_ ## task = FLAG(TASK_ ## task),
typedef enum e_task_id
{
    TASK_ID_NONE = 0,
    TASK_LIST
} task_id_t;
#undef XTASK

// Declare the task functions.
#define XTASK(task)     void task_ ## task (void) __attribute__ ((weak, alias ("default_task")));
TASK_LIST
#undef XTASK

// Make the task list.
#define XTASK(task)     task_ ## task,
static const task_fn_t task_fns[TASK_COUNT] =
{
    TASK_LIST
};
#undef XTASK

static volatile task_id_t queued_tasks = TASK_ID_NONE;
static volatile task_t running_task = TASK_UNDEFINED;

void tasks_queue(task_t task)
{
    task_id_t task_id = TASK_FLAG(task);
    queued_tasks = (task_id_t) (queued_tasks | task_id);
}

bool tasks_is_queued(task_t task)
{
    task_id_t task_id = TASK_FLAG(task);
    return ((queued_tasks & task_id) != 0);
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
        task_id_t tasks_to_run = queued_tasks;
        queued_tasks = TASK_ID_NONE;
        // Restore interrupts.
        FSP_CRITICAL_SECTION_EXIT;

        task_id_t current_task = TASK_FLAG(0);

        // Loop through the tasks, executing as we go.
        for (int i = 0; i < TASK_COUNT; i++)
        {
            if ((tasks_to_run & current_task) != 0)
            {
                // Clear this task in the queue list.
                tasks_to_run &= ~current_task;

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
                queued_tasks = (task_id_t) (queued_tasks | (tasks_to_run & ~(current_task - 1)));
                FSP_CRITICAL_SECTION_EXIT;
                break;
            }
        }
    }
}

/** @} */