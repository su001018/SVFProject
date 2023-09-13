#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char buf[7];
    read(0, buf, 7);
    char *ptr1 = malloc(8);
    char *ptr2 = malloc(8);
    if (buf[5] == 'e')
    {
        ptr2 = ptr1;
    }
    if (buf[3] == 's')
    {
        if (buf[1] == 'u')
        {
            free(ptr1);
        }
    }
    if (buf[4] == 'e')
        if (buf[2] == 'r')
            if (buf[0] == 'f')
                ptr2[0] = 'm';
    return 0;
}