/* Copyright 2007 Andrea Autiero, Simone Basso.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this client except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This file contains code borrowed for Ritchie & Kernighan's book.
 */

/**
 * @file compiler/main.c
 * Compiler main file.
 */

#include<compiler/compiler.h>

/**
 * @defgroup compiler_impl Compiler implementation
 * @ingroup compiler
 * @{
 * @defgroup mm Memory management
 * @{
 * Memory allocation strategy. We keep a cache for commonly used objects,
 * which are
 *
 * <ul>
 *   <li>headers for registering memory allocations;</li>
 *   <li>entries in backpatching lists;</li>
 *   <li>nodes used during parsing.</li>
 * </ul>
 *
 * These cache are never flushed, because the reasonable point where to do
 * it is at the end of the program. Other allocations, such as strings
 * etc, are registered for being freed when the proper storage is cleared.
 * We keep three storages: one for temporary allocations when parsing
 * current function, one for symbol table and one for code. The former is
 * cleared every time a function is successfully parsed; while the others
 * are never cleared because their lifecycle is the same as the program
 * one.
 */

/** Header for allocated block of memory. */
typedef struct p_storage_header {
    /** Next in linked list of allocated memory blocks. */
    struct p_storage_header *next;
    /** References allocated block of memory. */
    void *base;
} p_storage_header;

/** Memory allocator and garbage collector. */
typedef struct p_storage {
    /** References cache of unused p_storage_header. */
    p_storage_header *free_headers;
    /** References list of p_storage_header in use. */
    p_storage_header *headers;
    /** References cache of unused bp_entry. */
    bp_entry *free_entries;
    /** References cache of unused p_node. */
    p_node *free_nodes;
} p_storage;

/**
 * Create memory allocator.
 * @returns New memory allocator.
 */
static p_storage *p_storage_create(void)
{
    p_storage *storage = malloc(sizeof(p_storage));
    if (!storage) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return storage;
}

/**
 * Clear the memory used by memory allocator.
 * @param storage Memory allocator.
 */
static void p_storage_clear(p_storage *storage)
{
    while ((storage->headers)) {
        p_storage_header *h = storage->headers;
        storage->headers = h->next;
        h->next = storage->free_headers;
        storage->free_headers = h;
        free(h->base);
    }
}

/**
 * Allocate header for allocated block of memory.
 * @param storage Memory allocator.
 * @returns Header for allocated block of memory.
 */
static p_storage_header *p_storage_header_alloc(p_storage *storage)
{
    p_storage_header *h;

    if (storage->free_headers) {
        h = storage->free_headers;
        storage->free_headers = h->next;
    } else {
        h = malloc(sizeof (p_storage_header));
        if (!h) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
    }

    memset(h, 0, sizeof (p_storage_header));
    return h;
}

/**
 * Allocate node for parsing tree.
 * @param s Memory allocator.
 * @returns Node for parsing tree.
 */
static p_node *p_storage_node_alloc(p_storage * s)
{
    p_node * n;

    if (s->free_nodes == NULL) {
        n = malloc(sizeof (p_node));
        if (!n) {
            fprintf(stderr, "out of memory");
            exit(1);
        }
    } else {
        n = s->free_nodes;
        s->free_nodes = n->next;
    }

    memset(n, 0, sizeof (p_node));
    return n;
}

/**
 * Allocate entry for backpatching list.
 * @param s Memory allocator.
 * @returns Entry for backpatching list.
 */
static bp_entry *p_storage_entry_alloc(p_storage *s)
{
    bp_entry * e;

    if (s->free_entries == NULL) {
        e = malloc(sizeof (bp_entry));
        if (!e) {
            fprintf(stderr, "out of memory");
            exit(1);
        }
    } else {
        e = s->free_entries;
        s->free_entries = e->next;
    }

    memset(e, 0, sizeof (bp_entry));
    return e;
}

/**
 * Register a storage header with memory allocator.
 * @param s Memory allocator.
 * @param h Storage header.
 */
static void p_storage_header_register(p_storage *s, p_storage_header *h)
{
    h->next = s->headers;
    s->headers = h;
}

/**
 * Allocate a block of memory.
 * @param storage Memory allocator.
 * @param size Size of memory block.
 * @returns Memory block.
 */
static void *p_storage_alloc(p_storage *storage, size_t size)
{
    p_storage_header *header = p_storage_header_alloc(storage);
    header->base = malloc(size);
    if (!header->base) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    memset(header->base, 0, size);
    p_storage_header_register(storage, header);
    return header->base;
}

/**
 * Duplicate a C string.
 * @param storage Memory allocator.
 * @param str C string to duplicate.
 * @returns C string.
 */
static char *p_storage_strdup(p_storage *storage, const char *str)
{
    p_storage_header *h = p_storage_header_alloc(storage);
    h->base = strdup(str);
    if (!h->base) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    p_storage_header_register(storage, h);
    return h->base;
}

/**
 * @}
 * @defgroup pnode Parsing tree node management
 * @{
 * A parsing node is a node in the parse tree. Scanner creates it and
 * passes it to parser, using yylval global variable: Therefore, YYSTYPE
 * must be a pointer to a parsing node. We create a parsing node foreach
 * non terminal symbol which is meaningful, such as a string, a parameter
 * or function name, etc. We don't need to create a node for a terminal
 * symbol, such as if, for obvious reasons.
 */

/** Allocate a parsing tree node. */
#define p_node_alloc p_storage_node_alloc

/** Duplicate a string. */
#define p_node_strdup p_storage_strdup

/**
 * @}
 * @defgroup bpmanag Backpatching lists management
 * @{
 * We use three lists for backpatching: two are for IF statements, and one
 * is for BODY. When an IF statement is reduced, its truelist and falselist
 * should be empty, because it becomes a block: Therefore, only nextlist
 * must contains entries, e.g. because we don't know which address the
 * end of the block would be.
 *
 * @warning Current implementation of backpatching is O(N), while we
 * could have had O(1) with a ring. However, it seems N is not going
 * to be big.
 */

/**
 * Create backpatching list entry.
 * @param storage Memory allocator.
 * @param instr Instruction that needs backpatching.
 * @returns Backpatching list entry.
 */
static bp_entry *bp_entry_create(p_storage *storage, vm_instr *instr)
{
    bp_entry *entry = p_storage_entry_alloc(storage);
    entry->instr = instr;
    return entry;
}

/**
 * Backpatch @a _list_ with @a _instr_.
 * @param _list_ List of instructions to backpatch.
 * @param _instr_ Location to backpatch @a _list_ with.
 */
#define bp_backpatch(_list_, _instr_)                                       \
do {                                                                        \
    while ((_list_)) {                                                      \
        (_list_)->instr->location = (_instr_)->offset;                      \
        (_list_) = (_list_)->next;                                          \
    }                                                                       \
} while (0)

/**
 * Merge two backpatching lists.
 * @param a Backpatching list.
 * @param b Backpatching list.
 * @returns The merged list.
 * @warning This function may leak a backpatch entry.
 */
static bp_entry * bp_merge(bp_entry * a, bp_entry * b)
{
    bp_entry * rv;
    if (!a) {
        rv = b;
    } else if (!b) {
        rv = a;
    } else {
        rv = a;
        while ((a->next)) {
            a = a->next;
        }
        a->next = b;
    }
    return rv;
}

/**
 * @}
 * @defgroup Generated Code management
 * @{
 * To allocate the code we use a vector, because it's faster to generate
 * code iterating over it, after parsing has finished. Also, we should
 * not pay too much for reallocation, because we don't expect it to
 * happen, given the initial vector size.
 */

/** Code manager. */
typedef struct vm_code {
    /** Storage for strings, etc. */
    p_storage *storage;
    /** Base of instructions vector. */
    vm_instr *base;
    /** Offset of next instruction. */
    unsigned offset;
    /** Instructions vector size. */
    unsigned size;
} vm_code;

/** Duplicate a string. */
#define vm_instr_strdup(code, str) p_storage_strdup((code)->storage, (str))

/** Initial size of instructions vector. */
#define VM_CODE_SIZE 4096

/**
 * Create code manager.
 * @returns Code manager.
 */
static vm_code *vm_code_create(void)
{
    p_storage *s = p_storage_create();
    vm_code *code = p_storage_alloc(s, sizeof (vm_code));
    code->storage = s;
    code->base = calloc(VM_CODE_SIZE, sizeof (vm_instr));
    if (!code->base) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    code->size = VM_CODE_SIZE;
    return code;
}

/**
 * Create an instructions within @a code.
 * @param code Code manager.
 * @returns New instruction.
 */
static vm_instr *vm_instr_create(vm_code * code)
{
    vm_instr * instr;

    if (code->offset >= code->size) {
        code->size *= 2;
        code->base = realloc(code->base, code->size);
        if (!code->base) {
            fprintf(stderr, "out of memory");
            exit(1);
        }
    }

    instr = &code->base[code->offset];
    instr->offset = code->offset++;

    return instr;
}

/**
 * @}
 * @defgroup STM Symbol table management
 * @{
 * Symbol table: We keep a symbol table with names of functions, to
 * generate a GOT (Global Offset Table) to be prepended to executable
 * code. So, it's possible for the VM to jump from one function to
 * another, at least in theory.
 *
 * @note The code to manage symbol tables was taken from R&K "The C Programming
 * Language", 2nd edition, Prentice Hall.
 */

/** Size of hash table. */
#define SYM_TABLE_HASHSIZE 101

/** Symbol table entry that references a function. */
typedef struct sym_table_entry {
    /** Next pointer in case of collision. */
    struct sym_table_entry *next;
    /** First instruction of this function. */
    vm_instr *instr;
    /** Name of this function. */
    char * name;
} sym_table_entry;

/** Symbol table. */
typedef struct sym_table {
    /** Hash table. */
    sym_table_entry *table[SYM_TABLE_HASHSIZE];
    /** Memory allocator. */
    p_storage *storage;
} sym_table;

/**
 * Create symbol table.
 * @returns symbol table.
 */
static sym_table *sym_table_create(void)
{
    p_storage *s = p_storage_create();
    sym_table *t = p_storage_alloc(s, sizeof (sym_table));
    t->storage = s;
    return t;
}

/**
 * Hash function for symbol table.
 * @param s Input string.
 * @returns Hash value.
 */
static unsigned sym_table_hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++) {
        hashval = *s + 31 * hashval;
    }
    return hashval % SYM_TABLE_HASHSIZE;
}

/**
 * Lookup an entry in symbol table.
 * @param t Symbol table.
 * @param s Key.
 * @param h In output, hash for @a s.
 * @returns Symbol table entry, or NULL.
 */
static sym_table_entry *sym_table_lookup(sym_table *t, char *s, unsigned *h)
{
    struct sym_table_entry *e;

    *h = sym_table_hash(s);
    for (e = t->table[*h]; (e); e = e->next) {
        if (strcmp(e->name, s) == 0) {
            return e;
        }
    }
    return NULL;
}

/**
 * Install an entry in symbol table.
 * @param t Symbol table.
 * @param name Function name.
 * @param instr First instruction for function.
 */
static void sym_table_install(sym_table *t, char *name, vm_instr *instr)
{
    struct sym_table_entry *e;
    unsigned hashval;

    e = sym_table_lookup(t, name, &hashval);
    if (e) {
        fprintf(stderr, "error: function %s already exists\n", name);
        exit(1);
    }
    e = p_storage_alloc(t->storage, sizeof (sym_table_entry));
    e->name = p_storage_strdup(t->storage, name);
    e->instr = instr;
    e->next = t->table[hashval];
    t->table[hashval] = e;
}

/**
 * @}
 */

/** Symbol table for function names. */
static sym_table *SymbolTable;
/** Temporary storage used during parsing. */
static p_storage *Storage;
/** Code manager that contains all the generated code. */
static vm_code *Code;

/**
 * @}
 */

void code_eval_FUNC(p_node *name, p_node *param, p_node *body)
{
    vm_instr * instr = vm_instr_create(Code);
    instr->opcode = VM_RETURN;
    bp_backpatch(body->nextlist, instr);
    sym_table_install(SymbolTable, name->lexeme, body->code);
    p_storage_clear(Storage);
}

p_node *code_eval_BODY(p_node *body, p_node *if_or_exec)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = body->code;
    bp_backpatch(body->nextlist, if_or_exec->code);
    p->nextlist = if_or_exec->nextlist;
    return p;
}

p_node *code_eval_IF(p_node *condlist, p_node *body)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = condlist->code;
    bp_backpatch(condlist->truelist, body->code);
    p->nextlist = bp_merge(condlist->falselist, body->nextlist);
    return p;
}

p_node *code_eval_IF_ELSE(p_node *condlist, p_node *if_body, p_node *else_body)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = condlist->code;
    bp_backpatch(condlist->truelist, if_body->code);
    bp_backpatch(condlist->falselist, else_body->code);
    p->nextlist = bp_merge(if_body->nextlist, else_body->nextlist);
    return p;
}

p_node *code_eval_AND(p_node *l, p_node *r)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = l->code;
    bp_backpatch(l->truelist, r->code);
    p->falselist = bp_merge(l->falselist, r->falselist);
    p->truelist = r->truelist;
    return p;
}

p_node *code_eval_OR(p_node *l, p_node *r)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = l->code;
    bp_backpatch(l->falselist, r->code);
    p->truelist = bp_merge(l->truelist, r->truelist);
    p->falselist = r->falselist;
    return p;
}

p_node *code_eval_NOT(p_node *node)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->code = node->code;
    p->truelist = node->falselist;
    p->falselist = node->truelist;
    return p;
}

#define CODE_GEN_CMP(_opcode_)                                              \
p_node *code_gen_##_opcode_(p_node *l, p_node *r)                           \
{                                                                           \
    vm_instr *instr = vm_instr_create(Code);                                \
    p_node *p = p_storage_node_alloc(Storage);                              \
                                                                            \
    instr->opcode = VM_##_opcode_;                                          \
    instr->arg1 = vm_instr_strdup(Code, l->lexeme);                         \
    instr->arg2 = vm_instr_strdup(Code, r->lexeme);                         \
    p->code = instr;                                                        \
                                                                            \
    instr = vm_instr_create(Code);                                          \
    instr->opcode = VM_JTRUE;                                               \
    p->truelist = bp_entry_create(Storage, instr);                          \
                                                                            \
    instr = vm_instr_create(Code);                                          \
    instr->opcode = VM_JFALSE;                                              \
    p->falselist = bp_entry_create(Storage, instr);                         \
                                                                            \
    return p;                                                               \
}

CODE_GEN_CMP(EQ)
CODE_GEN_CMP(MAG)
CODE_GEN_CMP(MIN)
CODE_GEN_CMP(MAEQ)
CODE_GEN_CMP(MIEQ)
CODE_GEN_CMP(NEQ)

p_node * code_gen_NOP(void)
{
    vm_instr *instr = vm_instr_create(Code);
    vm_instr *jmp = vm_instr_create(Code);
    p_node *node = p_node_alloc(Storage);
    instr->opcode = VM_NOP;
    jmp->opcode = VM_JMP;
    node->code = instr;
    node->nextlist = bp_entry_create(Storage, jmp);
    return node;
}

p_node * code_gen_EXEC(p_node * node_string)
{
    vm_instr *instr = vm_instr_create(Code);
    vm_instr *jmp = vm_instr_create(Code);
    p_node *node = p_node_alloc(Storage);
    instr->opcode = VM_EXEC;
    jmp->opcode = VM_JMP;
    instr->arg1 = vm_instr_strdup(Code, node_string->lexeme);
    node->code = instr;
    node->nextlist = bp_entry_create(Storage, jmp);
    return node;
}

/* WARNING! In this function we also need to check that we're using the
 * right parameter name.
 */
p_node * code_eval_ID_P_ID(p_node * param, p_node * field)
{
    p_node * p = p_storage_node_alloc(Storage);

    if (strcmp(field->lexeme, "monitor_type") == 0) {
        p->lexeme = "$0";
    } else if (strcmp(field->lexeme, "port") == 0) {
        p->lexeme = "$1";
    } else if (strcmp(field->lexeme, "group") == 0) {
        p->lexeme = "$2";
    } else if (strcmp(field->lexeme, "label") == 0) {
        p->lexeme = "$3";
    } else if (strcmp(field->lexeme, "hostname") == 0) {
        p->lexeme = "$4";
    } else if (strcmp(field->lexeme, "family") == 0) {
        p->lexeme = "$5";
    } else {
        fprintf(stderr, "invalid field name: %s\n", field->lexeme);
        exit(1);
    }

    return p;
}

p_node *p_node_create(const char * value)
{
    p_node *p = p_storage_node_alloc(Storage);
    p->lexeme = p_node_strdup(Storage, value);
    return p;
}

/* Main: open all files passed on standard input and parse all of them,
 * exiting in case of parse error. On successful parsing, print on
 * standard output the compiled code.
 */

void subsys_code_init(void)         /* To be called to initialize           */
{
    SymbolTable = sym_table_create();
    Storage = p_storage_create();
    Code = vm_code_create();
}

void code_generate(void)            /* Generate code on standard output     */
{
    unsigned hashval;
    unsigned offset;

    /* Don't generate code in case there was nothing in input. */
    if (Code->offset == 0) {
        fprintf(stderr, "[warning] Nothing of interesting to compile\n");
        return;
    }

    printf(".got\n");
    for (hashval = 0; hashval < SYM_TABLE_HASHSIZE; ++hashval) {
        sym_table_entry * e = SymbolTable->table[hashval];
        while ((e)) {
            printf("\t%s %d\n", e->name, e->instr->offset);
            e = e->next;
        }
    }

    printf("\n.code\n");
    for (offset = 0; offset < Code->offset; ++offset) {
        vm_instr *instr = &Code->base[offset];
        printf("\t%d ", offset);
        switch (instr->opcode) {
            case VM_NOP:
                printf("VM_NOP");
                break;
            case VM_EXEC:
                printf("VM_EXEC %s", instr->arg1);
                break;
            case VM_EQ:
                printf("VM_EQ %s %s", instr->arg1, instr->arg2);
                break;
            case VM_MAG:
                printf("VM_MAG %s %s", instr->arg1, instr->arg2);
                break;
            case VM_MIN:
                printf("VM_MIN %s %s", instr->arg1, instr->arg2);
                break;
            case VM_MAEQ:
                printf("VM_MAEQ %s %s", instr->arg1, instr->arg2);
                break;
            case VM_MIEQ:
                printf("VM_MIEQ %s %s", instr->arg1, instr->arg2);
                break;
            case VM_NEQ:
                printf("VM_NEQ %s %s", instr->arg1, instr->arg2);
                break;
            case VM_JTRUE:
                printf("VM_JTRUE %d", instr->location);
                break;
            case VM_JFALSE:
                printf("VM_JFALSE %d", instr->location);
                break;
            case VM_JMP:
                printf("VM_JMP %d", instr->location);
                break;
            case VM_RETURN:
                printf("VM_RETURN");
                break;
        }
        putchar('\n');
    }
}

int yywrap(void)
{
    return 1;
}

void yyerror (char const * str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}

int main(int argc, char ** argv)
{
    char * prog = argv[0];
    ++argv, --argc;

    subsys_code_init();

    if (argc > 0) {
        for (; argc > 0; ++argv, --argc) {
            yyin = fopen(argv[0], "r");
            if (!yyin) {
                fprintf(stderr, "%s: error - can't open %s", prog, argv[0]);
                exit(1);
            }
            yyparse();
            fclose(yyin);
        }
    } else {
        yyin = stdin;
        yyparse();
    }

    code_generate();

    return 0;
}

