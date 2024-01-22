#include "example.h"
struct Data
{
    int value;
};

void doSomething(struct Data *data)
{
    printf("Doing something... Value: %d\n", data->value);
}

void cleanup(struct Data *data)
{
    dealloc(data);
    printf("Cleaning up...\n");
}

int main()
{
    struct Data *data = (struct Data *)malloc(sizeof(struct Data));
    struct Data *data2 = (struct Data *)malloc(sizeof(struct Data));
    doSomething(data);
    cleanup(data);
    doSomething(data2);
    return 0;
}
