#ifndef LEXER_H
#define LEXER_H

typedef enum token_type {
    // literais
    TT_NUMBER,
    TT_IDENTIFIER,
    TT_STRING,

    // operadores
    TT_PLUS,
    TT_MINUS,
    TT_SLASH,
    TT_STAR,
    TT_MOD,
    TT_PLUS_EQUAL,
    TT_MINUS_EQUAL,
    TT_STAR_EQUAL,
    TT_SLASH_EQUAL,
    TT_MOD_EQUAL,
    TT_PLUS_PLUS,
    TT_MINUS_MINUS,
    TT_ASSIGN,

    // operadores lógicos
    TT_GREATER,
    TT_LESS,
    TT_EQUAL,
    TT_LESS_EQUAL,
    TT_GREATER_EQUAL,
    TT_NOT_EQUAL,
    TT_EQUAL,
    TT_LOG_OR,
    TT_LOG_AND,

    // operadores de bit
    TT_NOT,
    TT_OR,
    TT_AND,

    // palavras-chave
    TT_FOR,
    TT_IF,
    TT_ELSE,
    TT_WHILE,
    TT_CONTINUE,
    TT_SWITCH,
    TT_CASE,
    TT_BREAK,
    TT_STATIC,
    TT_CONST,
    TT_LONG,
    TT_SHORT,
    TT_UNSIGNED,
    TT_CHAR,
    TT_INT,
    TT_FLOAT,
    TT_DOUBLE,
    TT_VOID,
    TT_RETURN,
    TT_CONTROLLER,
    TT_CLASS,
    TT_PRIVATE,
    TT_PUBLIC,
    TT_PROTECTED,
    TT_STRUCT,

    // simbolos
    TT_LEFT_PAREN,
    TT_RIGHT_PAREN,
    TT_LEFT_SQUARE,
    TT_RIGHT_SQUARE,
    TT_LEFT_CURLY,
    TT_RIGHT_CURLY,
    TT_COMMA,
    TT_DOT,
    TT_ARROW,
    TT_COLON,
    TT_SEMICOLON,

    TT_MACRO,
} token_type_t;

typedef struct token {
    token_type_t type;
    char* lexeme;
    struct token* next;
} token_t;

token_t* lexer_parse(char* source);
void lexer_push(token_type_t, char* lexeme);

#endif
