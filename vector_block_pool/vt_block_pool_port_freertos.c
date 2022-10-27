/**
  ******************************************************************************
  * @file          vt_block_pool_port_freertos.c
  * @brief         block memory pool port for freertos
  * @author        Egahp
  *                2687434412@qq.com
  * @version       1.0
  * @date          2022.10.27
  ******************************************************************************
  * @attention
  * 
  * <h2><center>&copy; Copyright 2021 Egahp.
  * All rights reserved.</center></h2>
  * 
  * @htmlonly 
  * <span style='font-weight: bold'>History</span> 
  * @endhtmlonly
  * 版本|作者|时间|描述
  * ----|----|----|----
  * 1.0|Egahp|2022.10.27|创建文件
  ******************************************************************************
  */

/* include -------------------------------------------------------------------*/
#include "vt_block_pool_conf.h"

/* marco ---------------------------------------------------------------------*/

/* typedef -------------------------------------------------------------------*/

/* declare -------------------------------------------------------------------*/

/* variable ------------------------------------------------------------------*/

/* code ----------------------------------------------------------------------*/

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD

/** @addtogroup semaphore
 * @{
 */

int vt_block_pool_semaphore_create(vt_block_pool_semaphore_t *sem, uint32_t count)
{
    *sem = xSemaphoreCreateCounting(count, count);

    if (*sem == NULL) {
        return -1;
    }

    return 0;
}

int vt_block_pool_semaphore_delete(vt_block_pool_semaphore_t *sem)
{
    vSemaphoreDelete(*sem);
    return 0;
}

int vt_block_pool_semaphore_get(vt_block_pool_semaphore_t *sem, uint32_t wait)
{
    if (pdTRUE != xSemaphoreTake(*sem, wait)) {
        return -1;
    }

    return 0;
}

int vt_block_pool_semaphore_put(vt_block_pool_semaphore_t *sem)
{
    if (pdTRUE != xSemaphoreGive(*sem)) {
        return -1;
    }

    return 0;
}

/**
 * @}
 */

/** @addtogroup mutex
 * @{
 */

int vt_block_pool_mutex_create(vt_block_pool_mutex_t *mtx)
{
    *mtx = xSemaphoreCreateMutex();

    if (*mtx == NULL) {
        return -1;
    }

    return 0;
}

int vt_block_pool_mutex_delete(vt_block_pool_mutex_t *mtx)
{
    vSemaphoreDelete(*mtx);
    return 0;
}

int vt_block_pool_mutex_get(vt_block_pool_mutex_t *mtx, uint32_t wait)
{
    if (pdTRUE != xSemaphoreTake(*mtx, wait)) {
        return -1;
    }

    return 0;
}

int vt_block_pool_mutex_put(vt_block_pool_mutex_t *mtx)
{
    if (pdTRUE != xSemaphoreGive(*mtx)) {
        return -1;
    }

    return 0;
}

/**
 * @}
 */

#endif

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/
