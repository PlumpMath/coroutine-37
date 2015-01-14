#include <stdio.h>
#include <stdlib.h>
#include "coroutine.h"

coroutine *__self = NULL;

static void *coroutine_run(coroutine *c);

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
    /*
     * the target stack layout is:
     * 
     *  high address                                            low address
     *    ---+-----------+---------------+------------+------------+---
     *   ... | pointer c | whatever %eip | saved %eip | saved %ebp | ...
     *       |           |               |            |            |
     *    ---+-----------+---------------+------------+------------+---
     *                                                  ^
     *                                                  |___ %ebp, %esp
     *                                                      in coroutine_swap_ctx
     */
    c->ctx.esp = (uint32_t)c->stack + COROUTINE_STACK_SIZE - sizeof(uint32_t);
    *((coroutine **)c->ctx.esp) = c;
    c->ctx.esp -= 3 * sizeof(uint32_t);
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


void coroutine_destroy(coroutine *c)
{
    free(c);
}


void coroutine_swap_ctx(coroutine_ctx *current, coroutine_ctx *target) __attribute__((noinline));
#if defined(__i386__) || defined(__i686__)
    __asm__ (
            ".text\n\t"
            ".globl coroutine_swap_ctx\n\t"
            "coroutine_swap_ctx:\n\t"
            // prolog
            "pushl    %ebp\n\t"
            "movl     %esp, %ebp\n\t"
            // preserve these two
            "pushl    %edi\n\t"
            "pushl    %esi\n\t"
            "movl     8(%ebp), %edi\n\t" // current
            "movl     12(%ebp), %esi\n\t" // target
            // save
            "movl     %eax, (%edi)\n\t" // eax
            "movl     %ebx, 4(%edi)\n\t" // ebx
            "movl     %ecx, 8(%edi)\n\t" // ecx
            "movl     %edx, 12(%edi)\n\t" // edx
            "leal     8(%esp), %eax\n\t"
            "movl     %eax, 16(%edi)\n\t" // esp
            "movl     (%ebp), %eax\n\t"
            "movl     %eax, 20(%edi)\n\t" // ebp
            "popl     %eax\n\t"
            "movl     %eax, 24(%edi)\n\t" // esi
            "popl     %eax\n\t"
            "movl     %eax, 28(%edi)\n\t" // edi
            "movl     4(%ebp), %eax\n\t"
            "movl     %eax, 32(%edi)\n\t" // eip
            // set
            "movl     4(%esi), %ebx\n\t" // ebx
            "movl     8(%esi), %ecx\n\t" // ecx
            "movl     12(%esi), %edx\n\t" // edx
            "movl     16(%esi), %esp\n\t" // esp
            "movl     20(%esi), %eax\n\t"
            "movl     %eax, (%esp)\n\t" // ebp
            "movl     %esp, %ebp\n\t"
            "movl     28(%esi), %edi\n\t" // edi
            "movl     32(%esi), %eax\n\t"
            "movl     %eax, 4(%esp)\n\t" // eip
            "movl     (%esi), %eax\n\t" // eax
            "movl     24(%esi), %esi\n\t" // esi
            // epilog
            "popl     %ebp\n\t"
            "ret\n\t"
            );
#elif defined(__x86_64__)
    __asm__ (
            ".text\n\t"
            ".globl coroutine_swap_ctx\n\t"
            "coroutine_swap_ctx:\n\t"
            // prolog
            "pushq   %rbp\n\t"
            "movq    %rsp, %rbp\n\t"
            // save
            "movq    %rax, (%rdi)\n\t" // rax
            "movq    %rbx, 8(%rdi)\n\t" // rbx
            "movq    %rcx, 16(%rdi)\n\t" // rcx
            "movq    %rdx, 24(%rdi)\n\t" // rdx
            "movq    %rsp, 32(%rdi)\n\t" // rsp
            "movq    (%rbp), %rax\n\t"
            "movq    %rax, 40(%rdi)\n\t" // rbp
            "movq    %rsi, 48(%rdi)\n\t" // rsi
            "movq    %rdi, 56(%rdi)\n\t" // rdi
            "movq    %r8, 64(%rdi)\n\t" // r8
            "movq    %r9, 72(%rdi)\n\t" // r9
            "movq    %r10, 80(%rdi)\n\t" // r10
            "movq    %r11, 88(%rdi)\n\t" // r11
            "movq    %r12, 96(%rdi)\n\t" // r12
            "movq    %r13, 104(%rdi)\n\t" // r13
            "movq    %r14, 112(%rdi)\n\t" // r14
            "movq    %r15, 120(%rdi)\n\t" // r15
            "movq    8(%rbp), %rax\n\t"
            "movq    %rax, 128(%rdi)\n\t" // rip
            // set
            "movq    8(%rsi), %rbx\n\t"  // rbx
            "movq    16(%rsi), %rcx\n\t"  // rcx
            "movq    24(%rsi), %rdx\n\t"  // rdx
            "movq    32(%rsi), %rsp\n\t"  // rsp
            "movq    40(%rsi), %rax\n\t"
            "movq    %rax, (%rsp)\n\t"  // rbp
            "movq    %rsp, %rbp\n\t"
            "movq    56(%rsi), %rdi\n\t"  // rdi
            "movq    64(%rsi), %r8\n\t"   // r8
            "movq    72(%rsi), %r9\n\t"   // r9
            "movq    80(%rsi), %r10\n\t"  // r10
            "movq    88(%rsi), %r10\n\t"  // r11
            "movq    96(%rsi), %r10\n\t"  // r12
            "movq    104(%rsi), %r13\n\t"  // r13
            "movq    112(%rsi), %r14\n\t"  // r14
            "movq    120(%rsi), %r15\n\t"  // r15
            "movq    128(%rsi), %rax\n\t"
            "movq    %rax, 8(%rsp)\n\t" // rip
            "movq    (%rsi), %rax\n\t" // rax
            "movq    48(%rsi), %rsi\n\t" // rsi
            // epilog
            "popq     %rbp\n\t"
            "ret\n\t"
            );
#else
#error "Do not support this platform."
#endif


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
