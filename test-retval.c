#include <stdio.h>
#include <assert.h>
#include "coroutine.h"

coroutine *k1, *k2;

void bonnie(coroutine *c)
{
    void *ret;

    printf("bonnie 1\n");
    ret = coroutine_resume(k2, NULL);
    printf("bonnie 2: ret value from resume is %ld\n", (long)ret);
    coroutine_resume(k2, (void *)(long)2);
}


void clyde(coroutine *c)
{
    void *ret;

    printf("clyde 1\n");
    ret = coroutine_yield(c, (void *)(long)1);
    printf("clyde 2: ret value from yield is: %ld\n", (long)ret);
}


int main()
{
    k1 = coroutine_create((coroutine_func)bonnie, NULL);
    k2 = coroutine_create((coroutine_func)clyde, NULL);

    coroutine_resume(k1, NULL);

    return 0;
}
