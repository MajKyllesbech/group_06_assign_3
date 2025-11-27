/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 *
 */

#include <stdint.h>

#include "mm.h"


/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
/* TODO: Mask out free flag - FIXED: Added outer parentheses to enforce cast precedence */
#define GET_NEXT(p)    ((BlockHeader *)((uintptr_t)((p)->next) & ~(uintptr_t)0x1))

/* TODO: Preserve free flag */
#define SET_NEXT(p,n)  ((p)->next = (BlockHeader *)(((uintptr_t)(n)) | GET_FREE(p)))

/* OK -- do not change */
#define GET_FREE(p)    ((uint8_t) ( (uintptr_t) ((p)->next) & 0x1 ))

/* TODO: Set free bit of p->next to f */
#define SET_FREE(p,f)  ((p)->next = (BlockHeader *)(((uintptr_t)GET_NEXT(p)) | ((f) & 0x1)))

/* TODO: Calculate size of block from p and p->next - FIXED: Removed undefined 'f' */
/* Correct SIZE macro: Distance to next block MINUS the header size */
#define SIZE(p) ((size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p) - sizeof(BlockHeader)))

#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user


static BlockHeader * first = NULL;
static BlockHeader * current = NULL;


/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
  uintptr_t aligned_memory_start = (memory_start+7) & ~(uintptr_t)7; //round up 8-bit aligned
  uintptr_t aligned_memory_end   = memory_end & ~(uintptr_t)7;  //round down 8-bit aligned
  BlockHeader * last;

  /* Already initalized ? */
  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      first = (BlockHeader *) aligned_memory_start; //place first

      // FIXED: Removed 'BlockHeader *' to avoid shadowing the variable declared at the top
      last = (BlockHeader *) (aligned_memory_end - sizeof(BlockHeader));

      SET_NEXT(first, last); //link first -> last + set free flag.
      SET_FREE(first, 1);

      SET_NEXT(last, first);
      SET_FREE(last, 0);
    }
    current = first;
  }
}

/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation.
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }

  // Ensure size is at least MIN_SIZE and aligned
  if (size < MIN_SIZE) size = MIN_SIZE;
  size_t aligned_size = (size + 7) & ~(size_t)7;

  BlockHeader * search_start = current;

  do {
    if (GET_FREE(current)) {
      // 1. Coalesce consecutive free blocks
      while (GET_FREE(GET_NEXT(current)) && GET_NEXT(current) != first) {
        BlockHeader *next = GET_NEXT(current);
        BlockHeader *next_next = GET_NEXT(next);

        // Safety check to prevent infinite loops if next pointers are corrupted
        if (next == current) break;

        SET_NEXT(current, next_next);
        SET_FREE(current, 1);
      }

      // 2. Calculate TOTAL size needed (Header + User Data)
      size_t needed_total = sizeof(BlockHeader) + aligned_size;

      // 3. Check if current block is big enough
      if (SIZE(current) >= aligned_size) { // SIZE() already excludes header of current block

        // Check if we can split
        if (SIZE(current) >= aligned_size + sizeof(BlockHeader) + MIN_SIZE) {
          // SPLIT
          uintptr_t block_start = (uintptr_t)current + sizeof(BlockHeader);
          BlockHeader * new_free = (BlockHeader *)(block_start + aligned_size);

          SET_NEXT(new_free, GET_NEXT(current));
          SET_FREE(new_free, 1);

          SET_NEXT(current, new_free);
          SET_FREE(current, 0); // Allocated

          current = new_free; // Next search starts here
          return (void *)block_start;
        } else {
          // NO SPLIT - Take whole block
          SET_FREE(current, 0);
          void * block = (void *)((uintptr_t)current + sizeof(BlockHeader));
          current = GET_NEXT(current);
          return block;
        }
      }
    }
    current = GET_NEXT(current);
  } while (current != search_start);

  return NULL;
}



/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation.
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
 /* Find block corresponding to ptr */
  // FIXED: Removed double semicolon
  BlockHeader * block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

  if (GET_FREE(block)) {
    /* Block is not in use -- probably an error */
    return;
  }

  /* Free block */
  SET_FREE(block, 1);

  /* Possibly coalesce consecutive free blocks here */

    // coalesce forward
  while (GET_FREE(GET_NEXT(block))) {
    SET_NEXT(block, GET_NEXT(GET_NEXT(block)));
    SET_FREE(block, 1);
  }

  // coalesce backward
  BlockHeader * prev = block;
  while (GET_NEXT(prev) != block) {
    prev = GET_NEXT(prev);
  }
  if (GET_FREE(prev)) {
    SET_NEXT(prev, GET_NEXT(block));
    SET_FREE(prev, 1);
    block = prev;

    while (GET_FREE(GET_NEXT(block))) {
      SET_NEXT(block, GET_NEXT(GET_NEXT(block)));
      SET_FREE(block, 1);
    }
  }
}

// NOTE: I removed the test case from here as instructed.
// It is now located in check_mm.c

#include "mm_aux.c"