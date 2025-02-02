#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "parser.h"

typedef enum {
    TIPO_INT,
    TIPO_VOID,
    TIPO_ERRO
} Tipo;

typedef struct Simbolo {
    char nome[50];
    char escopo[50];
    Tipo tipo;
    int isFuncao;
    int linha;
    struct Simbolo *prox;
    struct Simbolo *nextGlobal;
} Simbolo;

typedef struct TabelaSimbolos {
    Simbolo *simbolos;
    struct TabelaSimbolos *anterior;
} TabelaSimbolos;

TabelaSimbolos* criarTabela(TabelaSimbolos* anterior);
void destruirTabela(TabelaSimbolos* tabela);
int inserirSimbolo(TabelaSimbolos* tabela, const char* nome, Tipo tipo, int isFuncao, const char *escopo, int linha);
Simbolo* buscarSimbolo(TabelaSimbolos* tabela, const char* nome);
void analiseSemantica(NoArvore *raiz);

#endif // SEMANTICO_H
