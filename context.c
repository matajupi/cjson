#include "cjson.h"

static void advance_token(Context *context)
{
    if (context->current_token->kind == TK_EOF)
        return;
    Token *previous_token = context->current_token;
    context->current_token = context->current_token->next;
    free(previous_token);
}

bool consume_token(Context *context, TokenKind kind)
{
    if (context->current_token->kind == kind) {
        advance_token(context);
        return true;
    }
    return false;
}

bool consume_number(Context *context, long double *num_ptr)
{
    if (context->current_token->kind == TK_NUM) {
        *num_ptr = context->current_token->num;
        advance_token(context);
        return true;
    }
    return false;
}

bool consume_string(Context *context, const char **str_ptr)
{
    if (context->current_token->kind == TK_STR) {
        *str_ptr = context->current_token->str;
        advance_token(context);
        return true;
    }
    return false;
}

bool expect_token(Context *context, TokenKind kind)
{
    if (context->current_token->kind == kind) {
        advance_token(context);
        return true;
    }
    ERROR_FLAGS |= PARSE_ERROR;
    return false;
}

bool expect_string(Context *context, const char **str_ptr)
{
    if (context->current_token->kind == TK_STR) {
        *str_ptr = context->current_token->str;
        advance_token(context);
        return true;
    }
    ERROR_FLAGS |= PARSE_ERROR;
    return false;
}

bool lookahead_token(Context *context, TokenKind kind, unsigned int n)
{
    Token *token = context->current_token;
    for (unsigned int i = 0; n > i; i++) {
        token = token->next;
        if (!token)
            return false;
    }
    return token->kind == kind;
}

bool at_eof(Context *context)
{
    return context->current_token->kind == TK_EOF;
}
