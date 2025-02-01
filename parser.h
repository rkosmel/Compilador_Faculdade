#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Estrutura de um nó da árvore sintática
typedef struct NoArvore {
    char nome[50];         // Nome do nó (ex: "if", "assign_expr", "return_stmt")
    struct NoArvore** filhos;
    int num_filhos;
} NoArvore;

// Funções principais do parser
NoArvore* parse(FILE *arquivo);
void imprimir_arvore(NoArvore *raiz, int nivel);

// Funções auxiliares
void avance();
void casa(TokenType esperado);
void adicionar_filho(NoArvore *pai, NoArvore *filho);
NoArvore* criar_no(const char *nome);

// Funções de análise sintática
NoArvore* programa();
NoArvore* declaracao_lista();
NoArvore* declaracao();
NoArvore* funcao_parametros();
NoArvore* lista_parametros();
NoArvore* parametro();
NoArvore* composto_decl();
NoArvore* local_declaracoes();
NoArvore* statement_lista();
NoArvore* statement();
NoArvore* selecao_decl();
NoArvore* iteracao_decl();
NoArvore* retorno_decl();
NoArvore* expressao();
NoArvore* expressao_simples();
NoArvore* soma();
NoArvore* termo();
NoArvore* fator();
NoArvore* relacional();
NoArvore* args();
NoArvore* var();
NoArvore* lista_args();
NoArvore* var();

#endif // PARSER_H