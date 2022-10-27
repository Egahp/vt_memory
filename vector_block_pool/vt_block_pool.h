/**
  ******************************************************************************
  * @file          vt_block_pool.h
  * @brief         block memory pool
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

#ifndef _VT_BLOCK_POOL_H
#define _VT_BLOCK_POOL_H

#include <stdint.h>
#include "vt_block_pool_conf.h"

typedef struct {
    void *next; /*!< next nodes */
    void *pool; /*!< block ptr  */
} vt_block_pool_node_t;

typedef struct {
    vt_block_pool_node_t *free_list; /*!< free node list */
    vt_block_pool_node_t *busy_list; /*!< busy node list */

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    vt_block_pool_semaphore_t sem;
    vt_block_pool_mutex_t mtx;
#endif
} vt_block_pool_t;

extern int vt_block_pool_create(vt_block_pool_t *heap, vt_block_pool_node_t *nodes, uint8_t *pool_ptr, uint32_t blk_count, uint32_t blk_size);
extern int vt_block_pool_delete(vt_block_pool_t *heap);
extern int vt_block_pool_alloc(vt_block_pool_t *heap, void **blk_ptr);
extern int vt_block_pool_free(vt_block_pool_t *heap, void *blk_ptr);
extern int vt_block_pool_get_info(vt_block_pool_t *heap, uint32_t *busy_blk, uint32_t *free_blk);

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
extern int vt_block_pool_alloc_wait(vt_block_pool_t *heap, void **blk_ptr, uint32_t wait);
#endif

#endif

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/
