#ifndef PB_MEM_POOL_H__
#define PB_MEM_POOL_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    void*       memory_pool;
    bool*       memory_used;
    uint32_t    sizeof_chunk;
    uint32_t    num_chunks_max;
    uint32_t    num_chunks_used;
} pb_mem_pool_t;

typedef enum
{
    PB_MEM_POOL_SUCCESS,
    PB_MEM_POOL_FAILURE,
    PB_MEM_POOL_NOT_IMPLEMENTED,
}pb_mem_pool_retval_t;


/* Brief: Defines a struct of type pb_mem_pool_t and allocate 
 *        memory for memory_pool and memory_used.
 */
#define MEMORY_POOL_DEF(name_, typeof_chunk_, num_chunks_)                               \
            typeof_chunk_ name_ ## _pool[(num_chunks_)];                                 \
            bool          name_ ## _used[(num_chunks_)];                                 \
            pb_mem_pool_t (name_) = {                                                    \
                (name_ ## _pool),                                                        \
                (name_ ## _used),                                                        \
                sizeof(typeof_chunk_),                                                   \
                (num_chunks_),                                                           \
                0                                                                        \
            }


pb_mem_pool_retval_t pb_mem_pool_zero_init(pb_mem_pool_t *mem_pool);


pb_mem_pool_retval_t pb_mem_pool_get_memory(pb_mem_pool_t *mem_pool, void** mem_ptr);


pb_mem_pool_retval_t pb_mem_pool_return_memory(pb_mem_pool_t *mem_pool, void** mem_ptr);


#endif /* PB_MEM_POOL_H__ */
