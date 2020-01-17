/*
 * src/include/utils/algos.h
 *
 * Copyright (c) Zhao Wei (david.zhao.cn@gmail.com)
 *
 *
 * Various algorithm functions.
 */

#ifndef ALGOS_H
#define ALGOS_H

typedef int (*func_cmp_t)(const void *elem1, const void *elem2);
/*
 * Do binary search to find 'key' from [base, base + num_elems), each element's
 * size is elesz bytes. Use cmpf as comparison function. If key not found, *inspos
 * takes back the index of the element right before which to insert the key.
 * return the found 'key' element's index in the array, or -1 if not found.
 * return -2 if invalid arguments.
 * */
int bin_search(const void *key, const void *base, size_t num_elems,
    size_t elesz, func_cmp_t cmpf, int *inspos);


#endif
