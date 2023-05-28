#include "pixelkey_hal.h"
#include "hal_tasks.h"

pixelkey_error_t pixelkey_hal_frame_timer_update(framerate_t new_framerate)
{
    (void)new_framerate;
    return PIXELKEY_ERROR_NONE;
}

void tasks_queue(task_t task)
{

}
