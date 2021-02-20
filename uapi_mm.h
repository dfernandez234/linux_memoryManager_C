#ifndef __UAPI_MM__
#define __UAPI_MM__
#include <stdint.h>
#include "mm.h"

//Initialize functions
void mm_init();
//Registration function
void mm_instantiate_new_page_family(char *struct_name, uint32_t struct_size);
//print function
void mm_print_registeredFamilies();
//lookup function
vm_page_family_t * lookup_page_family_by_name (char *struct_name);


#define MM_REG_STRUCT(struct_name)\
    (mm_instantiate_new_page_family(#struct_name, sizeof(struct_name)))

#define MM_PRINT()\
    (mm_print_registeredFamilies())

#define MM_SEARCH(struct_name)\
    (lookup_page_family_by_name (#struct_name))

#endif /*__UAPI_MM__*/