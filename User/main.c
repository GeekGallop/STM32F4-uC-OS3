#include "bsp.h"
#include "task.h"

int main(void)
{
    /* 板级初始化 */
    bsp_init();
    
    /* 任务启动 */
    app_start();
    
    return 0;
}
