#include "cjson.h"

unsigned char ERROR_FLAGS;

JSONValue *parse(const char *code)
{
    ERROR_FLAGS = 0x00;

    // Tokenize
    Token *tokens = tokenize(code);
    if (ERROR_FLAGS) {
        return NULL;
    }

    // Create context
    Context context;
    context.current_token = tokens;

    // Parse
    JSONValue *value = json_node(&context);
    free_tokens(context.current_token);
    if (ERROR_FLAGS) {
        return NULL;
    }
    return value;
}
