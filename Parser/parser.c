#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <trie.h>

extern token_t* list;
extern token_t* head;
struct literal_pair* literals_head;
static size_t str_count;
static struct trie functions;

token_t* next(void) {
    list = list->next;
    return list;
}

uint8_t check_consume(token_type_t type) {
    if(!list) return 0;
    if (list->type == type) {
        list = list->next;
        return 1;
    }
    return 0;
}

uint8_t check_next(token_type_t type) {
    if (!(list->next)) return 0;
    return list->next->type == type;
}

uint8_t check_token(token_type_t type) {
    if(!list) return 0;
    return list->type == type;
}

struct statement_list* ast_parse(void) {
    list = head;
    list = list->next;

    struct statement_list* statements = (struct statement_list*)malloc(sizeof(struct statement_list));
    struct statement_list* statements_head = statements;

    while(list != NULL) {
        statements->statement = ast_statement();
        statements->next = (struct statement_list*)malloc(sizeof(struct statement_list));
        statements = statements->next;
    }
    return statements_head;
}

ast_statement_t* ast_function(void) {
    ast_statement_t* fn = malloc(sizeof(ast_statement_t));
    fn->t = FUNCTION;
    fn->statement.function.type = ast_type();

    if (!check_token(TT_IDENTIFIER)) {
        perror("This identifier is not really a identifier");
        return -1;
    }
    fn->statement.function.identifier = list->lexeme;
    next();
    check_consume(TT_LEFT_PAREN);

    size_t stack_size = 0;

    token_t* aux = list;
    expr_type_t arg_type = ast_type();
    if ((arg_type & TYPE_MASK) == VOID_T) {
        if (arg_type & TYPE_POINTER) goto check_args;
        else {
            check_consume(TT_RIGHT_PAREN);
            fn->statement.function.arg_count = 0;
        }
    } else if (!check_consume(TT_RIGHT_PAREN)) {
        check_args:
        list = aux;
        size_t arg_count = 0;
        while (!check_consume(TT_RIGHT_PAREN)) {
            if (check_consume(TT_COMMA)) arg_count++;
            else next();
        }
        arg_count++;
        fn->statement.function.arg_count = arg_count;
        fn->statement.function.args = (struct args*)malloc(sizeof(struct arg) * arg_count);
        struct arg* args = fn->statement.function.args;

        list = aux;
        for(size_t i = 0; !check_consume(TT_RIGHT_PAREN); i++) {
            args[i].type = ast_type();
            stack_size += ast_type_size(args[i].type);
            if (!check_token(TT_IDENTIFIER)) args[i].identifier = NULL;
            else {
                args[i].identifier = list->lexeme;
                next();
            }

            check_consume(TT_COMMA);
        }
    } else fn->statement.function.arg_count = 0;

    if (check_consume(TT_SEMICOLON)) fn->statement.function.block = NULL;
    else if (check_token(TT_LEFT_CURLY)) {
        fn->statement.function.block = ast_block();
        fn->statement.function.block->stack_size += stack_size;
    }
    else if (check_consume(TT_SEMICOLON)) fn->statement.function.block = NULL;
    else perror("EXCEPTION DE BURRICE");

    trie_insert(&functions, fn->statement.function.identifier, fn->statement.function.type);
    return fn;
}

ast_statement_t* ast_asm(void) {
    if (!check_consume(TT_LEFT_PAREN))
        perror("Expected '(' after asm");

    ast_statement_t* inline_asm = malloc(sizeof(ast_statement_t));
    inline_asm->t = INLINE_ASM;

    if (!check_token(TT_STRING))
        perror("Expected string literal in asm");

    inline_asm->statement.asms.source = strdup(list->lexeme);
    next();

    if (!check_consume(TT_COLON))
        perror("Expected ':' after asm source");

    size_t value_count = 0;
    token_t* aux = list;

    while (!check_token(TT_RIGHT_PAREN)) {
        value_count++;
        next();
        check_consume(TT_COMMA);
    }

    list = aux;

    inline_asm->statement.asms.value_count = value_count;
    inline_asm->statement.asms.values = malloc(sizeof(ast_node_t*) * value_count);
    ast_node_t** values = inline_asm->statement.asms.values;

    for (size_t i = 0; i < value_count; i++) {
        values[i] = expression();
        check_consume(TT_COMMA);
    }

    check_consume(TT_RIGHT_PAREN);
    return inline_asm;
}

struct block_member* ast_block(void) {
    struct block_member* block = NULL;
    struct block_member* head = block;
    size_t stack_size = 0;
    if (!check_consume(TT_LEFT_CURLY)) perror("Expected { to initiate a block");
    
    if (check_consume(TT_RIGHT_CURLY)) return NULL;

    while (!check_consume(TT_RIGHT_CURLY)) {
        ast_statement_t* statement = ast_statement();
        if (statement->t == VAR_DEF) stack_size += ast_type_size(statement->statement.var_def.type);

        if (block == NULL) block = malloc(sizeof(struct block_member));

        if (head == NULL) head = block;

        block->value = statement;
        block->next = malloc(sizeof(struct block_member));
        block = block->next;
        block->value = NULL;
    }

    head->stack_size = stack_size;
    return head;
}

ast_node_t* ast_call(void) {
    ast_node_t* call = malloc(sizeof(ast_node_t));
    call->type = CALL;

    if (!check_token(TT_IDENTIFIER)) perror("Nome não reconhecido");

    call->expr.call.identifier = list->lexeme;
    next();

    size_t arg_count = 0;

    if(!check_consume(TT_LEFT_PAREN)) perror("Expected ( isso msm");

    token_t* aux = list;
    while(!check_token(TT_RIGHT_PAREN)) {
        if (check_consume(TT_COMMA)) arg_count++;
        else next();
    }

    list = aux;
    arg_count++;
    call->expr.call.arg_count = arg_count;
    call->expr.call.args = (ast_node_t**)malloc(sizeof(ast_node_t*) * arg_count);
    ast_node_t** args = call->expr.call.args;

    for(size_t i = 0; !check_consume(TT_RIGHT_PAREN); i++) {
        args[i] = expression();

        check_consume(TT_COMMA);
    }

    if (!check_consume(TT_SEMICOLON)) perror("Expected ';' after asm source");

    call->expr_type = trie_get(&functions, call->expr.call.identifier);

    return call;
}

ast_statement_t* ast_variable(void) {
    ast_statement_t* var = (ast_statement_t*)malloc(sizeof(ast_statement_t));

    var->t = VAR_DEF;
    var->statement.var_def.type = ast_type();
    if (!check_consume(TT_IDENTIFIER)) perror("Deixa de ser burro");

    var->statement.var_def.identifier = list->lexeme;
    next();

    if (check_consume(TT_ASSIGN)) {
        ast_statement_t* assignment = (ast_statement_t*)malloc(sizeof(ast_statement_t));
        assignment->t = VAR_ASSIGN;
        assignment->statement.var_assign.identifier = var->statement.var_def.identifier;
        assignment->statement.var_assign.value = expression();
        var->statement.var_def.assigment = assignment;
    }

    if (!check_consume(TT_SEMICOLON)) perror("Expected ';' after asm source");

    return var;
}

ast_statement_t* ast_return(void) {
    ast_statement_t* value = (ast_statement_t*)malloc(sizeof(ast_statement_t));
    value->t = RETURN_STATEMENT;
    value->statement.ret.value = expression();

    if (!check_consume(TT_SEMICOLON)) perror("Expected ';' after asm source");

    return value;
}

ast_statement_t* ast_statement(void) {
    token_t* aux = list;
    if(check_consume(TT_RETURN)) return ast_return();
    else {
        switch (list->type){
            case TT_LONG:
            case TT_SHORT:
            case TT_UNSIGNED:
            case TT_CHAR:
            case TT_INT:
            case TT_FLOAT:
            case TT_DOUBLE:
            case TT_VOID: {
                aux = list;
                ast_type();
                check_consume(TT_IDENTIFIER);
                if (check_token(TT_ASSIGN)) {
                    list = aux;
                    return ast_variable();
                } else if (check_token(TT_LEFT_PAREN)) {
                    list = aux;
                    return ast_function();
                }
            } 
            break;
            default:
                break;
        }

        ast_statement_t* expr = (ast_statement_t*)malloc(sizeof(ast_statement));
        expr->t = EXPRESSION;
        expr->statement.expression = expression();

        if (!check_consume(TT_SEMICOLON)) perror("Expected ';' after asm source");

        return expr;
    }
}

ast_node_t* expression(void) {
    ast_node_t* t = term();

    while(check_token(TT_PLUS) || check_token(TT_MINUS)) {
        ast_node_t* b = malloc(sizeof(ast_node_t));
        b->type = BINARY;
        b->expr.binary.op = check_token(TT_PLUS) ? OP_PLUS : OP_MINUS;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = term();

        t = b;
    }

    return t;
}

ast_node_t* term(void) {
    ast_node_t* t = factor();

    while(check_token(TT_STAR) || check_token(TT_SLASH)) {
        ast_node_t* b = malloc(sizeof(ast_node_t));
        b->type = BINARY;
        b->expr.binary.op = check_token(TT_STAR) ? OP_STAR : OP_SLASH;
        b->expr.binary.left = t;
        next();
        b->expr.binary.right = factor();

        t = b;
    }

    return t;
}

ast_node_t* factor(void) {
    token_t* aux = list;
    if(check_token(TT_NUMBER)) {
        ast_node_t* node = malloc(sizeof(ast_node_t));
        node->expr.integer = atoi(list->lexeme);
        node->type = INTEGER;
        next();

        return node;
    } else if(check_consume(TT_IDENTIFIER)) {
        if(check_token(TT_LEFT_PAREN)) {
            list = aux;
            return ast_call();
        }
        list = aux;

        ast_node_t* node = malloc(sizeof(ast_node_t));
        node->expr.identifier = list->lexeme;
        node->type = ID;
        next();
        
        return node;
    } else if(check_token(TT_STRING)) {
        struct literal_pair* literal = malloc(sizeof(struct literal_pair));

        literal->literal = list->lexeme;
        literal->label = str_count;
        if(literals_head != NULL) 
            literal->next = literals_head;
        literals_head = literal;

        ast_node_t* node = malloc(sizeof(ast_node_t));
        node->expr.string = str_count;
        node->type = STRING_LIT;
        next();
        str_count++;

        return node;
    }

    return NULL;
}

expr_type_t ast_type(void) {
    expr_type_t res;

    if (check_token(TT_STRUCT)) {
        char* s_name = next()->lexeme;
        char* s_type = aligned_alloc(0x1000, strlen(s_name)+1);
        strcpy(s_type, s_name);
        next();
        res = ((size_t)s_type) | (STRUC & 0xfff);
    } else if (check_consume(TT_LONG)) {
        check_consume(TT_INT);
        res = INT64;
    } else if (check_consume(TT_SHORT)) {
        check_consume(TT_INT);
        res = INT16;
    } else if (check_consume(TT_UNSIGNED)) {
        expr_type_t t = ast_type();
        switch (t) {
            case INT8:
                res = UINT8;
                break;
            case INT16:
                res = UINT16;
                break;
            case INT32:
                res = UINT32;
                break;
            case INT64:
                res = UINT64;
                break;
            default:
                break;
        }
    } else if (check_consume(TT_CHAR))
        res = INT8;
    else if (check_consume(TT_INT))
        res = INT32;
    else if (check_consume(TT_VOID))
        res = VOID_T;
    
    if (check_consume(TT_STAR))
        res |= TYPE_POINTER;
    
    return res;
}

size_t ast_type_size(expr_type_t type) {
    switch (type) {
        case INT8:
            return 1;
        case INT16:
            return 2;
        case INT32:
            return 4;
        case INT64:
            return 8;
        case UINT8:
            return 1;
        case UINT16:
            return 2;
        case UINT32:
            return 4;
        case UINT64:
            return 8;
        case F32:
            return 4;
        case F64:
            return 8;
        case VOID_T:
            return 0;
        case STRUC:
            return 0;
        default:
            return -1;
    }
}

void ast_walk(ast_node_t* ast) {
    if((ast->type & 0xfff) == INTEGER)
        printf("n: %ld\n", ast->expr.integer);
    else if ((ast->type) == BINARY) {
        printf("(");
        ast_walk(ast->expr.binary.left);
        printf(" + ");
        ast_walk(ast->expr.binary.right);
        printf(")\n");
    }
}
