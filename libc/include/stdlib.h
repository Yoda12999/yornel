#pragma once

#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);
void* malloc(size_t);

#ifdef __cplusplus
}
#endif

