#include <stdio.h>
#include "coroutine.h"

coroutine *k1, *k2;

void bonnie()
{
    while (1) {
        printf("bonnie, sp=%p\n", (void *)k1->ctx.rsp);
        coroutine_resume(k1, k2, NULL);
    }
    //printf("bonnie 1\n");
    //coroutine_resume(k1, k2, NULL);
    //printf("bonnie 2\n");
    //coroutine_resume(k1, k2, NULL);
    //printf("bonnie 3\n");
    //coroutine_resume(k1, k2, NULL);
    //coroutine_resume(k1, k2, NULL);
    //coroutine_resume(k1, k2, NULL);
}


void clyde()
{
    int i = 1;
    while (1) {
        printf("clyde, sp=%p\n", (void *)k2->ctx.rsp);
        coroutine_yield(k2, NULL);
    }
    //printf("clyde 1\n");
    //coroutine_yield(k2, NULL);
    //printf("clyde 2\n");
    //coroutine_yield(k2, NULL);
    //printf("clyde 3\n");
    //coroutine_yield(k2, NULL);
    //printf("clyde 4\n");
}


int main()
{
    k1 = coroutine_create((coroutine_func)bonnie, NULL);
    k2 = coroutine_create((coroutine_func)clyde, NULL);

    coroutine_resume(NULL, k1, NULL);

    return 0;
}
