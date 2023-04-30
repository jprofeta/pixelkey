#ifndef HAL_TASKS_H
#define HAL_TASKS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup hal__tasks Task Manager
 * @ingroup hal
 * @{
 */

/** XMACRO for defining tasks. */
#define TASK_LIST           \
    XTASK(USB_HANDLER)      \
    XTASK(FRAME_RENDER)     \
    XTASK(CMD_RX)

#define XTASK(task)     TASK_ ## task,
typedef enum e_task
{
    TASK_LIST
    TASK_COUNT,
    TASK_UNDEFINED = -1
} task_t;
#undef XTASK

typedef void (*task_fn_t)(void);

/**
 * Main task runner loop.
 */
void tasks_run(void);

/**
 * Queue a task for execution.
 * @param task The task to queue.
 */
void tasks_queue(task_t task);

/**
 * Checks if a task is queued to run.
 * @param task The task to check.
 * @return true if the task is queued, false otherwise.
 */
bool tasks_is_queued(task_t task);

/** @} */

#endif
