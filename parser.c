#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

Token token;
Buffer *buffer;
FILE *arquivo;

NoArvore* parse(FILE *arquivo);

// Declarações antecipadas das funções para evitar erros de compilação
void adicionar_filho(NoArvore *pai, NoArvore *filho);
NoArvore* expression_stmt();
NoArvore* return_stmt();
NoArvore* criar_no(const char *nome); 
void advance();
void match(TokenType esperado);  

// Função para criar um novo nó da árvore sintática
NoArvore* criar_no(const char *nome) {
    NoArvore* no = (NoArvore*) malloc(sizeof(NoArvore));
    if (no == NULL) {
        printf("Erro: Falha na alocação de memória para nó da árvore.\n");
        exit(1);
    }
    strcpy(no->nome, nome);
    no->filhos = NULL;
    no->num_filhos = 0;
    return no;
}

// Obtém o próximo token do lexer
void advance() {
    if (token.token == FIM_DE_ARQUIVO) {
        printf("Aviso: Tentativa de avançar após FIM_DE_ARQUIVO. Ignorando avanço.\n");
        return; // Impede que continue avançando
    }

    token = next_token(buffer, arquivo);

    if (token.token == FIM_DE_ARQUIVO) {
        printf("Token recebido: FIM_DE_ARQUIVO\n");
    } else {
        printf("Token recebido: %s (Lexema: '%s', Linha: %d)\n", token_names[token.token], token.lexema, token.linha);
    }
}

// Verifica se o token atual é o esperado
void match(TokenType esperado) {
    printf("match(): Esperado: %s, Encontrado: %s (Linha: %d)\n", 
           token_names[esperado], token_names[token.token], token.linha);

    if (token.token == esperado) {
        advance();
    } else {
        printf("Erro sintático na linha %d: esperado '%s', encontrado '%s'.\n",
               token.linha, token_names[esperado], token_names[token.token]);
        exit(1);
    }
}


NoArvore* type_specifier() {
    NoArvore* no = criar_no("type_specifier");

    if (token.token == INT) {
        match(INT);
        adicionar_filho(no, criar_no("int"));
    } else if (token.token == VOID) {
        match(VOID);
        adicionar_filho(no, criar_no("void"));
    } else {
        printf("Erro sintático na linha %d: esperado 'int' ou 'void'.\n", token.linha);
        exit(1);
    }

    return no;
}

// Função para adicionar filhos a um nó
void adicionar_filho(NoArvore *pai, NoArvore *filho) {
    if (pai == NULL || filho == NULL) {
        printf("Erro: Tentativa de adicionar filho a um nó nulo.\n");
        return;
    }

    pai->num_filhos++;
    pai->filhos = (NoArvore**) realloc(pai->filhos, pai->num_filhos * sizeof(NoArvore*));
    if (pai->filhos == NULL) {
        printf("Erro: Falha na realocação de memória para filhos.\n");
        exit(1);
    }

    pai->filhos[pai->num_filhos - 1] = filho;
}

// Imprime a árvore sintática recursivamente
void imprimir_arvore(NoArvore *raiz, int nivel) {
    if (raiz == NULL) return;  // Evita acessar um nó inválido

    for (int i = 0; i < nivel; i++) {
        printf("    ");
    }
    printf("%s(\n", raiz->nome);

    for (int i = 0; i < raiz->num_filhos; i++) {
        if (raiz->filhos[i] != NULL) {  // Evita acessar filhos nulos
            imprimir_arvore(raiz->filhos[i], nivel + 1);
        }
    }

    for (int i = 0; i < nivel; i++) {
        printf("    ");
    }
    printf(")\n");
}

NoArvore* parse(FILE *arquivo_passado) {
    arquivo = arquivo_passado;
    buffer = allocate_buffer();

    printf("Arquivo atribuído corretamente no parser: %p\n", (void*)arquivo);

    advance();  // Garante que o primeiro token seja lido corretamente
    NoArvore* raiz = program();

    if (raiz == NULL) {
        printf("Erro: A árvore sintática não foi construída corretamente.\n");
        exit(1);
    }

    printf("Análise sintática concluída com sucesso!\n");
    return raiz;
}

// Implementação das funções para cada regra da gramática
NoArvore* program() {
    NoArvore* no = criar_no("program");
    adicionar_filho(no, decl_list());
    return no;
}

NoArvore* decl_list() {
    NoArvore* no = criar_no("decl_list");
    while (token.token == INT || token.token == VOID) {
        adicionar_filho(no, decl());
    }
    return no;
}

NoArvore* decl() {
    NoArvore* no = criar_no("decl");

    NoArvore* tipo_no = type_specifier();
    adicionar_filho(no, tipo_no);

    NoArvore* id_no = criar_no(token.lexema);  // ✅ Captura o ID antes de consumi-lo
    match(ID);

    if (token.token == ABRE_PARENTESES) {  // Se for uma função
        adicionar_filho(no, fun_decl(id_no));
    } else {
        adicionar_filho(no, var_decl(id_no));
    }

    return no;
}

NoArvore* var_decl() {
    NoArvore* no = criar_no("var_decl");

    adicionar_filho(no, type_specifier()); // Tipo da variável

    NoArvore* id_no = criar_no(token.lexema); // Adiciona o nome da variável
    adicionar_filho(no, id_no);
    match(ID);

    match(PONTO_VIRGULA); // Finaliza a declaração

    return no;
}

NoArvore* fun_decl(NoArvore* id_no) {  // Recebe o nó do ID já consumido
    NoArvore* no = criar_no("fun_decl");

    adicionar_filho(no, id_no);  // ✅ Adiciona corretamente o ID (main)

    match(ABRE_PARENTESES);
    adicionar_filho(no, params());
    match(FECHA_PARENTESES);
    
    adicionar_filho(no, compound_stmt());

    return no;
}

NoArvore* params() {
    NoArvore* no = criar_no("params");

    if (token.token == VOID) {
        match(VOID);
        // ⚠️ Se a função tem apenas "void" como parâmetro, já podemos retornar
        return no;
    } else if (token.token == INT) {
        adicionar_filho(no, param_list());
    } else if (token.token == FECHA_PARENTESES) {
        // ⚠️ Se chegamos em ")", os parâmetros já foram consumidos corretamente
        return no;
    } else {
        printf("Erro sintático na linha %d: esperado tipo de parâmetro ('int' ou 'void'), mas encontrado '%s'.\n",
               token.linha, token_names[token.token]);
        exit(1);
    }

    return no;
}

NoArvore* param_list() {
    NoArvore* no = criar_no("param_list");

    do {
        adicionar_filho(no, param());

        if (token.token == VIRGULA) {
            match(VIRGULA);
        } else if (token.token == FECHA_PARENTESES) {
            // ⚠️ Se encontramos ")", significa que os parâmetros acabaram
            return no;
        }
    } while (token.token == INT || token.token == VOID);

    return no;
}

NoArvore* param() {
    NoArvore* no = criar_no("param");
    adicionar_filho(no, type_specifier());
    match(ID);
    return no;
}

NoArvore* compound_stmt() {
    NoArvore* no = criar_no("compound_stmt");
    match(ABRE_CHAVES);
    adicionar_filho(no, local_decl());
    adicionar_filho(no, stmt_list());
    
    printf("compound_stmt(): Antes de consumir FECHA_CHAVES, token atual: %s\n", 
           token_names[token.token]);

    match(FECHA_CHAVES); // Verifica se estamos consumindo corretamente

    return no;
}

NoArvore* local_decl() {
    NoArvore* no = criar_no("local_decl");

    while (token.token == INT || token.token == VOID) {  // Se houver uma variável declarada
        adicionar_filho(no, var_decl());
    }

    return no;
}

NoArvore* stmt_list() {
    NoArvore* no = criar_no("stmt_list");

    while (token.token != FECHA_CHAVES && token.token != FIM_DE_ARQUIVO) {
        printf("stmt_list(): Processando stmt() com token: %s (Linha %d)\n", token_names[token.token], token.linha);
        adicionar_filho(no, stmt());
    }

    printf("stmt_list(): Saindo com token: %s (Linha %d)\n", token_names[token.token], token.linha);
    return no;
}

NoArvore* stmt() {
    NoArvore* no = criar_no("stmt");

    if (token.token == ID || token.token == NUM) {
        printf("stmt(): Chamando expression_stmt() (Linha %d)\n", token.linha);
        adicionar_filho(no, expression_stmt());
    } else if (token.token == RETURN) {
        printf("stmt(): Chamando return_stmt() (Linha %d)\n", token.linha);
        adicionar_filho(no, return_stmt());
        return no;  // ✅ Retorna imediatamente para evitar consumir outro `PONTO_VIRGULA`
    } else {
        printf("Erro sintático na linha %d: comando inválido.\n", token.linha);
        exit(1);
    }

    return no;
}

NoArvore* expression_stmt() {
    NoArvore* no = criar_no("expression_stmt");

    NoArvore* var_no = criar_no("var");
    adicionar_filho(var_no, criar_no(token.lexema)); // Nome da variável
    match(ID);

    if (token.token == ATRIBUICAO) {  // Detecta atribuição
        NoArvore* assign_no = criar_no("assign_expr");
        adicionar_filho(assign_no, var_no);
        
        match(ATRIBUICAO);
        
        NoArvore* value_no = criar_no(token.lexema); // Valor atribuído
        adicionar_filho(assign_no, value_no);
        match(NUM);

        adicionar_filho(no, assign_no);
    }

    match(PONTO_VIRGULA);
    return no;
}

NoArvore* return_stmt() {
    NoArvore* no = criar_no("return_stmt");
    match(RETURN);

    if (token.token != PONTO_VIRGULA) {  
        printf("return_stmt(): Chamando expression_stmt() (Linha %d)\n", token.linha);
        adicionar_filho(no, expression_stmt());
    }

    // ✅ Só consome `;` se estiver presente, e já sai imediatamente
    if (token.token == PONTO_VIRGULA) {
        printf("return_stmt(): Chamando match(PONTO_VIRGULA) (Linha %d)\n", token.linha);
        match(PONTO_VIRGULA);
    }

    printf("return_stmt(): Concluído corretamente (Linha %d)\n", token.linha);
    return no;
}
