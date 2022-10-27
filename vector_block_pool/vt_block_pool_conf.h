/**
  ******************************************************************************
  * @file          vt_block_pool_conf.h
  * @brief         block memory pool config
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

#ifndef _VT_BLOCK_POOL_CONF_H
#define _VT_BLOCK_POOL_CONF_H

#if defined(CONFIG_VT_BLOCK_POOL_MULTITHREAD)

#if (CONFIG_VT_BLOCK_POOL_MULTITHREAD == 1)
#include <FreeRTOS.h>
#include "task.h"
#include "semphr.h"

#ifndef CONFIG_VT_BLOCK_POOL_MUTEX_TIMEOUT
#define CONFIG_VT_BLOCK_POOL_MUTEX_TIMEOUT 1000
#endif

#define vt_block_pool_semaphore_t SemaphoreHandle_t
#define vt_block_pool_mutex_t     SemaphoreHandle_t

#else
#error "block pool multithread illegal port"
#endif

#endif

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/

#endif