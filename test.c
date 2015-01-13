#include <stdio.h>
#include <assert.h>
#include "coroutine.h"

coroutine *k1, *k2;

void bonnie(coroutine *c)
{
    while (1) {
        printf("bonnie, sp=%p\n", (void *)c->ctx.rsp);
        coroutine_resume(k2, NULL);
        assert(__self == c);
    }
    //assert(__self == c);
    //printf("bonnie 1\n");
    //coroutine_resume(k2, NULL);
    //assert(__self == c);
    //printf("bonnie 2\n");
    //coroutine_resume(k2, NULL);
    //assert(__self == c);
    //printf("bonnie 3\n");
    //coroutine_resume(k2, NULL);
    //assert(__self == c);
    //coroutine_resume(k2, NULL);
    //assert(__self == c);
    //coroutine_resume(k2, NULL);
    //assert(__self == c);
}


void clyde(coroutine *c)
{
    int i = 1;
    while (1) {
        printf("clyde, sp=%p\n", (void *)c->ctx.rsp);
        coroutine_yield(c, NULL);
        assert(__self == c);
    }
    //assert(__self == c);
    //printf("clyde 1\n");
    //coroutine_yield(k2, NULL);
    //assert(__self == c);
    //printf("clyde 2\n");
    //coroutine_yield(k2, NULL);
    //assert(__self == c);
    //printf("clyde 3\n");
    //coroutine_yield(k2, NULL);
    //assert(__self == c);
    //printf("clyde 4\n");
}


int main()
{
    k1 = coroutine_create((coroutine_func)bonnie, NULL);
    k2 = coroutine_create((coroutine_func)clyde, NULL);

    coroutine_resume(k1, NULL);

    return 0;
}
