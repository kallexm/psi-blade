#include "pb_mem_pool.h"

#include "stdio.h"

pb_mem_pool_retval_t pb_mem_pool_zero_init(pb_mem_pool_t *mem_pool)
{
    memset(mem_pool->memory_pool, 0, mem_pool->num_chunks_max*mem_pool->sizeof_chunk);
    memset(mem_pool->memory_used, 0, mem_pool->num_chunks_max*sizeof(bool));
    return PB_MEM_POOL_SUCCESS;
}


pb_mem_pool_retval_t pb_mem_pool_get_memory(pb_mem_pool_t *mem_pool, void** mem_ptr)
{
    if (mem_pool->num_chunks_used >= mem_pool->num_chunks_max)
    {   
        *mem_ptr = NULL;
        return PB_MEM_POOL_FAILURE;
    }

    for (uint32_t i = 0; i < mem_pool->num_chunks_max; i++)
    {
        if (mem_pool->memory_used[i] == false)
        {
            mem_pool->num_chunks_used++;
            mem_pool->memory_used[i] = true;
            *mem_ptr = (void*)(mem_pool->memory_pool + i*mem_pool->sizeof_chunk);
            return PB_MEM_POOL_SUCCESS;
        }
    }
    return PB_MEM_POOL_FAILURE;
}


pb_mem_pool_retval_t pb_mem_pool_return_memory(pb_mem_pool_t *mem_pool, void** mem_ptr)
{   
    void* pool_start = mem_pool->memory_pool;
    void* pool_end   = mem_pool->memory_pool + mem_pool->num_chunks_max*mem_pool->sizeof_chunk-1;
    if (!(pool_start <= *mem_ptr || *mem_ptr <= pool_end))
    {   
        printf("Hell!\n");
        return PB_MEM_POOL_FAILURE;
    }

    

    for (uint32_t i = 0; i < mem_pool->num_chunks_max; i++)
    {   
        if ( (mem_pool->memory_used[i] == true) && ((uint64_t)mem_pool->memory_pool + i*mem_pool->sizeof_chunk <= (uint64_t)*mem_ptr) && ((uint64_t)*mem_ptr < (uint64_t)mem_pool->memory_pool + (i+1)*mem_pool->sizeof_chunk) )
        {   
            *mem_ptr = NULL;
            mem_pool->memory_used[i] = false;
            mem_pool->num_chunks_used--;
            memset( (void*)((uint64_t)mem_pool->memory_pool + i*mem_pool->sizeof_chunk), 0, mem_pool->sizeof_chunk);
            return PB_MEM_POOL_SUCCESS;
        }
    }
    return PB_MEM_POOL_FAILURE;
}



int main()
{
    printf("Hei!\n");
    MEMORY_POOL_DEF(test_pool, uint32_t, 3);
    pb_mem_pool_zero_init(&test_pool);

    printf("test_pool.memory_pool = 0x%x\n", test_pool.memory_pool);

    uint32_t* test_a = NULL;
    uint32_t* test_b = NULL;
    pb_mem_pool_retval_t err;

    printf("test_pool.num_chunks_used = %u\n", test_pool.num_chunks_used);
    err = pb_mem_pool_get_memory(&test_pool, (void**)&test_a);
    if (err != PB_MEM_POOL_SUCCESS)
    {
        printf("Error = %u\n", err);
    }

    printf("test_pool.num_chunks_used = %u\n", test_pool.num_chunks_used);
    err = pb_mem_pool_get_memory(&test_pool, (void**)&test_b);
    if (err != PB_MEM_POOL_SUCCESS)
    {
        printf("Error = %u\n", err);
    }

    printf("Hei!\n");

    printf("test_pool.num_chunks_used = %u\n", test_pool.num_chunks_used);

    printf("test_a = %p\n", (uint32_t*)test_a);

    printf("*test_a = %u\n", *(uint32_t*)test_a);
    *test_a = 10;
    printf("*test_a = %u\n", *(uint32_t*)test_a);
    *test_b = 22;

    printf("Hei!\n");

    for (int i = 0; i < 3; i++)
    {
        printf("i = %u:%u\n", i, ((uint32_t*)(test_pool.memory_pool))[i]);
    }

    err = pb_mem_pool_return_memory(&test_pool, (void**)&test_a);

    for (int i = 0; i < 3; i++)
    {
        printf("i = %u:%u\n", i, ((uint32_t*)(test_pool.memory_pool))[i]);
    }

    printf("test_pool.num_chunks_used = %u\n", test_pool.num_chunks_used);

    printf("END!\n");

    return 0;
}
