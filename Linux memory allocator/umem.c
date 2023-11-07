#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "umem.h"

#define PAGE_SIZE 4096
#define ALIGNMENT 8

typedef struct block
{
    size_t size;
    struct block *next;
} block_t;

static block_t *free_list = NULL;
static block_t *next_fit_block = NULL;
static void *start_of_region = NULL;
static size_t size_of_region = 0;
static int allocation_algorithm = BEST_FIT;

static size_t align_size(size_t size)
{
    size_t aligned_size = size + ALIGNMENT - 1;
    return aligned_size - (aligned_size % ALIGNMENT);
}

int umeminit(size_t sizeOfRegion, int allocationAlgo)
{
    if (start_of_region != NULL || sizeOfRegion <= 0)
    {
        return -1;
    }

    size_t total_size = sizeOfRegion + (size_t)getpagesize() - 1;
    total_size &= ~((size_t)getpagesize() - 1);

    void *ptr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (ptr == MAP_FAILED)
    {
        return -1;
    }

    start_of_region = ptr;
    size_of_region = total_size;
    allocation_algorithm = allocationAlgo;

    block_t *initial_block = (block_t *)ptr;
    initial_block->size = total_size - sizeof(block_t);
    initial_block->next = NULL;
    free_list = initial_block;

    return 0;
}

void *umalloc(size_t size)
{
    size_t total_size;
    block_t *current_block, *prev_block, *best_block, *best_prev_block;
    void *result = NULL;

    if (size == 0)
    {
        return NULL;
    }

    total_size = align_size(size) + sizeof(block_t);

    switch (allocation_algorithm)
    {
    case BEST_FIT:
        best_block = NULL;
        best_prev_block = NULL;
        current_block = free_list;
        prev_block = NULL;

        while (current_block != NULL)
        {
            if (current_block->size >= total_size && (best_block == NULL || current_block->size < best_block->size))
            {
                best_block = current_block;
                best_prev_block = prev_block;
            }

            prev_block = current_block;
            current_block = current_block->next;
        }

        if (best_block != NULL)
        {
            if (best_block->size >= total_size + sizeof(block_t) + ALIGNMENT)
            {
                block_t *new_block = (block_t *)((void *)best_block + total_size);
                new_block->size = best_block->size - total_size;
                new_block->next = best_block->next;

                if (best_prev_block == NULL)
                {
                    free_list = new_block;
                }
                else
                {
                    best_prev_block->next = new_block;
                }

                best_block->size = total_size;
                best_block->next = NULL;
            }

            result = (void *)best_block + sizeof(block_t);
        }

        break;

    default:
        return NULL;
    }

    return result;
}

int ufree(void *ptr)
{
    if (ptr == NULL)
    {
        return 0;
    }

    block_t *freed_block = (block_t *)((void *)ptr - sizeof(block_t));
    block_t *current_block, *prev_block;

    prev_block = NULL;
    current_block = free_list;

    while (current_block != NULL && current_block < freed_block)
    {
        prev_block = current_block;
        current_block = current_block->next;
    }

    if (prev_block == NULL)
    {
        free_list = freed_block;
    }
    else
    {
        prev_block->next = freed_block;
    }

    freed_block->next = current_block;

    if (current_block != NULL && (void *)freed_block + freed_block->size == (void *)current_block)
    {
        freed_block->size += current_block->size;
        freed_block->next = current_block->next;
    }

    if (prev_block != NULL && (void *)prev_block + prev_block->size == (void *)freed_block)
    {
        prev_block->size += freed_block->size;
        prev_block->next = freed_block->next;
    }

    return 0;
}

void umemdump()
{
    block_t *current_block;

    printf("start_of_region = %p, size_of_region = %zu\n", start_of_region, size_of_region);

    for (current_block = free_list; current_block != NULL; current_block = current_block->next)
    {
        printf("Free block address: %p, size: %zu\n", current_block, current_block->size);
    }
}
