#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Estrutura para representar os nós da árvore de análise sintática
typedef struct NoArvore {
    char nome[50];                // Nome do nó (ex: "Declaração", "Expressão", etc.)
    struct NoArvore **filhos;     // Vetor de ponteiros para filhos
    int num_filhos;               // Número de filhos
} NoArvore;

// Função principal para iniciar a análise sintática
NoArvore* parse();

// Funções para imprimir a árvore sintática
void imprimir_arvore(NoArvore *raiz, int nivel);

// Funções recursivas para cada regra da gramática
NoArvore* program();
NoArvore* decl_list();
NoArvore* decl();
NoArvore* var_decl();
NoArvore* fun_decl();
NoArvore* type_specifier();
NoArvore* params();
NoArvore* param_list();
NoArvore* param();
NoArvore* compound_stmt();
NoArvore* local_decl();
NoArvore* stmt_list();
NoArvore* stmt();
NoArvore* expression();
NoArvore* term();
NoArvore* factor();

#endif
