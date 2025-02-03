#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct NoArvore {
    char nome[50];         // Tipo do nó (ex.: "declaracao", "var", etc.)
    char lexema[65];       // Lexema (se aplicável)
    int linha;             // Linha em que o token foi reconhecido
    struct NoArvore **filhos;
    int num_filhos;
} NoArvore;

NoArvore* parse(FILE *arquivo);
void imprimir_arvore_formatada(NoArvore *raiz, int indent);
void avance();
void casa(TokenType esperado);
NoArvore* criar_no_line(const char *nome, const char *lexema, int linha);
void adicionar_filho(NoArvore *pai, NoArvore *filho);
void destroi_arvore(NoArvore *raiz);

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
