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

typedef struct {
    char *content;
    size_t count;
    size_t capacity;
} String_Builder;

void sb_append(String_Builder *sb, char *s);
void sb_insert(String_Builder *sb, size_t index, char *s);
void sb_delete(String_Builder *sb, size_t start, size_t end);
void sb_replace(String_Builder *sb, size_t start, size_t end, char *s);

typedef struct {
    const char *data;
    size_t count;
} String_View;

String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);

#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).count, (sv).data

#ifdef LATTICE_IMPLEMENTATION

#include <stdio.h>
#include <ctype.h>

// ------------------------------------------
// |     Arena Allocator Implementation     |
// ------------------------------------------

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

// Maybe delete this function? One can always do `arena->position = 0`
// to restart the arena, and it's the same amount of code.
void arena_clear(Arena *arena)
{
    arena->position = 0;
}

// -----------------------------------------
// |     String Builder Implementation     |
// -----------------------------------------

void sb_append(String_Builder *sb, char *s)
{
    return;
}

void sb_insert(String_Builder *sb, size_t index, char *s)
{
    return;
}

void sb_delete(String_Builder *sb, size_t start, size_t end)
{
    return;
}

void sb_replace(String_Builder *sb, size_t start, size_t end, char *s)
{
    return;
}

// --------------------------------------
// |     String View Implementation     |
// --------------------------------------

String_View sv_trim_left(String_View sv)
{
    size_t count = 0;
    for (size_t i = 0; i < sv.count; ++i) {
        if (!isspace(sv.data[i])) break;
        count++;
    }

    String_View new_sv = { .data = sv.data + count, .count = sv.count - count };
    return new_sv;
}

String_View sv_trim_right(String_View sv)
{
    size_t count = 0;

    for (size_t i = sv.count - 1; i >= 0; --i) {
        if (!isspace(sv.data[i])) break;
        count++;
    }

    String_View new_sv = { .data = sv.data, .count = sv.count - count };
    return new_sv;
}

String_View sv_trim(String_View sv)
{
    String_View a = sv_trim_left(sv);
    String_View b = sv_trim_right(a);
    return b;
}

String_View sv_chop_by_delim(String_View *sv, char delim)
{
    size_t count = 0;
    for (size_t i = 0; i < sv->count; ++i) {
        if (sv->data[i] == delim) break;
        count++;
    }

    String_View s = {0};
    s.data = sv->data;
    s.count = count;

    if (count >= sv->count) {
        sv->count = 0;
    } else {
        sv->data += count;
        sv->count -= count;
    }

    return s;
}

#endif // LATTICE_IMPLEMENTATION
