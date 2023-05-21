#ifndef HAL_TASKS_H
#define HAL_TASKS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup hal__tasks Task Manager
 * @ingroup hal
 * @{
 */

/**
 * XMACRO(name,fn,docstring) for defining tasks.
 * @note Tasks should be defined in order of priority (highest first).
 */
#define TASK_LIST \
    XTASK(FRAME_TX, npdata_frame_send, Transmits the last rendered frame.) \
    XTASK(FRAME_RENDER, pixelkey_task_do_frame, Calculates the next frame.) \
    XTASK(TERMINAL_CONNECTED, pixelkey_task_terminal_connected, Sends strings for newly connected terminals.) \
    XTASK(CMD_RX, pixelkey_task_command_rx, Command string reception and parsing.) \
    XTASK(CMD_HANDLER, pixelkey_commandproc_task, Command handling.) \
    XTASK(CMD_PROMPT, pixelkey_task_command_prompt, Sends the command prompt string once command handling completes.) \


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

void tasks_run(task_fn_t idle_task);

void tasks_queue(task_t task);

task_status_t tasks_status_get(task_t task);

/** @} */

#endif
