#include <stdio.h>
#include <stdlib.h>
#include "coroutine.h"

coroutine *__self = NULL;

static void *coroutine_run(coroutine *c);
static void coroutine_swap_ctx(coroutine_ctx *current, coroutine_ctx *target)
__attribute__((noinline));


coroutine *coroutine_create(coroutine_func func, void *arg)
{
    coroutine *c;

    c = calloc(1, sizeof(*c) + COROUTINE_STACK_SIZE);
    if (!c)
        return NULL;

    c->status = COROUTINE_SUSPENDED;
    c->stack = (char *)(c + 1);
    c->func = func;
    c->arg = arg;

#if defined(__i386__) || defined(__i686__)
    c->ctx.esp = (uint32_t)c->stack + COROUTINE_STACK_SIZE;
    c->ctx.eip = (uint32_t)coroutine_run;
#elif defined(__x86_64__)
    c->ctx.rsp = (uint64_t)c->stack + COROUTINE_STACK_SIZE;
    c->ctx.rdi = (uint64_t)c;
    c->ctx.rip = (uint64_t)coroutine_run;
#else
#error "Do not support this platform."
#endif

    return c;
}


static void coroutine_swap_ctx(coroutine_ctx *current, coroutine_ctx *target)
{
#if defined(__i386__) || defined(__i686__)
#elif defined(__x86_64__)
    __asm__ (
            // save
            "movq    %%rax, (%%rdi)\n\t" // rax
            "movq    %%rbx, 8(%%rdi)\n\t" // rbx
            "movq    %%rcx, 16(%%rdi)\n\t" // rcx
            "movq    %%rdx, 24(%%rdi)\n\t" // rdx
            "movq    %%rsp, 32(%%rdi)\n\t" // rsp
            "movq    (%%rsp), %%rax\n\t"
            "movq    %%rax, 40(%%rdi)\n\t" // rbp
            "movq    %%rsi, 48(%%rdi)\n\t" // rsi
            "movq    %%rdi, 56(%%rdi)\n\t" // rdi
            "movq    %%r8, 64(%%rdi)\n\t" // r8
            "movq    %%r9, 72(%%rdi)\n\t" // r9
            "movq    %%r10, 80(%%rdi)\n\t" // r10
            "movq    %%r11, 88(%%rdi)\n\t" // r11
            "movq    %%r12, 96(%%rdi)\n\t" // r12
            "movq    %%r13, 104(%%rdi)\n\t" // r13
            "movq    %%r14, 112(%%rdi)\n\t" // r14
            "movq    %%r15, 120(%%rdi)\n\t" // r15
            "movq    8(%%rsp), %%rax\n\t"
            "movq    %%rax, 128(%%rdi)\n\t" // rip
            // set
            "movq    8(%%rsi), %%rbx\n\t"  // rbx
            "movq    16(%%rsi), %%rcx\n\t"  // rcx
            "movq    24(%%rsi), %%rdx\n\t"  // rdx
            "movq    32(%%rsi), %%rsp\n\t"  // rsp
            "movq    40(%%rsi), %%rax\n\t"
            "movq    %%rax, (%%rsp)\n\t"  // rbp
            "movq    %%rsp, %%rbp\n\t"
            "movq    56(%%rsi), %%rdi\n\t"  // rdi
            "movq    64(%%rsi), %%r8\n\t"   // r8
            "movq    72(%%rsi), %%r9\n\t"   // r9
            "movq    80(%%rsi), %%r10\n\t"  // r10
            "movq    88(%%rsi), %%r10\n\t"  // r11
            "movq    96(%%rsi), %%r10\n\t"  // r12
            "movq    104(%%rsi), %%r13\n\t"  // r13
            "movq    112(%%rsi), %%r14\n\t"  // r14
            "movq    120(%%rsi), %%r15\n\t"  // r15
            "movq    128(%%rsi), %%rax\n\t"
            "movq    %%rax, 8(%%rsp)\n\t" // rip
            "movw    144(%%rsi), %%ax\n\t"
            "movq    (%%rsi), %%rax\n\t" // rax
            "movq    48(%%rsi), %%rsi\n\t" // rsi
            :::
            );
#else
#error "Do not support this platform."
#endif
}


void *coroutine_yield(coroutine *c, void *retval)
{
    c->retval = retval;
    c->status = COROUTINE_SUSPENDED;

    coroutine_swap_ctx(&c->ctx, &c->from);

    return c->retval;
}


void *coroutine_resume(coroutine *c, void *retval)
{
    coroutine *saved_self;

    if (c->status != COROUTINE_SUSPENDED)
        return NULL;

    c->retval = retval;
    if (__self)
        __self->status = COROUTINE_NORMAL;
    saved_self = __self;
    __self = c;
    c->status = COROUTINE_RUNNING;

    coroutine_swap_ctx(&c->from, &c->ctx);

    __self = saved_self;
    if (__self)
        __self->status = COROUTINE_RUNNING;

    return c->retval;
}


static void *coroutine_run(coroutine *c)
{
    __self = c;
    c->status = COROUTINE_RUNNING;
    c->retval = c->func(c, c->arg);
    c->status = COROUTINE_DEAD;
    coroutine_swap_ctx(&c->ctx, &c->from);

    return NULL;
}
