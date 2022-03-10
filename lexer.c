#include "cjson.h"

#define IF_STRING_MATCH_THEN_CREATE_TOKEN(kind, str) \
{ \
    unsigned int len = strlen(str); \
    if (!strncmp(current_char, str, len)) { \
        if (!(head = insert_token(head, kind, current_char, len))) \
            goto failed; \
        current_char += len; \
        continue; \
    } \
} \

static const char *get_nullchar_ptr(const char *str)
{
    while (*str)
        str++;
    return str;
}

static Token *insert_token(Token *head, TokenKind kind, const char *str, int str_length)
{
    Token *new_token = malloc(sizeof(Token));
    if (!new_token) {
        ERROR_FLAGS |= MEMORY_ALLOCATION_ERROR;
        return NULL;
    }

    new_token->kind = kind;
    new_token->str = str;
    new_token->str_length = str_length;
    new_token->next = NULL;

    head->next = new_token;
    return new_token;
}

static const char *tokenize_string(const char *current_char, const char **next_ptr)
{
    if (*current_char != '"')
        return NULL;
    current_char++;

    StringBuilder builder;
    if (!initial_sb(&builder)) {
        ERROR_FLAGS |= MEMORY_ALLOCATION_ERROR;
        return NULL;
    }

    char c;
    while (*current_char != '"') {
        if (*current_char == '\0')
            return NULL;
        if (*current_char == '\\') {
            current_char++;
            if (*current_char == '"') {
                c = 0x22;
                goto update;
            }
            if (*current_char == '\\') {
                c = 0x5C;
                goto update;
            }
            if (*current_char == '/') {
                c = 0x2F;
                goto update;
            }
            if (*current_char == 'b') {
                c = 0x08;
                goto update;
            }
            if (*current_char == 'f') {
                c = 0x0C;
                goto update;
            }
            if (*current_char == 'n') {
                c = 0x0A;
                goto update;
            }
            if (*current_char == 'r') {
                c = 0x0D;
                goto update;
            }
            if (*current_char == 't') {
                c = 0x09;
                goto update;
            }
            if (*current_char == 'u') {
                // TODO: あとでワイド文字に対応させる
                ERROR_FLAGS |= UNSUPPORTED_ERROR;
                return NULL;
            }
            return NULL;
        }
        c = *current_char;
update:
        if (!append_sb(&builder, c)) {
            ERROR_FLAGS |= MEMORY_ALLOCATION_ERROR;
            return NULL;
        }
        current_char++;
    }
    current_char++;
    *next_ptr = current_char;
    const char *str = get_str_sb(&builder);
    if (!str) {
        ERROR_FLAGS |= MEMORY_ALLOCATION_ERROR;
        return NULL;
    }
    return str;
}

Token *tokenize(const char *code)
{
    Token dummy;
    dummy.next = NULL;

    Token *head = &dummy;
    const char *current_char = code;
    const char *end_char = get_nullchar_ptr(code);

    while (end_char > current_char) {
        if (strchr("\x20\x09\x0A\x0D", *current_char)) {
            current_char++;
            continue;
        }

// printf("%p %c\n", current_char, *current_char);

        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_FALSE, "false");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_TRUE, "true");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_NULL, "null");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_BEGIN_ARRAY, "[");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_END_ARRAY, "]");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_VALUE_SEP, ",");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_BEGIN_OBJECT, "{");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_NAME_SEP, ":");
        IF_STRING_MATCH_THEN_CREATE_TOKEN(TK_END_OBJECT, "}");

        // Number
        if (*current_char == '-' || isdigit(*current_char)) {
            if (!(head = insert_token(head, TK_NUM, current_char, 0)))
                goto failed;
            char *old_char = (char*)current_char;
            head->num = strtold(current_char, (char **)&current_char);
            if (old_char == current_char)
                goto failed;
            continue;
        }

        // String
        if (*current_char == '"') {
            const char *str;
            if (!(str = tokenize_string(current_char, &current_char)))
                goto failed;
            if (!(head = insert_token(head, TK_STR, current_char, 0)))
                goto failed;
            head->str = str;
            head->str_length = strlen(str);
            continue;
        }

        goto failed;
    }

// printf("%p %c\n", current_char, *current_char);

    if (!insert_token(head, TK_EOF, current_char, 0))
        goto failed;
    return dummy.next;

failed:
    ERROR_FLAGS |= TOKENIZE_ERROR;
    free_tokens(dummy.next);
    return NULL;
}

void free_tokens(Token *tokens)
{
    if (tokens && tokens->next)
        free_tokens(tokens->next);
    free(tokens);
}
