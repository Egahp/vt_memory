/**
  ******************************************************************************
  * @file          vblock.h
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

#ifndef __VBLOCK_H__
#define __VBLOCK_H__

#include <stdint.h>

/** @addtogroup vblock
 * @{
 */

/** @addtogroup vblock config
 * @{
 */

/*!< enable performance tracking */
#ifndef CONFIG_VBLOCK_PERF
#define CONFIG_VBLOCK_PERF 1
#endif

/*!< enable multi-threading support */
#ifndef CONFIG_VBLOCK_THREAD
#define CONFIG_VBLOCK_THREAD 1
#endif

/*!< mutex timeout */
#ifndef CONFIG_VBLOCK_TIMEOUT
#define CONFIG_VBLOCK_TIMEOUT 100
#endif

/**
 * @}
 */

/** @addtogroup vblock param
 * @{
 */

/*!< align size */
#define VBLOCK_PARAM_ALIGN_1   0x00
#define VBLOCK_PARAM_ALIGN_2   0x01
#define VBLOCK_PARAM_ALIGN_4   0x02
#define VBLOCK_PARAM_ALIGN_8   0x03
#define VBLOCK_PARAM_ALIGN_16  0x04
#define VBLOCK_PARAM_ALIGN_32  0x05
#define VBLOCK_PARAM_ALIGN_64  0x06
#define VBLOCK_PARAM_ALIGN_128 0x07

/**
 * @}
 */

/** @addtogroup vblock return value
 * @{
 */

#define VBLOCK_RET_SUCCESS   0
#define VBLOCK_RET_NOBLOCK   -1
#define VBLOCK_RET_OTHERS    -2
#define VBLOCK_RET_OCCUPANCY -3
#define VBLOCK_RET_TIMEOUT   -4

/**
 * @}
 */

/** @addtogroup vblock type
 * @{
 */

typedef struct
{
    void *list; /*!< free block list    */
    void *node; /*!< list node address  */
    void *area; /*!< block area address */

#if CONFIG_VBLOCK_THREAD
    int (*mtx_get)(uint32_t wait); /*!< mutex get     */
    void (*mtx_put)(void);         /*!< mutex put     */
    int (*sem_get)(uint32_t wait); /*!< semaphore get */
    void (*sem_put)(void);         /*!< semaphore put */
#endif

    uint32_t size;  /*!< block size         */
    uint32_t total; /*!< block total num    */
    uint32_t free;  /*!< block free num     */

#if CONFIG_VBLOCK_PERF
    uint32_t perf_alloc; /*!< performance info total alloc count */
    uint32_t perf_free;  /*!< performance info total free count */
#if CONFIG_VBLOCK_THREAD
    uint32_t perf_timeout; /*!< performance info total timeout count     */
#endif
#endif

} vblock_t;

/**
 * @}
 */

/** @addtogroup vblock api
 * @{
 */

extern int vblock_create(vblock_t *vblock, uint32_t size, uint32_t align, void *memory_ptr, uint32_t memory_size);
extern int vblock_delete(vblock_t *vblock);

#if CONFIG_VBLOCK_THREAD
extern int vblock_add_mtx(vblock_t *vblock, int (*get)(uint32_t wait), void (*put)(void));
extern int vblock_add_sem(vblock_t *vblock, int (*get)(uint32_t wait), void (*put)(void));
#endif

extern int vblock_alloc(vblock_t *vblock, void **addr, uint32_t wait);
extern int vblock_free(vblock_t *vblock, void *addr);

extern void vblock_get_info(vblock_t *vblock, uint32_t *total, uint32_t *free);

#if CONFIG_VBLOCK_PERF
extern void vblock_get_perf(vblock_t *vblock, uint32_t *perf_alloc, uint32_t *perf_free);
#if CONFIG_VBLOCK_THREAD
extern void vblock_get_timeout(vblock_t *vblock, uint32_t *perf_timeout);
#endif
#endif

/**
 * @}
 */

/**
 * @}
 */

#endif

/************************ (C) COPYRIGHT 2021 Egahp *****END OF FILE*************/
