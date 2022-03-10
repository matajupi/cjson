#include "cjson.h"

static JSONValue *new_node(JSONValueType type)
{
    JSONValue *node = malloc(sizeof(JSONValue));
    if (!node) {
        ERROR_FLAGS |= MEMORY_ALLOCATION_ERROR;
        return NULL;
    }
    node->type = type;
    return node;
}

JSONValue *json_node(Context *context);
static JSONValue *value_node(Context *context);
static JSONValue *array_node(Context *context);
static JSONValue *object_node(Context *context);
static JSONMember *member_node(Context *context);

void free_json(JSONValue *value);
static void free_value(JSONValue *value);
static void free_primitive(JSONValue *value);
static void free_array(JSONValue *value);
static void free_array_elements(JSONValue *value);
static void free_object(JSONValue *value);
static void free_object_members(JSONMember *value);

// json = ws value ws
JSONValue *json_node(Context *context)
{
    if (at_eof(context))
        return NULL;

    JSONValue *node = value_node(context);
    if (!node || !at_eof(context))
        goto failed;
    return node;

failed:
    ERROR_FLAGS |= PARSE_ERROR;
    return NULL;
}

// value = false / null / true / object / array / number / string
static JSONValue *value_node(Context *context)
{
    JSONValue *node = NULL;
    long double num;
    const char *str = NULL;
    if (consume_token(context, TK_FALSE)) {
        if (!(node = new_node(JV_BOOL)))
            return NULL;
        node->value = false;
    }
    else if (consume_token(context, TK_TRUE)) {
        if (!(node = new_node(JV_BOOL)))
            return NULL;
        node->value = true;
    }
    else if (consume_token(context, TK_NULL)) {
        if (!(node = new_node(JV_NULL)))
            return NULL;
    }
    else if (consume_number(context, &num)) {
        if (!(node = new_node(JV_NUM)))
            return NULL;
        node->num = num;
    }
    else if (consume_string(context, &str)) {
        if (!(node = new_node(JV_STR)))
            return NULL;
        node->str = str;
    }
    else if (consume_token(context, TK_BEGIN_ARRAY)) {
        if (!(node = array_node(context)))
            return NULL;
    }
    else if (consume_token(context, TK_BEGIN_OBJECT)) {
        if (!(node = object_node(context)))
            return NULL;
    }
    else {
        return NULL;
    }
    return node;
}

static JSONValue *array_node(Context *context)
{
    JSONValue dummy;
    dummy.next = NULL;
    JSONValue *head = &dummy;

    // Emtpy array
    if (consume_token(context, TK_END_ARRAY))
        goto endloop;

    while (true) {
        if (!(head->next = value_node(context)))
            goto failed;
        head = head->next;
        head->next = NULL;
        if (consume_token(context, TK_END_ARRAY))
            goto endloop;
        if (!expect_token(context, TK_VALUE_SEP))
            goto failed;
    }

endloop: {
    JSONValue *node = new_node(JV_ARRAY);
    if (!node)
        goto failed;
    node->elements = dummy.next;
    return node;
}

failed: {
    free_array_elements(dummy.next);
    return NULL;
}
}

static JSONValue *object_node(Context *context)
{
    JSONMember dummy;
    dummy.next = NULL;
    JSONMember *head = &dummy;

    // Empty object
    if (consume_token(context, TK_END_OBJECT))
        goto endloop;

    while (true) {
        if (!(head->next = member_node(context)))
            goto failed;
        head = head->next;
        if (consume_token(context, TK_END_OBJECT))
            goto endloop;
        if (!expect_token(context, TK_VALUE_SEP))
            goto failed;
    }

endloop: {
    JSONValue *node = new_node(JV_OBJECT);
    if (!node)
        goto failed;
    node->members = dummy.next;
    return node;
}

failed: {
    free_object_members(dummy.next);
    return NULL;
}
}

static JSONMember *member_node(Context *context)
{
    const char *key;
    if (!expect_string(context, &key))
        return NULL;

    if (!expect_token(context, TK_NAME_SEP))
        return NULL;

    JSONValue *value = value_node(context);
    if (!value)
        return NULL;

    JSONMember *node = malloc(sizeof(JSONMember));
    if (!node)
        return NULL;
    node->key = key;
    node->value = value;
    node->next = NULL;
    return node;
}

void free_json(JSONValue *value)
{
    if (!value)
        return;
    free_value(value);
}

static void free_value(JSONValue *value)
{
    if (!value)
        return;
    switch (value->type) {
        case JV_BOOL:
        case JV_NULL:
        case JV_NUM:
        case JV_STR:
            free_primitive(value);
            break;
        case JV_ARRAY:
            free_array(value);
            break;
        case JV_OBJECT:
            free_object(value);
            break;
    }
}

static void free_primitive(JSONValue *value)
{
    free(value);
}

static void free_array(JSONValue *value)
{
    if (!value)
        return;
    free_array_elements(value->elements);
    free(value);
}

static void free_array_elements(JSONValue *value)
{
    if (!value)
        return;
    if (value->next)
        free_array_elements(value->next);
    free_value(value);
}

static void free_object(JSONValue *value)
{
    if (!value)
        return;
    free_object_members(value->members);
    free(value);
}

static void free_object_members(JSONMember *value)
{
    if (!value)
        return;
    if (value->next)
        free_object_members(value->next);
    free_value(value->value);
    free(value);
}
