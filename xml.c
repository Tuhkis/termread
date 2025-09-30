/**
 * The worst XML parser ever for tr.
 */

#include <stdlib.h>

#include "arena.h"
#include "xml.h"

static void xml_print_children_recursive(XMLNode *node, int level);
static void xml_print_children_recursive(XMLNode *node, int level)
{
	int i = 0;
	XMLNode *iterator = node->first_child;

	for (i = 0; i < level; ++i) putc(' ', stdout);
	printf("%s", node->tag);
	if (node->first_attributes != NULL)
		printf(": %s\n", node->first_attributes->key);
	else
		printf("\n");

	if (node->first_child == NULL) return;
	while (iterator != NULL)
	{
		xml_print_children_recursive(iterator, level + 1);
		iterator = iterator->next_sibling;
	}
}

void xml_test(void)
{
	FILE *fp = fopen("test.opf", "r");
	size_t fsz;
	char *test_xhtml = NULL;
	Arena arena;

	fseek(fp, 0L, SEEK_END);
	fsz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	test_xhtml = malloc(fsz);
	fread(test_xhtml, 1, fsz, fp);

	arena_init(&arena, KILOBYTE * 16);

	XMLNode *root = xml_parse_string(&arena, test_xhtml);

	xml_print_children_recursive(root, 0);

	arena_deinit(&arena);
}

/* WARN: NOT CONFORMANT AT ALL */
/* Linked lists go brr.        */
XMLNode *xml_parse_string(Arena *a, char *string)
{
	XMLNode *root;
	XMLNode *top;

	root = arena_alloc(a, sizeof(XMLNode));
	memset(root, 0, sizeof(XMLNode));
	root->tag = arena_alloc(a, 8);
	memcpy(root->tag, "root", 5);

	top = root;

	char *walk = string;
	while (*walk != 0)
	{
		if (*walk == '/' && *(walk + 1) == '>')
		{
			top->first_child = NULL;
			top = top->parent;
			if (top == NULL)
				break;
		} else if (*walk == '<' && !(*(walk + 1) == '!' && *(walk + 2) == '-' && *(walk + 3) == '-'))
		{
			if (*(walk + 1) == '/')
			{
				top = top->parent;
				if (top == NULL)
					break;
			} else
			{
				XMLNode *new_node = arena_alloc(a, sizeof(XMLNode));
				char *tag_name;
				char *tag_end = walk;
				size_t tag_length = 1;

				memset(new_node, 0, sizeof(XMLNode));
				new_node->parent = top;

				while (*(tag_end++) != ' ' && *tag_end != '>' && *tag_end != '/') tag_length++;
				tag_name = arena_alloc(a, tag_length);
				memcpy(tag_name, walk + 1, tag_length - 1);

				/* printf("Tag name: %s & Tag end: %c\n", tag_name, *(tag_end)); */
				/* Has at least one attribute. */
				if (*tag_end != '>' && *tag_end != '/')
				{
					XMLAttribute *attribute = arena_alloc(a, sizeof(XMLAttribute));
					char *attribute_start = tag_end;
					char *attribute_end = attribute_start;
					size_t attribute_length = 1;

					while (*(attribute_end++) != ' ' && *attribute_end != '>' && *attribute_end != '/' &&
						*attribute_end != '=' && *attribute_end != '?') attribute_length++;

					if (*attribute_end == '=')
					{
						if (*(attribute_end + 1) == '\"') attribute->type = XML_ATTRIBUTE_STRING;
						else if (*(attribute_end + 1) >= '0' && *(attribute_end + 1) <= '9') attribute->type = XML_ATTRIBUTE_NUMBER;
						else attribute->type = XML_ATTRIBUTE_UNKNOWN;
					} else
					{
						attribute->type = XML_ATTRIBUTE_NO_VALUE;
						attribute->value = NULL;
					}

					attribute->key = arena_alloc(a, attribute_length + 1);
					memcpy(attribute->key, attribute_start, attribute_length);

					new_node->first_attributes = attribute;

					printf("Len: %ld & Key: %s\n & is string: %d\n", attribute_length, attribute->key, attribute->type);
				}

				new_node->tag = tag_name;

				if (top->first_child == NULL)
				{
					top->first_child = new_node;
				} else
				{
					XMLNode *iterator = top->first_child;

					while (iterator->next_sibling != NULL) iterator = iterator->next_sibling;
					iterator->next_sibling = new_node;
				}

				top = new_node;
			}
		}
		walk++;
	}

	return root;
}
