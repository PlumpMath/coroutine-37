#include <stdio.h>
#include <assert.h>
#include "coroutine.h"

coroutine *k1, *k2, *k3;

void what(coroutine *c)
{
    assert(__self == c);
    printf("what 1\n");
    coroutine_yield(c, NULL);
}

void bonnie(coroutine *c)
{
    assert(__self == c);
    printf("bonnie 1\n");
    coroutine_resume(k2, NULL);
    assert(__self == c);
    printf("bonnie 2\n");
    coroutine_resume(k2, NULL);
    assert(__self == c);
    printf("bonnie 3\n");
    coroutine_resume(k2, NULL);
    assert(__self == c);
    coroutine_resume(k2, NULL);
    assert(__self == c);
    coroutine_resume(k2, NULL);
    assert(__self == c);
}


void clyde(coroutine *c)
{
    assert(__self == c);
    printf("clyde 1\n");
    coroutine_yield(k2, NULL);
    assert(__self == c);
    printf("clyde 2\n");
    coroutine_yield(k2, NULL);
    assert(__self == c);
    printf("clyde 3\n");
    coroutine_yield(k2, NULL);
    assert(__self == c);
    printf("clyde 4\n");
    assert(__self == c);
    coroutine_resume(k3, NULL);
    assert(__self == c);
    printf("clyde 5\n");
}


int main()
{
    k1 = coroutine_create((coroutine_func)bonnie, NULL);
    k2 = coroutine_create((coroutine_func)clyde, NULL);
    k3 = coroutine_create((coroutine_func)what, NULL);

    coroutine_resume(k1, NULL);

    return 0;
}
