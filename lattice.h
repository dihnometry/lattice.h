#include <stdlib.h>

#define ARENA_SIZE 1024*1024 // 1 MB

typedef struct {
    char *memory;
    size_t capacity;
    size_t position;
} Arena;

Arena arena_create(size_t size);
void arena_delete(Arena *arena);
void arena_clear(Arena *arena);

#define arena_push_array(arena, type, count) arena_push((arena), sizeof(type)*(count))
#define arena_push_struct(arena, type) arena_push((arena), sizeof(type))

#ifdef LATTICE_IMPLEMENTATION

#include <stdio.h>

Arena arena_create(size_t size)
{
    Arena arena;
    // TODO: check if malloc fails
    arena.memory = (char*)malloc(size);
    arena.capacity = size;
    arena.position = 0;
    return arena;
}

void arena_delete(Arena *arena)
{
    free(arena);
}

// TODO: Make capacity grow when reaching limit
void *arena_push(Arena *arena, size_t size)
{
    size_t padding = arena->position % size;
    arena->position += padding;

    if (arena->position + size > arena->capacity) {
        printf("ERROR: Arena capacity overflow.\n");
        exit(1);
    }

    void *block_start = arena->memory + arena->position;
    arena->position += size;
    return block_start;
}

void arena_clear(Arena *arena)
{
    arena->position = 0;
}

#endif // LATTICE_IMPLEMENTATION
