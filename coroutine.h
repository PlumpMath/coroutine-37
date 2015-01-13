#ifndef __COROUTINE_H__
#define __COROUTINE_H__

#include <stdint.h>

#define COROUTINE_STACK_SIZE 8192

typedef struct coroutine_t coroutine;

typedef void *(*coroutine_func)(coroutine *k, void *arg);

enum {
    COROUTINE_SUSPENDED,
    COROUTINE_RUNNING,
    COROUTINE_NORMAL,
    COROUTINE_DEAD,
};

typedef struct coroutine_ctx_t {
#if defined(__i386__) || defined(__i686__)
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t eip;
#elif defined(__x86_64__)
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
#else
#error "Do not support his platform" 
#endif
} coroutine_ctx;

struct coroutine_t {
    int status;
    char *stack;
    coroutine_func func;
    void *arg;
    void *retval;
    coroutine_ctx from;
    coroutine_ctx ctx;
};

extern coroutine *__self;

coroutine *coroutine_create(coroutine_func func, void *arg);
void *coroutine_yield(coroutine *c, void *retval);
void *coroutine_resume(coroutine *target, void *retval);

#endif
