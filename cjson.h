#ifndef CJSON_H
#define CJSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// ========== util.c ==========
#define STRING_BUILDER_INITIAL_CAPACITY 256
typedef struct StringBuilder StringBuilder;
struct StringBuilder {
    size_t size;
    size_t capacity;
    char *str;
};

StringBuilder *new_sb();
bool initial_sb(StringBuilder *sb);
bool append_sb(StringBuilder *sb, char c);
const char *get_str_sb(StringBuilder *sb);

// ========== lexer.c ==========
typedef enum TokenKind TokenKind;
enum TokenKind {
    TK_FALSE,
    TK_TRUE,
    TK_NULL,
    TK_NUM,
    TK_STR,
    TK_BEGIN_ARRAY,
    TK_VALUE_SEP,
    TK_END_ARRAY,
    TK_BEGIN_OBJECT,
    TK_NAME_SEP,
    TK_END_OBJECT,
    TK_EOF,
};

typedef struct Token Token;
struct Token {
    TokenKind kind;
    const char *str;
    int str_length;
    long double num;

    Token *next;
};

Token *tokenize(const char *code);
void free_tokens(Token *tokens);


// ========== context.c ==========
typedef struct Context Context;
struct Context {
    Token *current_token;
};

bool consume_token(Context *context, TokenKind kind);
bool consume_number(Context *context, long double *num_ptr);
bool consume_string(Context *context, const char **str_ptr);
bool expect_token(Context *context, TokenKind kind);
bool expect_string(Context *context, const char **str_ptr);
bool lookahead_token(Context *context, TokenKind kind, unsigned int n);
bool at_eof(Context *context);


// ========== parser.c ==========
typedef enum JSONValueType JSONValueType;
enum JSONValueType {
    JV_BOOL,
    JV_NULL,
    JV_NUM,
    JV_STR,
    JV_ARRAY,
    JV_OBJECT,
};

typedef struct JSONMember JSONMember;
struct JSONMember;

typedef struct JSONValue JSONValue;
struct JSONValue {
    JSONValueType type;

    // Object members
    JSONMember *members; // 最後はNULLポインタ

    // Array members
    JSONValue *elements; // 最後はNULLポインタ
    JSONValue *next;

    // String members
    const char *str;

    // Number members
    long double num;

    // Boolean members
    bool value;
};

struct JSONMember {
    const char *key;
    JSONValue *value;

    JSONMember *next;
};

JSONValue *json_node(Context *context);
void free_json(JSONValue *value);


// ========== cjson.c ==========
#define TOKENIZE_ERROR 0x01
#define PARSE_ERROR 0x02
#define MEMORY_ALLOCATION_ERROR 0x04
#define UNSUPPORTED_ERROR 0x08

extern unsigned char ERROR_FLAGS;

JSONValue *parse(const char *code);

#endif // CJSON_H
