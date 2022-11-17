/**
  ******************************************************************************
  * @file          vblock_pool.c
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
  * 1.1|Egahp|2022.11.17|重构API
  ******************************************************************************
  */

/* include -------------------------------------------------------------------*/
#include <stdint.h>
#include "vblock.h"

/* marco ---------------------------------------------------------------------*/

#ifdef CONFIG_VLOCK_DEBUG
#define _VBLOCK_CHECK(_expr, _ret) \
    if (!(_expr))                  \
    return _ret
#else
#define _VBLOCK_CHECK(_expr, _ret) ((void)0)
#endif

/* typedef -------------------------------------------------------------------*/

/* declare -------------------------------------------------------------------*/

/* variable ------------------------------------------------------------------*/

/* code ----------------------------------------------------------------------*/

/**
 *   @brief         create a vblock pool
 *   @param  vblock                 vblock instance
 *   @param  size                   block size
 *   @param  align                  block size
 *   @param  memory_ptr             memory pointer
 *   @param  memory_size            memory size
 *   @return int 
 */
int
vblock_create(
    vblock_t *vblock,
    uint32_t  size,
    uint32_t  align,
    void     *memory_ptr,
    uint32_t  memory_size)
{
    _VBLOCK_CHECK(vblock != NULL, -1);
    _VBLOCK_CHECK(memory_ptr != NULL, -1);
    _VBLOCK_CHECK(memory_size > 0, -1);
    _VBLOCK_CHECK(size > 0, -1);
    _VBLOCK_CHECK(((align >= VBLOCK_PARAM_ALIGN_1) && (align <= VBLOCK_PARAM_ALIGN_128)), -1);
    _VBLOCK_CHECK(((uintptr_t)memory_ptr & (sizeof(void *) - 1)) == 0, -1);

    uint32_t  bitmask = ((0x1 << align) - 1);
    uint32_t  count;
    uintptr_t address;

    if (size & bitmask)
    {
        size = (size & (~bitmask)) + (0x1 << align);
    }

    count = memory_size / (sizeof(void *) + size);

    /*!< not enough memory */
    if (count == 0)
    {
        return VBLOCK_RET_NOBLOCK;
    }

    /*!< calculate block area start address */
    address = (uintptr_t)memory_ptr + sizeof(void *) * count;

    /*!< align address */
    if (address & bitmask)
    {
        address = (address & (~bitmask)) + (0x1 << align);
    }

    /*!< if not enough memory after align */
    if ((address + (count * size)) > ((uintptr_t)memory_ptr + memory_size))
    {
        /*!< reduce count */
        count -= 1;

        /*!< not enough memory */
        if (count == 0)
        {
            return VBLOCK_RET_NOBLOCK;
        }

        /*!< recalculate block area start address */
        address = (uintptr_t)memory_ptr + sizeof(void *) * count;

        /*!< align address */
        if (address & bitmask)
        {
            address = (address & (~bitmask)) + (0x1 << align);
        }
    }

    /*!< build list */
    for (uint32_t i = 0; i < count; i++)
    {
        ((void **)memory_ptr)[i] = &((void **)memory_ptr)[i + 1];
    }

    /*!< last node link to NULL */
    ((void **)memory_ptr)[count - 1] = NULL;

    /*!< set free node list */
    vblock->list = memory_ptr;

    /*!< set node start address */
    vblock->node = memory_ptr;

    /*!< set block area start address */
    vblock->area = (void *)address;

    /*!< set block size (aligned) */
    vblock->size = size;

    vblock->total = count;

    vblock->free = count;

#if CONFIG_VBLOCK_THREAD
    vblock->mtx_get = NULL;
    vblock->mtx_put = NULL;
    vblock->sem_get = NULL;
    vblock->sem_put = NULL;
#endif

#if CONFIG_VBLOCK_PERF
    vblock->perf_alloc = 0;
    vblock->perf_free  = 0;
#if CONFIG_VBLOCK_THREAD
    vblock->perf_timeout = 0;
#endif
#endif

    return VBLOCK_RET_SUCCESS;
}

/**
 *   @brief         delete vblock
 *   @param  vblock                 block pool instance
 *   @return int 
 */
int
vblock_delete(vblock_t *vblock)
{
    _VBLOCK_CHECK(vblock != NULL, -1);

    if (vblock->total != vblock->free)
    {
        return VBLOCK_RET_OCCUPANCY;
    }

    vblock->list = NULL;
    vblock->node = NULL;
    vblock->area = NULL;

    vblock->size  = 0;
    vblock->total = 0;
    vblock->free  = 0;

#if CONFIG_VBLOCK_THREAD
    vblock->mtx_get = NULL;
    vblock->mtx_put = NULL;
    vblock->sem_get = NULL;
    vblock->sem_put = NULL;
#endif

#if CONFIG_VBLOCK_PERF
    vblock->perf_alloc = 0;
    vblock->perf_free  = 0;
#if CONFIG_VBLOCK_THREAD
    vblock->perf_timeout = 0;
#endif
#endif

    return VBLOCK_RET_SUCCESS;
}

#if CONFIG_VBLOCK_THREAD
/**
 *   @brief         add mutex
 *   @param  vblock                 block pool instance
 *   @param  get                    mutex get
 *   @param  put                    mutex put
 *   @return int 
 */
int
vblock_add_mtx(vblock_t *vblock, int (*get)(uint32_t wait), void (*put)(void))
{
    _VBLOCK_CHECK(vblock != NULL, -1);

    if ((get == NULL) || (put == NULL))
    {
        vblock->mtx_get = NULL;
        vblock->mtx_put = NULL;
    }
    else
    {
        vblock->mtx_get = get;
        vblock->mtx_put = put;
    }

    return VBLOCK_RET_SUCCESS;
}
#endif

#if CONFIG_VBLOCK_THREAD
/**
 *   @brief         add semaphore
 *   @param  vblock                 block pool instance
 *   @param  get                    semaphore get
 *   @param  put                    semaphore put
 *   @return int 
 */
int
vblock_add_sem(vblock_t *vblock, int (*get)(uint32_t wait), void (*put)(void))
{
    _VBLOCK_CHECK(vblock != NULL, -1);

    if ((get == NULL) || (put == NULL))
    {
        vblock->sem_get = NULL;
        vblock->sem_put = NULL;
    }
    else
    {
        vblock->sem_get = get;
        vblock->sem_put = put;
    }

    return VBLOCK_RET_SUCCESS;
}
#endif

/**
 *   @brief         alloc a block
 *   @param  vblock                 block pool instance
 *   @param  addr                   block pointer
 *   @param  wait                   wait time
 *   @return int 
 */
int
vblock_alloc(vblock_t *vblock, void **addr, uint32_t wait)
{
    _VBLOCK_CHECK(vblock != NULL, -1);
    _VBLOCK_CHECK(addr != NULL, -1);

    void    *node;
    uint32_t index;

#if CONFIG_VBLOCK_PERF
    vblock->perf_alloc += 1;
#endif

#if CONFIG_VBLOCK_THREAD
    if (vblock->sem_get)
    {
        /*!< check for free block by sem */
        if (vblock->sem_get(wait))
        {
            return VBLOCK_RET_NOBLOCK;
        }
    }
#endif

    /*!< always check for free block */
    if (vblock->free == 0)
    {
        return VBLOCK_RET_NOBLOCK;
    }

#if CONFIG_VBLOCK_THREAD
    /*!< get mutex */
    if (vblock->mtx_get)
    {
        if (vblock->mtx_get(CONFIG_VBLOCK_TIMEOUT))
        {
            vblock->sem_put();

#if CONFIG_VBLOCK_PERF
            vblock->perf_timeout += 1;
#endif

            return VBLOCK_RET_TIMEOUT;
        }
    }
#endif

    /*!< get first free list node */
    node = vblock->list;

    /*!< delete node from list */
    vblock->list = *((void **)node);

    /*!< calculate the index */
    index = ((uintptr_t)node - (uintptr_t)(vblock->node)) / sizeof(void *);

    /*!< calculate block address */
    *addr = (void *)((uintptr_t)(vblock->area) + (index * vblock->size));

    /*!< decrease free block count */
    vblock->free -= 1;

#if CONFIG_VBLOCK_THREAD
    /*!< put mutex */
    if (vblock->mtx_put)
    {
        vblock->mtx_put();
    }
#endif

    return VBLOCK_RET_SUCCESS;
}

/**
 *   @brief         free a block
 *   @param  vblock                 block pool instance
 *   @param  addr                   block pointer
 *   @return int 
 */
int
vblock_free(vblock_t *vblock, void *addr)
{
    _VBLOCK_CHECK(vblock != NULL, -1);
    _VBLOCK_CHECK(addr != NULL, -1);

    void    *node;
    uint32_t index;

#if CONFIG_VBLOCK_THREAD
    if (vblock->mtx_get)
    {
        if (vblock->mtx_get(CONFIG_VBLOCK_TIMEOUT))
        {
#if CONFIG_VBLOCK_PERF
            vblock->perf_timeout += 1;
#endif
            return VBLOCK_RET_TIMEOUT;
        }
    }
#endif

    /*!< simple check if addr is our block */
    if (vblock->total == vblock->free)
    {
        return VBLOCK_RET_OTHERS;
    }

#if CONFIG_VBLOCK_PERF
    vblock->perf_free += 1;
#endif

    /*!< calculate the index */
    /*!< maybe the original value calculated will be negative, but it has no effect. */
    index = ((uintptr_t)addr - (uintptr_t)(vblock->area)) / vblock->size;

    /*!< this is not our block */
    if (index > vblock->total)
    {
        return -1;
    }

    /*!< calculate node address */
    node = (void *)((uintptr_t)(vblock->node) + (index * sizeof(void *)));

    /*!< link first free list node */
    *((void **)node) = vblock->list;

    /*!< set first free list node */
    vblock->list = node;

    /*!< increase free block count */
    vblock->free += 1;

#if CONFIG_VBLOCK_THREAD
    if (vblock->mtx_put)
    {
        vblock->mtx_put();
    }

    if (vblock->sem_put)
    {
        vblock->sem_put();
    }
#endif

    return VBLOCK_RET_SUCCESS;
}

/**
 *   @brief         get vblock total and free block info
 *   @param  vblock                 block pool instance
 *   @param  total                  total block count
 *   @param  free                   free block count
 */
void
vblock_get_info(vblock_t *vblock, uint32_t *total, uint32_t *free)
{
    _VBLOCK_CHECK(vblock != NULL, );

    if (total != NULL)
    {
        *total = vblock->total;
    }

    if (free != NULL)
    {
        *free = vblock->free;
    }
}

#if CONFIG_VBLOCK_PERF
void
vblock_get_perf(vblock_t *vblock, uint32_t *perf_alloc, uint32_t *perf_free)
{
    _VBLOCK_CHECK(vblock != NULL, );

    if (perf_alloc != NULL)
    {
        *perf_alloc = vblock->perf_alloc;
    }

    if (perf_free != NULL)
    {
        *perf_free = vblock->perf_free;
    }
}
#endif

#if (CONFIG_VBLOCK_PERF && CONFIG_VBLOCK_THREAD)
void
vblock_get_timeout(vblock_t *vblock, uint32_t *perf_timeout)
{
    _VBLOCK_CHECK(vblock != NULL, );

    if (perf_timeout != NULL)
    {
        *perf_timeout = vblock->perf_timeout;
    }
}
#endif

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/
