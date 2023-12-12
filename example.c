#include "example.h"
struct Data
{
    int value;
    void (*callback)();
};

void doSomething(struct Data *data)
{
    printf("Doing something... Value: %d\n", data->value);
    data->callback();
}

void cleanup(struct Data *data)
{
    dealloc(data);
    printf("Cleaning up...\n");
}

void callbackFunction()
{
    printf("Callback function called!\n");
}

int main()
{
    struct Data *data = (struct Data *)malloc(sizeof(struct Data));
    data->value = 42;
    data->callback = callbackFunction;

    if (rand() > 5)
    {
        cleanup(data);
        printSomething();
        return 0;
    }

    printSomething();
    doSomething(data);

    return 0;
}