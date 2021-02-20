#ifndef mm
#define MM_MAX_STRUCT_NAME 32
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include "glthreads/glthread.h"

typedef enum{
    MM_FALSE,
    MM_TRUE
}vm_bool_t;

typedef struct vm_page_family_{
    char struct_name[MM_MAX_STRUCT_NAME];
    uint32_t struct_size;
    vm_page_t *first_page;
    glthread_t free_block_priority_head;
} vm_page_family_t;

typedef struct vm_page_for_families{
    struct vm_page_for_families *next;
    vm_page_family_t vm_page_family[0];
}vm_page_for_families;

typedef struct block_meta_data{
    vm_bool_t is_free;
    uint32_t block_size;
    uint32_t offset;
    struct block_meta_data *next;
    struct block_meta_data *prev;
    glthread_t priority_glue;
}block_meta_data_t;
GLTHREAD_TO_STRUCT(glthread_to_block_meta_data, block_meta_data_tt, priority_thread_glue, glthread_ptr);

typedef struct vm_page_{
    struct vm_page_ *next;
    struct vm_page_ *prev;
    struct vm_page_family_ *pg_family;
    block_meta_data_t block_meta_data;
    char page_memory[0];
}vm_page_t;

#define MAX_FAMILIES_PER_VM_PAGE  ((SYSTEM_PAGE_SIZE - sizeof(vm_page_family_t*))/sizeof(vm_page_family_t))

#define offset_of(container_structure, field_name) ((size_t)&(((container_structure *)0)->field_name))

#define MM_GET_PAGE_FROM_META_BLOCK (block_meta_data_ptr) ((void *)((char*)block_meta_data_ptr - block_meta_data_ptr->offset))

#define NEXT_META_BLOCK (block_meta_data_ptr) (block_meta_data_ptr->next)

#define PREV_META_BLOCK (block_meta_data_ptr) (block_meta_data_ptr->prev)

#define NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr) (block_meta_data_ptr *) ((char*)(block_meta_data_ptr+1)+block_meta_data_ptr->block_size)


#define mm_bind_blocks_for_allocation(allocated_meta_block, free_meta_block) \
    free_meta_block->next = allocated_meta_block->next;                      \
    free_meta_block->previous = allocated_meta_block;                        \
    allocated_meta_block->next = free_meta_block;                            \
    if(free_meta_block->next != NULL){                                       \
        free_meta_block->next->previous = free_meta_block;                   \
    }

#define MARK_VM_PAGE_EMPTY(vm_page_t_ptr)\
    vm_page_t_ptr->block_meta_data.next = NULL;\
    vm_page_t_ptr->block_meta_data.prev = NULL;\
    vm_page_t_ptr->block_meta_data.is_free = MM_TRUE;\


#endif