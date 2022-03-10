#include <stdio.h>
#include <stdlib.h>
#include "cjson.h"

typedef unsigned char byte;
typedef signed char sbyte;

static void dump_json(JSONValue *value, unsigned int depth);

static void print_tabs(unsigned int depth)
{
    for (unsigned int i = 0; depth > i; i++)
        printf("\t");
}

static void dump_member(JSONMember *value, unsigned int depth)
{
    if (!value)
        return;
    print_tabs(depth);

    printf("MemberNode(%s) {\n", value->key);
    dump_json(value->value, depth + 1);

    print_tabs(depth);
    printf("}\n");
}

static void dump_json(JSONValue *value, unsigned int depth)
{
    if (!value)
        return;

    print_tabs(depth);
    switch (value->type) {
        case JV_BOOL:
            printf("BoolNode(%s)\n", value->value ? "true" : "false");
            break;
        case JV_NULL:
            printf("NULLNode\n");
            break;
        case JV_NUM:
            printf("NumNode(%Lf)\n", value->num);
            break;
        case JV_STR:
            printf("StringNode(%s)\n", value->str);
            break;
        case JV_ARRAY:
            printf("ArrayNode {\n");
            for (JSONValue *current = value->elements; current; current = current->next)
                dump_json(current, depth + 1);
            print_tabs(depth);
            printf("}\n");
            break;
        case JV_OBJECT:
            printf("ObjectNode {\n");
            for (JSONMember *current = value->members; current; current = current->next)
                dump_member(current, depth + 1);
            print_tabs(depth);
            printf("}\n");
            break;
        default:
            printf("Unsupported Node\n");
            break;
    }
}

static void test(const char *code)
{
    printf("==================== Code =======================\n");
    printf("%s\n", code);

    printf("=================== Result ======================\n");
    JSONValue *value = parse(code);
    if (ERROR_FLAGS)
        printf("Failure\n");
    else
        printf("Success\n");

    printf("=================== Detail ======================\n");
    if (ERROR_FLAGS)
        printf("ERROR_FLAGS: %02x\n", ERROR_FLAGS);
    else
        dump_json(value, 0);

    printf("=================================================\n");
    free_json(value);
}

static void test_from_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Runtime Error: Couldn't open the file named \"%s\".\n", filename);
        return;
    }


    StringBuilder builder;
    if (!initial_sb(&builder)) {
        fprintf(stderr, "Runtime Error: Couldn't allocate required memory.\n");
        return;
    }

    char c;
    while ((c = fgetc(fp)) != EOF) {
        if (!append_sb(&builder, c)) {
            fprintf(stderr, "Runtime Error: Couldn't allocate required memory.\n");
            return;
        }
    }

    fclose(fp);

    test(get_str_sb(&builder));
}

int main(int argc, char **argv)
{
    printf("================== Start test ===================\n");
    test("");
    test("   true      ");
    test("true");
    test("false   ");
    test("  null ");
    test("   hello   ");
    test("  3 ");
    test(" 13  ");
    test("\n\n 343.234  \n");
    test("  -2.5E+14");
    test("   43.000e-3  ");
    test("  021  ");
    test("  -   ");
    test("\"hello world\"");
    test("   \"\"  ");
    test("  \"  \\\" \"");
    test("  \"  hello\\nworld\"");
    test("\"  hello\\tworld\"");
    test("\"  hello\\n\\tworld\\\\\"");
    test("\" \\u3424  \"");
    test("\"          test  ");

    test("");
    test("    true   ");
    test("              false           ");
    test("      null ");
    test("  -12 ");
    test("      234.01  ");
    test("   0 ");
    test("       345.3457E32    ");
    test("          -43.32e-23   ");
    test("  6.02e23    ");
    test("      1.602E-19");
    test("      3e20 ");
    test("\"Hello world\"");
    test("    \"    Hello \\n     \"   ");
    test("          \"   ,    \\u3333    \\n \\\\   \\\"  \"   ");
    test("   \"\"   ");
    test("          \" fjfkdsjfdsl");
    test("              \"\"\"   ");
    test("      true  \" fdsf ");
    test("      \"   \\a   \"  ");
    test("{ \"member\"  :   \"Hello world\" }   ");
    test("{     }");
    test("   { \"member\"  : { \"haha\" : \"mom\" }   , \"pi\"   : 3.14 }   ");
    test("  { \"member\" : \"Hello\", } ");
    test("   { true:     { }   }");
    test("[   \"Hello\"   , \" world   \"   ]");
    test("   [   {  \"m\": 3  },  3.4e32  ]  ");
    test("   [  \"string\", true, false, 3.14, null, {}  ]  ");
    test("   [   \"  fail   \"   , false  , null   ,  ]   ");
    test("   [   \" fail \",    \" suspend ]");
    test("   [   \"  fail   \"   , false  , null     ]   ");
    test("[ \"fail\", \"etc\"");

    test_from_file("case1.json");
    test_from_file("case2.json");

    printf("================== Finish test ==================\n");
    return EXIT_SUCCESS;
}
