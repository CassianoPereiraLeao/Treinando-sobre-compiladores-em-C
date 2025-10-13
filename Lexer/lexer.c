#include <lexer.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <trie.h>

struct trie keywords = {0};
token_t* list;
token_t* head;

void lexer_push(token_type_t type, char* lexeme) {
    list->next = (token_t*)malloc(sizeof(token_t));
    list->next->type = type;
    list->next->lexeme = lexeme;

    list = list->next;
}

token_t* lexer_parse(char* source) {
    trie_insert(&keywords, "for", TT_FOR);
    trie_insert(&keywords, "IF", TT_IF);
    trie_insert(&keywords, "else", TT_ELSE);
    trie_insert(&keywords, "while", TT_WHILE);
    trie_insert(&keywords, "continue", TT_CONTINUE);
    trie_insert(&keywords, "switch", TT_SWITCH);
    trie_insert(&keywords, "case", TT_CASE);
    trie_insert(&keywords, "break", TT_BREAK);
    trie_insert(&keywords, "static", TT_STATIC);
    trie_insert(&keywords, "const", TT_CONST);
    trie_insert(&keywords, "long", TT_LONG);
    trie_insert(&keywords, "short", TT_SHORT);
    trie_insert(&keywords, "unsigned", TT_UNSIGNED);
    trie_insert(&keywords, "char", TT_CHAR);
    trie_insert(&keywords, "int", TT_INT);
    trie_insert(&keywords, "float", TT_FLOAT);
    trie_insert(&keywords, "double", TT_DOUBLE);
    trie_insert(&keywords, "void", TT_VOID);
    trie_insert(&keywords, "return", TT_RETURN);
    trie_insert(&keywords, "controller", TT_CONTROLLER);
    trie_insert(&keywords, "class", TT_CLASS);
    trie_insert(&keywords, "public", TT_PUBLIC);
    trie_insert(&keywords, "private", TT_PRIVATE);
    trie_insert(&keywords, "protected", TT_PROTECTED);

    list = (token_t*)malloc(sizeof(token_t));
    head = list;
    bzero(list, sizeof(token_t));

    for(size_t i = 0; i < strlen(source); i++) {
        char current = source[i];
        char next;

        if(isspace(current)) continue;

        switch(current) {
            case '+':
                next = source[i + 1];
                if(next == '+') {
                    lexer_push(TT_PLUS_PLUS, "++");
                    i++;
                } else if(next == '=') {
                    lexer_push(TT_PLUS_EQUAL, "+=");
                    i++;
                } else {
                    lexer_push(TT_PLUS, "+");
                }
                break;
            case '-':
                next = source[i+1];
                if(next == '-') {
                    lexer_push(TT_MINUS_MINUS, "--");
                    i++;
                } else if(next == '=') {
                    lexer_push(TT_MINUS_EQUAL, "-=");
                    i++;
                } else if(next == '>') {
                    lexer_push(TT_ARROW, "->");
                    i++;
                } else {
                    lexer_push(TT_MINUS, "-");
                }
                break;
            case '*':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_STAR_EQUAL, "*=");
                    i++;
                } else {
                    lexer_push(TT_STAR, "*");
                }
                break;
            case '/':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_SLASH_EQUAL, "/=");
                    i++;
                } else {
                    lexer_push(TT_SLASH, "/");
                }
                break;
            case '=':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_EQUAL, "==");
                    i++;
                } else {
                    lexer_push(TT_ASSIGN, "=");
                }
                break;
            case '>':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_GREATER_EQUAL, ">=");
                    i++;
                } else {
                    lexer_push(TT_GREATER, ">");
                }
                break;
            case '<':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_LESS_EQUAL, "<=");
                    i++;
                } else {
                    lexer_push(TT_LESS, "<");
                }
                break;
            case '!':
                next = source[i+1];
                if(next == '=') {
                    lexer_push(TT_NOT_EQUAL, "!=");
                    i++;
                } else {
                    lexer_push(TT_NOT, "!");
                }
                break;
            case '|':
                next = source[i+1];
                if(next == '|') {
                    lexer_push(TT_LOG_OR, "||");
                    i++;
                } else {
                    lexer_push(TT_OR, "|");
                }
                break;
            case '&':
                if(next == '&') {
                    lexer_push(TT_LOG_AND, "&&");
                    i++;
                } else {
                    lexer_push(TT_AND, "&");
                }
                break;
            case '(':
                lexer_push(TT_LEFT_PAREN, "(");
                break;
            case ')':
                lexer_push(TT_RIGHT_PAREN, ")");
                break;
            case '[':
                lexer_push(TT_LEFT_SQUARE, "[");
                break;
            case ']':
                lexer_push(TT_RIGHT_SQUARE, "]");
                break;
            case '{':
                lexer_push(TT_LEFT_CURLY, "{");
                break;
            case '}':
                lexer_push(TT_RIGHT_CURLY, "}");
                break;
            case '.':
                lexer_push(TT_DOT, ".");
                break;
            case ',':
                lexer_push(TT_COMMA, ",");
                break;
            case ':':
                lexer_push(TT_COLON, ":");
                break;
            case ';':
                lexer_push(TT_SEMICOLON, ";");
                break;
        }

        if (isdigit(current)) {
            char* ptr = &source[i];
            size_t len = i;
            i++;
            current = source[i];

            while(isdigit(current)) {
                i++;
                current = source[i];
            }

            len -= i;
            char* lexeme = malloc(sizeof(char) * (len + 1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';
            lexer_push(TT_NUMBER, lexeme);
            i--;
        }

        if (current == '"') {
            char* ptr = &source[i+1];
            size_t len = i+1;
            i++;
            current = source[i];

            while (current != '"') {
                i++;
                current = source[i];
            }

            len -= i;
            char* lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';

            lexer_push(TT_STRING, lexeme);
        }

        if (current == "#") {
            char* ptr = &source[i+1];
            size_t len = i+1;
            i++;
            current = source[i];

            while (current != '\n') {
                i++;
                current = source[i];
            }

            len -= i;
            char* lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';

            lexer_push(TT_MACRO, lexeme);
            i--;
        }

        if (isalpha(current)) {
            char* ptr = &source[i];
            size_t len = i;
            i++;
            current = source[i];

            while (isalnum(current)) {
                i++;
                current = source[i];
            }

            len -= i;
            char* lexeme = malloc(sizeof(char) * (len+1));
            strncpy(lexeme, ptr, len);
            lexeme[len] = '\0';
            token_type_t type = trie_get(&keywords, lexeme);
            if (type == -1) type = TT_IDENTIFIER;

            lexer_push(type, lexeme);
            i--;
        }
    }

    return head;
}
