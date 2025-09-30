#include "arena.h"

void arena_reset(Arena *arena)
{
	memset(arena->data, 0, arena->capacity);
	arena->pointer = 0;
}

void arena_init(Arena *arena, size_t size)
{
	arena->capacity = size;
	arena->data = malloc(arena->capacity);
	arena_reset(arena);
}

void *arena_alloc(Arena *arena, size_t size)
{
	while (arena->pointer + size >= arena->capacity)
	{
		/* arena->capacity *= 2;
		arena->data = realloc(arena->data, arena->capacity); */
		fprintf(stderr, "Arena shat the bed.\n");
		exit(1);
	}
	void *ret = arena->data + arena->pointer;
	arena->pointer += size;
	return ret;
}

void arena_deinit(Arena *arena)
{
	arena->capacity = 0;
	arena->pointer = -1;
	free(arena->data);
}
