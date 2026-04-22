#include <stdlib.h>

#define ARENA_SIZE 1024*1024 // 1 MB
#define SB_INIT_CAPACITY 16

#define GROWTH_FACTOR 2

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

String_View sv_from_cstr(const char *data);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);

#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).count, (sv).data

#define LATTICE_IMPLEMENTATION
#ifdef LATTICE_IMPLEMENTATION

#include <stdio.h>
#include <ctype.h>
#include <string.h>

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
    free(arena->memory);
}

// TODO: Make capacity grow when reaching limit
void *arena_push(Arena *arena, size_t size)
{
    size_t align = sizeof(void*);
    size_t padding = (align - (arena->position % align)) % align;
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

// Grow the capacity of sb until it can fit n
void sb_grow_until(String_Builder *sb, size_t n)
{
    if (sb->count + n > sb->capacity) {
        size_t new_capacity = sb->capacity ? sb->capacity : SB_INIT_CAPACITY;
        while (sb->count + n > new_capacity) {
            // TODO: check for possible overflow
            new_capacity *= GROWTH_FACTOR;
        }

        // TODO: check if realloc fails
        char *new_content = (char*)realloc(sb->content, new_capacity);
        sb->content = new_content;
        sb->capacity = new_capacity;
    }
}

void sb_append(String_Builder *sb, char *s)
{
    if (sb->capacity == 0) {
        sb->capacity = SB_INIT_CAPACITY;
        // TODO: check if malloc fails
        sb->content = (char*)malloc(sb->capacity);
    }

    size_t len = strlen(s);
    sb_grow_until(sb, len);

    memcpy(&sb->content[sb->count], s, len);
    sb->count += len;
    sb->content[sb->count] = '\0';
}

void sb_insert(String_Builder *sb, size_t index, char *s)
{
    if (index >= sb->count) {
        printf("ERROR: Index out of bounds.");
        exit(1);
    }

    size_t len = strlen(s);
    if (len + sb->count > sb->capacity) sb_grow_until(sb, len);

    memmove(&sb->content[index + len], &sb->content[index], sb->count - index);
    memcpy(&sb->content[index], s, len);

    sb->count += len;
}

void sb_delete(String_Builder *sb, size_t start, size_t end)
{
    if (start >= sb->count || end >= sb->count) {
        printf("ERROR: Index out of bounds.");
        exit(1);
    }

    // I don't think this error is important enough to crash the program
    // Maybe decide later
    if (start > end) return;

    if (end == sb->count - 1) {
        sb->count = start;
        sb->content[sb->count] = '\0';
    } else {
        memmove(&sb->content[start], &sb->content[end + 1], sb->count - end - 2);
        sb->count -= end - start + 1;
        sb->content[sb->count] = '\0';
    }
}

void sb_replace(String_Builder *sb, size_t start, size_t end, char *s)
{
    if (start >= sb->count || end >= sb->count) {
        printf("ERROR: Index out of bounds.");
        exit(1);
    }

    // I don't think this error is important enough to crash the program
    // Maybe decide later
    if (start > end) return;

    size_t len = strlen(s);
    size_t new_count = sb->count + len - (end - start + 1);
    if (new_count > sb->capacity) {
        sb_grow_until(sb, new_count - sb->count);
    }


    if (end == sb->count - 1) {
        memcpy(&sb->content[start], s, len);
        sb->content[new_count] = '\0';
    } else {
        memmove(&sb->content[start + len], &sb->content[end + 1], new_count - len);
        memmove(&sb->content[start], s, len);
        sb->content[new_count] = '\0';
    }

    sb->count = new_count;
}

// --------------------------------------
// |     String View Implementation     |
// --------------------------------------

String_View sv_from_cstr(const char *data)
{
    String_View sv = {0};
    sv.data = data;
    sv.count = strlen(data);
    return sv;
}

String_View sv_from_sb(String_Builder *sb)
{
    return (String_View) {
        .data = sb->content,
        .count = sb->count
    };
}

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

    for (size_t i = sv.count; i > 0; --i) {
        if (!isspace(sv.data[i - 1])) break;
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
