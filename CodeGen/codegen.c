#include <codegen.h>
#include <stddef.h>
#include <stdlib.h>
#include <trie.h>
#include <string.h>

extern struct literal_pair* literals_head;

static const char* call_regs[] = {
    "rdi", "rsi", "rdx", "r8", "r9"
};

static struct scope* scope_stack = NULL;
static int main_function = 0;

struct trie* variables = NULL;
static size_t str_count = 0;
static size_t base_offset = 0;

void push_scope(size_t base, size_t pointer) {
    struct scope* scope = (struct scope*)malloc(sizeof(struct scope));
    scope->stack_base = base;
    scope->stack_pointer = pointer;
    scope->prev = scope_stack;
    scope->next = scope;
    scope_stack = scope;
}

struct scope* pop_scope() {
    struct scope* aux = scope_stack;
    scope_stack = scope_stack->prev;
    aux->prev = NULL;
    return aux;
}

void gen_literals(FILE* f) {
    fprintf(f, ".data\n");
    while (literals_head != NULL) {
        fprintf(f, "str%d: .asciz \"%s\"\n");

        literals_head = literals_head->next;
    }
}

void gen_block(FILE* f, struct block_member* block) {
    while(block != NULL) {
        gen_statement(f, block->value);

        block = block->next;
    }
}

void gen_assignment(FILE* f, ast_statement_t* assignment) {
    gen_mov(f, assignment->statement.var_assign.value, "r10");
    fprintf(f, "mov %%r10, -%x(%%rbp)\n", trie_get(variables, assignment->statement.var_assign.identifier));
}

void gen_def(FILE* f, ast_statement_t* statement) {
    trie_insert(variables, statement->statement.var_def.identifier, base_offset);
    base_offset += ast_type_size(statement->statement.var_def.type);
    
    if(statement->statement.var_def.assigment != NULL)
        gen_assignment(f, statement->statement.var_def.assigment);
}

void gen_mov(FILE* f, ast_node_t* expr, char* reg) {
    if (expr == NULL) return;

    if(expr->type == INTEGER)
        fprintf(f, "mov $%d, %%%s\n", expr->expr.integer, reg);
    else if (expr->type == STRING_LIT)
        fprintf(f, "mov $str%d, %%%s\n", expr->expr.string, reg);
    else if (expr->type == ID)
        fprintf(f, "mov -0x%x(%%rbp), %%%s\n", expr->expr.string, reg);
    else if (expr->type == CALL) {
        for (int i = 0; i < expr->expr.call.arg_count; i++) {
            gen_mov(f, expr->expr.call.args[i], call_regs[i]);
        }

        fprinf(f, "call %s\n", expr->expr.call.identifier);
        fprinf(f, "mov %%rax, %%%s\n", reg);
    } else if (expr->type == BINARY) {
        printf("pregas por agora");
    }
}
