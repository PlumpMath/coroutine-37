#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "coroutine.h"

static void *coroutine_entry(coroutine *k);
static void coroutine_swap_ctx(coroutine_ctx *current, coroutine_ctx *target)
__attribute__((noinline));


// TODO all coroutine functions func take two arguments:
// 1. the coroutine pointer, 2. void *arg
// arg 1 is to call yield and resume
// TODO how to save coroutine status without affect resume's
// prototype? i just need coroutine_resume(coroutine *k, void *retval);
coroutine *coroutine_create(coroutine_func func, void *arg)
{
    coroutine *k;

    k = calloc(1, sizeof(*k) + COROUTINE_STACK_SIZE);
    if (!k)
        return NULL;

    k->status = COROUTINE_SUSPENDED;
    k->stack = (char *)(k + 1);
    k->func = func;
    k->arg = arg;

#if defined(__i386__) || defined(__i686__)
#elif defined(__x86_64__)
    k->ctx.rsp = (uint64_t)k->stack + COROUTINE_STACK_SIZE;
    k->ctx.rdi = (uint64_t)k;
    k->ctx.rip = (uint64_t)coroutine_entry;
#else
#error "Do not support this platform."
#endif

    return k;
}


// TODO make sure that this begins with push rbp, mov rsp, rbp,
// never compile with -fomit-frame-pointer !!!
// define it with pure assembly ?
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
            /*
               save segment registers ? if yes how about cs ?
            */
            // set, fake the stack frame and return
            "movq    8(%%rsi), %%rbx\n\t"  // rbx
            "movq    16(%%rsi), %%rcx\n\t"  // rcx
            "movq    24(%%rsi), %%rdx\n\t"  // rdx
            "movq    32(%%rsi), %%rsp\n\t"  // rsp, new stack
            "movq    40(%%rsi), %%rax\n\t"
            "movq    %%rax, (%%rsp)\n\t"  // rbp
            "movq    %%rsp, %%rbp\n\t" // fake it
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
            /*
            */
            "movq    (%%rsi), %%rax\n\t" // rax
            "movq    48(%%rsi), %%rsi\n\t" // rsi

            // swap
            //"jmpq    *%%rax\n\t"
            :::
            );
#else
#error "Do not support this platform."
#endif
}


void *coroutine_yield(coroutine *k, void *retval)
{
    k->retval = retval;
    k->status = COROUTINE_SUSPENDED;

    coroutine_swap_ctx(&k->ctx, &k->from);

    return k->retval;
}


void *coroutine_resume(coroutine *current, coroutine *target, void *retval)
{
    if (target->status != COROUTINE_SUSPENDED)
        return NULL;

    target->retval = retval;
    if (current)
        current->status = COROUTINE_NORMAL;
    target->status = COROUTINE_RUNNING;

    coroutine_swap_ctx(&target->from, &target->ctx);

    if (current)
        current->status = COROUTINE_RUNNING;

    return target->retval;
}


static void *coroutine_entry(coroutine *k)
{
    k->status = COROUTINE_RUNNING;
    k->retval = k->func(k->arg);
    k->status = COROUTINE_DEAD;
    coroutine_swap_ctx(&k->ctx, &k->from);

    return NULL;
}
