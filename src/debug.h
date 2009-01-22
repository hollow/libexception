#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef CONFIG_DEBUG
#include <stdio.h>
#define debug(...) do { \
	printf("[debug] %s:%d in %s(): ", __FILE__, __LINE__, __FUNCTION__); \
	printf(__VA_ARGS__); \
	printf("\n"); \
} while (0)
#else
#define debug(...)
#endif

#endif
