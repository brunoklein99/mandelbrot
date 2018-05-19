//
// Created by klein on 19/05/18.
//

#ifndef MANDELBROT_DYNARRAY_H
#define MANDELBROT_DYNARRAY_H

#include <glob.h>
#include <malloc.h>

typedef struct {
    void** array;
    size_t used;
    size_t size;
} DynArray;

void dynarray_init(DynArray* a, size_t size){
    a->array = malloc(size * sizeof(void*));
    a->used = 0;
    a->size = size;
}

void dynarray_insert(DynArray *a, void* element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(void*));
    }
    a->array[a->used++] = element;
}

void dynarray_free(DynArray *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

#endif //MANDELBROT_DYNARRAY_H
