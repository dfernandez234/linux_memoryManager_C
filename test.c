#include "uapi_mm.h"

typedef struct emp_{
    char name [35];
    uint32_t emp_id;
}emp_t;

typedef struct student_{
    char name[35];
    uint32_t rollno;
    uint32_t marks_phys;
    uint32_t marks_chems;
    uint32_t marks_maths;
    struct student_ *next;
} student_t;


int main(int argc, char const *argv[])
{
    mm_init();
    MM_REG_STRUCT(emp_t);
    MM_REG_STRUCT(student_t);
    MM_PRINT();
    
    XCALLOC(emp_t, 1);
    XCALLOC(emp_t, 1);
    XCALLOC(emp_t, 1);

    XCALLOC(student_t, 1);
    XCALLOC(student_t, 1);

    return 0;
}
