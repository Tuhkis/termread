#ifndef ARENA_H
#define ARENA_H

#define KILOBYTE (1024)

/* In essence the same as AppendBuffer in tr.c */
typedef struct
{
	size_t capacity;
	size_t pointer;
	void *data;
} Arena;

void arena_reset(Arena *arena);
void arena_init(Arena *arena, size_t size);
void *arena_alloc(Arena *arena, size_t size);
void arena_deinit(Arena *arena);

#endif /* ARENA_H */
