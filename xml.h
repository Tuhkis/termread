#ifndef XML_H
#define XML_H

#include "arena.h"

typedef struct XMLNode XMLNode;
typedef struct XMLAttribute XMLAttribute;

typedef enum
{
	XML_ATTRIBUTE_NO_VALUE = 0,
	XML_ATTRIBUTE_STRING,
	XML_ATTRIBUTE_NUMBER,
	XML_ATTRIBUTE_UNKNOWN
} XMLAttributeType;

typedef struct XMLAttribute
{
	XMLAttribute *next_sibling;
	char *key;
	char *value;
	XMLAttributeType type;
} XMLAttribute;

typedef struct XMLNode
{
	XMLAttribute *first_attributes;
	XMLNode *parent;
	XMLNode *first_child;
	XMLNode *next_sibling;
	char *tag;
} XMLNode;

void xml_test(void);
XMLNode *xml_parse_string(Arena *a, char *string);

#endif /* XML_H */
