#include <stdio.h>
#include <lexer.h>

static const char* source = "1 + 2";

int main() {
    token_t* list = lexer_parse(source);

    while (list != NULL) {
        printf("token: %s\n", list->lexeme);

        list = list->next;
    }
    return 0;
}
