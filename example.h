#include <stdio.h>
#include <stdlib.h>
#define dealloc(ptr)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        free(ptr);                                                                                                     \
        ptr = NULL;                                                                                                    \
    } while (0)

void printSomething()
{
    printf("printSomething...\n");
}