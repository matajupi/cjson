#include "cjson.h"

static bool expand_sb(StringBuilder *sb)
{
    sb->str = realloc(sb->str, (sb->capacity) *= 2);
    return sb->str;
}

static bool is_full_sb(StringBuilder *sb)
{
    return sb->size >= sb->capacity;
}

bool initial_sb(StringBuilder *sb)
{
    sb->size = 0;
    sb->capacity = STRING_BUILDER_INITIAL_CAPACITY;
    return (sb->str = malloc(sb->capacity));
}

StringBuilder *new_sb()
{
    StringBuilder *sb = malloc(sizeof(StringBuilder));
    if (!sb)
        return NULL;

    if (initial_sb(sb))
        return sb;

    free(sb);
    return NULL;
}

bool append_sb(StringBuilder *sb, char c)
{
    if (is_full_sb(sb))
        if (!expand_sb(sb))
            return false;
    *(sb->str + sb->size++) = c;
    return true;
}

const char *get_str_sb(StringBuilder *sb)
{
    if (!append_sb(sb, '\0'))
        return NULL;
    const char *str = sb->str;
    sb->size--;
    return str;
}
