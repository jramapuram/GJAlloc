#include "gjalloc.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define N 64

struct foo {
    unsigned long long n;
    int free;
    uint64_t trigger;
    uint64_t padding[4];
};

struct foo ** list;

void relocate_simple(void * start, void * stop, ptrdiff_t diff) {
    struct foo * n = (struct foo*)start;
    
    fprintf(stderr, "relocate_simple(%p, %p, %llu)\n", start, stop, (unsigned long long)diff);
    fprintf(stderr, "[%llu ... %llu] \n", (unsigned long long)n->n, (unsigned long long)((struct foo *)stop - 1)->n);
    while (n < (struct foo*)stop) {
	ba_simple_rel_pointer(list + n->n, diff);
	n++;
    }
}

void callback(struct foo * start, struct foo * stop, void * data) {
    fprintf(stderr, "callback walking over [%llu..%llu]\n", start->n, (stop-1)->n);

    while (start < stop) {
//	fprintf(stderr, "is there: %llu\n", (unsigned long long)start->n);
	if (start->free) {
	    fprintf(stderr, "walking over free block %llu\n", (unsigned long long) start->n);
	}
	start->trigger = 1;	
	start++;
    }
}

TEST_INIT(foo);

int main() {
    size_t i;

    list = (struct foo**)malloc(N*sizeof(struct foo));
    DYNAMIC_INIT(foo);

    for (i = 0; i < N; i++) {
	list[i] = TEST_ALLOC(foo);
	list[i]->free = 0;
	list[i]->n = i;
#ifdef TEST_FREE
	if (i & 1) {
	    list[i-1]->free = 1;
	    TEST_FREE(foo, list[i-1]);
	}
#endif
    }

#ifdef TEST_FREE
    list[5]->free = 1;
    list[7]->free = 1;
    TEST_FREE(&a, list[5]);
    TEST_FREE(&a, list[7]);
#endif
    TEST_WALK(&a, callback, NULL);

    TEST_DEINIT(foo);
    free(list);

    return 0;
}
