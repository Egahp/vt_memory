/**
  ******************************************************************************
  * @file          vt_block_pool.c
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

/* include -------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "vt_block_pool.h"

/* marco ---------------------------------------------------------------------*/
#ifdef CONFIG_VT_BLOCK_POOL_DEBUG
#define _VT_BLOCK_POOL_CHECK(_expr, _ret) \
    if (!(_expr))                   \
    return _ret
#else
#define _VT_BLOCK_POOL_CHECK(_expr, _ret) ((void)0)
#endif

/* typedef -------------------------------------------------------------------*/

/* declare -------------------------------------------------------------------*/
#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
extern int vt_block_pool_semaphore_create(vt_block_pool_semaphore_t *sem, uint32_t count);
extern int vt_block_pool_semaphore_delete(vt_block_pool_semaphore_t *sem);
extern int vt_block_pool_semaphore_get(vt_block_pool_semaphore_t *sem, uint32_t wait);
extern int vt_block_pool_semaphore_put(vt_block_pool_semaphore_t *sem);
extern int vt_block_pool_mutex_create(vt_block_pool_mutex_t *mtx);
extern int vt_block_pool_mutex_delete(vt_block_pool_mutex_t *mtx);
extern int vt_block_pool_mutex_get(vt_block_pool_mutex_t *mtx, uint32_t wait);
extern int vt_block_pool_mutex_put(vt_block_pool_mutex_t *mtx);
#endif

/* variable ------------------------------------------------------------------*/

/* code ----------------------------------------------------------------------*/

/**
 *   @brief         create a block heap
 *   @param  heap                   heap
 *   @param  nodes                  nodes pool, size = blk_count
 *   @param  pool_ptr               memory pool, size = blk_count * blk_size
 *   @param  blk_count              block count
 *   @param  blk_size               block size
 *   @return int 
 */
int vt_block_pool_create(vt_block_pool_t *heap, vt_block_pool_node_t *nodes, uint8_t *pool_ptr, uint32_t blk_count, uint32_t blk_size)
{
    _VT_BLOCK_POOL_CHECK(heap != NULL, -1);
    _VT_BLOCK_POOL_CHECK(nodes != NULL, -1);
    _VT_BLOCK_POOL_CHECK(pool_ptr != NULL, -1);
    _VT_BLOCK_POOL_CHECK(blk_count > 0, -1);
    _VT_BLOCK_POOL_CHECK(blk_size > 0, -1);

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    if (vt_block_pool_semaphore_create(&heap->sem, blk_count)) {
        return -1;
    }

    if (vt_block_pool_mutex_create(&heap->mtx)) {
        vt_block_pool_semaphore_delete(&heap->sem);
        return -1;
    }
#endif

    heap->busy_list = NULL;
    heap->free_list = &nodes[0];

    for (uint32_t i = 0; i < blk_count; i++) {
        nodes[i].next = &nodes[i + 1];
        nodes[i].pool = &pool_ptr[blk_size * i];
    }

    nodes[blk_count - 1].next = NULL;

    return 0;
}

/**
 *   @brief         delete a block heap
 *   @param  heap                   heap
 *   @return int 
 */
int vt_block_pool_delete(vt_block_pool_t *heap)
{
    _VT_BLOCK_POOL_CHECK(heap != NULL, -1);

    if (heap->busy_list != NULL) {
        return -1;
    }

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    if (vt_block_pool_mutex_delete(&heap->mtx)) {
        return -1;
    }

    if (vt_block_pool_semaphore_delete(&heap->sem)) {
        return -1;
    }
#endif

    return 0;
}

/**
 *   @brief         alloc a block with timeout
 *   @param  heap                   heap
 *   @param  blk_ptr                block pointer
 *   @param  wait                   timeout
 *   @return int 
 */
int vt_block_pool_alloc_wait(vt_block_pool_t *heap, void **blk_ptr, uint32_t wait)
{
    _VT_BLOCK_POOL_CHECK(heap != NULL, -1);
    _VT_BLOCK_POOL_CHECK(blk_ptr != NULL, -1);

    vt_block_pool_node_t *node;

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    if (vt_block_pool_semaphore_get(&heap->sem, wait)) {
        return -1;
    }

    if (vt_block_pool_mutex_get(&heap->mtx, CONFIG_VT_BLOCK_POOL_MUTEX_TIMEOUT)) {
        vt_block_pool_semaphore_put(&heap->sem);
        return -1;
    }
#else
    if (heap->free_list == NULL) {
        return -1;
    }
#endif

    /*!< delete node from free_list */
    node = heap->free_list;
    heap->free_list = heap->free_list->next;

    /*!< add node to busy_list */
    node->next = heap->busy_list;
    heap->busy_list = node;

    /*!< block pointer */
    *blk_ptr = node->pool;

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    vt_block_pool_mutex_put(&heap->mtx);
#endif

    return 0;
}

/**
 *   @brief         alloc a block
 *   @param  heap                   heap
 *   @param  blk_ptr                block pointer
 *   @return int 
 */
int vt_block_pool_alloc(vt_block_pool_t *heap, void **blk_ptr)
{
    return vt_block_pool_alloc_wait(heap, blk_ptr, 0);
}

/**
 *   @brief         free a block
 *   @param  heap                   heap
 *   @param  blk_ptr                block pointer
 *   @return int 
 */
int vt_block_pool_free(vt_block_pool_t *heap, void *blk_ptr)
{
    _VT_BLOCK_POOL_CHECK(heap != NULL, -1);
    _VT_BLOCK_POOL_CHECK(blk_ptr != NULL, -1);

    vt_block_pool_node_t *node;
    vt_block_pool_node_t *next;
    uint8_t flag = 0;

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    if (vt_block_pool_mutex_get(&heap->mtx, CONFIG_VT_BLOCK_POOL_MUTEX_TIMEOUT)) {
        return -1;
    }
#else
    if (heap->busy_list == NULL) {
        return -1;
    }
#endif

    /*!< first busy_list node */
    node = heap->busy_list;

    /*!< second busy_list node */
    next = heap->busy_list->next;

    if (node->pool == blk_ptr) {
        /*!< delete node from busy_list */
        heap->busy_list = next;

        /*!< add node to free_list */
        node->next = heap->free_list;
        heap->free_list = node;

        /*!< set success flag */
        flag = 1;
    } else {
        while (next != NULL) {
            if (next->pool == blk_ptr) {
                /*!< delete node from busy_list */
                node->next = next->next;

                /*!< add node to free_list */
                next->next = heap->free_list;
                heap->free_list = next;

                /*!< set success flag */
                flag = 1;
                break;
            }

            node = next;
            next = next->next;
        }
    }

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    vt_block_pool_mutex_put(&heap->mtx);

    if (flag) {
        vt_block_pool_semaphore_put(&heap->sem);
        return 0;
    }

#else
    if (flag) {
        return 0;
    }
#endif

    return -1;
}

/**
 *   @brief         get block heap info
 *   @param  heap                   heap
 *   @param  busy_blk               busy block count pointer
 *   @param  free_blk               free block count pointer
 *   @return int 
 */
int vt_block_pool_get_info(vt_block_pool_t *heap, uint32_t *busy_blk, uint32_t *free_blk)
{
    _VT_BLOCK_POOL_CHECK(heap != NULL, -1);
    _VT_BLOCK_POOL_CHECK(busy_blk != NULL, -1);
    _VT_BLOCK_POOL_CHECK(free_blk != NULL, -1);

    vt_block_pool_node_t *node;

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    if (vt_block_pool_mutex_get(&heap->mtx, CONFIG_VT_BLOCK_POOL_MUTEX_TIMEOUT)) {
        return -1;
    }
#endif

    *busy_blk = 0;

    node = heap->busy_list;

    while (node != NULL) {
        node = node->next;
        *busy_blk += 1;
    }

    *free_blk = 0;

    node = heap->free_list;

    while (node != NULL) {
        node = node->next;
        *free_blk += 1;
    }

#ifdef CONFIG_VT_BLOCK_POOL_MULTITHREAD
    vt_block_pool_mutex_put(&heap->mtx);
#endif

    return 0;
}

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/
