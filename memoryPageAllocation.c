#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mm.h"
#include <assert.h>

static size_t SYSTEM_PAGE_SIZE = 0;

static vm_page_for_families *first_vm_page_for_families = NULL;
void mm_init(){
    SYSTEM_PAGE_SIZE = getpagesize();
}

/*REQUEST WM PAGE FROM KERNEL*/

static void * get_new_VM_page(int units){
    char *vm_page = mmap(0,units*SYSTEM_PAGE_SIZE,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANON|MAP_PRIVATE,0,0);
    if(vm_page == MAP_FAILED){
        printf("Error : VM Page allocation failed\n");
        return NULL;
    }
    memset(vm_page,0,units*SYSTEM_PAGE_SIZE);
    return (void *)vm_page;
}

static void return_VM_to_kernel(void *vm_page, int units){
    if(munmap(vm_page,units*SYSTEM_PAGE_SIZE)){
        printf("Error : Could not munmap VM page to kernel");
    }
}

void mm_instantiate_new_page_family(char *struct_name, uint32_t struct_size){
    uint32_t count = 0;
    vm_page_family_t *current_family = NULL;

    if(struct_size>SYSTEM_PAGE_SIZE){
        printf("Error: %s() Structure exceeds system page size\n",struct_name);
        return;
    }

    if(first_vm_page_for_families == NULL){
        first_vm_page_for_families = (vm_page_for_families*)get_new_VM_page(1);
        first_vm_page_for_families->next == NULL;
        strcpy(first_vm_page_for_families->vm_page_family[0].struct_name, struct_name);
        first_vm_page_for_families->vm_page_family[0].struct_size = struct_size;
        init_glthread(&first_vm_page_for_families->vm_page_family[0].free_block_priority_head, (unsigned int)offset_of(block_meta_data_t, priority_glue));
        return;
    }

    //check for duplicated names
    for(current_family = &first_vm_page_for_families->vm_page_family[0];current_family->struct_size && count<MAX_FAMILIES_PER_VM_PAGE;current_family++,count++){
        if(strcmp(struct_name, current_family->struct_name)!=0){
            count++;
            continue;
        }
        assert(0);
    }
    
    //already filled page
    if(count == MAX_FAMILIES_PER_VM_PAGE){
        vm_page_for_families *new_vm_page_for_families;
        new_vm_page_for_families = (vm_page_for_families*)get_new_VM_page(1);
        new_vm_page_for_families->next = first_vm_page_for_families;
        first_vm_page_for_families = new_vm_page_for_families;
        current_family = &first_vm_page_for_families->vm_page_family[0];
    }
    strcpy(current_family->struct_name,struct_name);
    current_family->struct_size = struct_size;
}


void mm_print_registeredFamilies(){
    vm_page_family_t *current_family = NULL;
    uint32_t count = 0;
    if(first_vm_page_for_families == NULL){
        printf("No Families Registered\n");
    }else{
        while (first_vm_page_for_families){
            for(current_family = &first_vm_page_for_families->vm_page_family[0];current_family->struct_size && count<MAX_FAMILIES_PER_VM_PAGE;current_family++,count++){
                printf("Page Family: %s, Size = %d\n",current_family->struct_name,current_family->struct_size);
            }
            first_vm_page_for_families = first_vm_page_for_families->next;
        }
    }
}

vm_page_family_t * lookup_page_family_by_name (char *struct_name){
    vm_page_family_t *current_family = NULL;
    uint32_t count = 0;
    if(first_vm_page_for_families == NULL){
    printf("No Families Registered\n");
    }else{
        while (first_vm_page_for_families){
            for(current_family = &first_vm_page_for_families->vm_page_family[0];current_family->struct_size && count<MAX_FAMILIES_PER_VM_PAGE;current_family++,count++){
                if(strcmp(current_family->struct_name, struct_name)==0){
                    return current_family;
                }
            }
            first_vm_page_for_families = first_vm_page_for_families->next;
        }
        if (first_vm_page_for_families == NULL){
            return NULL;
        }
    }
}

static void mm_union_free_blocks(block_meta_data_t *first, block_meta_data_t *second){
    assert(first->is_free == MM_TRUE && second->is_free == MM_TRUE);
    first->block_size=+sizeof(second)+second->block_size;
    first->next = second->next;
    if(second){
        second->next->prev = first;
    }
}

vm_bool_t mm_isEmpty(vm_page_t *vm_page){
    if(vm_page->block_meta_data.next == NULL && vm_page->block_meta_data.prev == NULL && vm_page->block_meta_data.is_free==MM_TRUE){
        return MM_TRUE;
    }else{
        return MM_FALSE;
    }
}

static inline uint32_t mm_max_allocatable_memmory(int units){
    return (uint32_t)((SYSTEM_PAGE_SIZE * units)-offset_of(vm_page_t, page_memory));
}

vm_page_t * allocate_vm_page (vm_page_family_t *vm_page_family){
    vm_page_t *vm_page = get_new_VM_page(1);
    MARK_VM_PAGE_EMPTY(vm_page);
    vm_page->block_meta_data.block_size = mm_max_allocatable_memmory(1);
    vm_page->block_meta_data.offset = offset_of(vm_page_t, block_meta_data);
    init_glthread(&vm_page->block_meta_data.priority_glue,(unsigned int)offset_of(block_meta_data_t, priority_glue));
    vm_page->next = NULL;
    vm_page->prev = NULL;
    
    vm_page->pg_family = vm_page_family;

    if(!vm_page_family->first_page){
        vm_page_family->first_page = vm_page;
        return vm_page;
    }

    vm_page->next = vm_page_family->first_page;
    vm_page_family->first_page = vm_page;
    return vm_page;
}

void delete_vm_page_and_free(vm_page_t *vm_page){
    vm_page_family_t *vm_page_family = vm_page->pg_family;
    if(vm_page == vm_page_family->first_page){
        vm_page_family->first_page = vm_page->next;
        if(vm_page->next){
            vm_page->next->prev = NULL;
        }
        vm_page->next = NULL;
        vm_page->prev = NULL;
        return_VM_to_kernel((void *)vm_page,1);
    }

    if(vm_page->next){
        vm_page->next->prev = vm_page->prev;
    }
    vm_page->prev->next = vm_page->next;
    return_VM_to_kernel((void *)vm_page,1);
}

static int free_block_comparison_function(void *_block_meta_data1, void *_block_meta_data2){
    block_meta_data_t *block_meta_data1 = (block_meta_data_t *)_block_meta_data1;
    block_meta_data_t *block_meta_data2 = (block_meta_data_t *)_block_meta_data2;
    if(block_meta_data1->block_size > block_meta_data2->block_size){
        return -1;
    }
    else if(block_meta_data1->block_size < block_meta_data2->block_size){
        return 1;
    }
    return 0;
}

static void mm_add_metablock_priority_list(vm_page_family_t *vm_page_family, block_meta_data_t* free_block){

}