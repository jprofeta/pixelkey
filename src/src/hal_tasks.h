#ifndef HAL_TASKS_H
#define HAL_TASKS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup hal__tasks Task Manager
 * @ingroup hal
 * @{
 */

/** XMACRO(name,fn,docstring) for defining tasks. */
#define TASK_LIST \
    XTASK(USB_HANDLER, hal_usbcmd_handler, Handles USB events.) \
    XTASK(FRAME_RENDER, frame_render, Calculates the next frame.) \
    XTASK(CMD_RX, cmd_data_handler, Command string reception and parsing.)


#define XTASK(task,fn,doc)     TASK_ ## task /*!< doc See @ref fn. */,
/** Available tasks. */
typedef enum e_task
{
    TASK_LIST
    TASK_COUNT,         ///< Number of available tasks.
    TASK_UNDEFINED = -1 ///< Task is undefined.
} task_t;
#undef XTASK

/** Task status. */
typedef enum e_task_status
{
    TASK_STATUS_IDLE,    ///< The task is idle and not queued to run.
    TASK_STATUS_QUEUED,  ///< The task has been queued to run.
    TASK_STATUS_PENDING, ///< The task is pending execution and is awaiting higher priority tasks.
    TASK_STATUS_RUNNING  ///< The task is currently executing.
} task_status_t;

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
 * Gets the status of a given task.
 * @param task The task to check.
 * @raturn The task status.
 */
task_status_t tasks_status_get(task_t task);

/** @} */

#endif
