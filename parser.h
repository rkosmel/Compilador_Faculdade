#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

/* Estrutura da árvore sintática (AST) */
typedef struct NoArvore {
    char nome[50];         // Tipo do nó (ex.: "declaracao", "var", "expressao", etc.)
    char lexema[65];       // Lexema real (para terminais, ex.: identificador)
    int linha;             // Linha onde o token foi reconhecido
    struct NoArvore **filhos;
    int num_filhos;
} NoArvore;

/* Protótipos */
NoArvore* parse(FILE *arquivo);
void imprimir_arvore(NoArvore *raiz, int nivel);

void avance();
void casa(TokenType esperado);
NoArvore* criar_no_line(const char *nome, const char *lexema, int linha);
void adicionar_filho(NoArvore *pai, NoArvore *filho);

/* Protótipos das funções da gramática */
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
NoArvore* lista_args();
NoArvore* var();

#endif // PARSER_H
