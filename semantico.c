#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantico.h"

/* Lista global para armazenar uma cópia dos símbolos inseridos (não serão liberados posteriormente) */
Simbolo *globalSymbols = NULL;

TabelaSimbolos* criarTabela(TabelaSimbolos* anterior) {
    TabelaSimbolos* tabela = (TabelaSimbolos*) malloc(sizeof(TabelaSimbolos));
    if (!tabela) {
        perror("Erro ao alocar tabela de símbolos");
        exit(EXIT_FAILURE);
    }
    tabela->simbolos = NULL;
    tabela->anterior = anterior;
    return tabela;
}

void destruirTabela(TabelaSimbolos* tabela) {
    Simbolo *atual = tabela->simbolos;
    while (atual != NULL) {
        Simbolo *temp = atual;
        atual = atual->prox;
        free(temp);
    }
    free(tabela);
}

/*
 * Insere um símbolo na tabela de símbolos e, ao mesmo tempo, adiciona uma cópia
 * à lista global (globalSymbols) para posterior impressão.
 */
int inserirSimbolo(TabelaSimbolos* tabela, const char* nome, Tipo tipo, int isFuncao, const char *escopo, int linha) {
    Simbolo* existente = tabela->simbolos;
    while (existente != NULL) {
        if (strcmp(existente->nome, nome) == 0)
            return 0; // Símbolo já existe neste escopo
        existente = existente->prox;
    }
    Simbolo* novo = (Simbolo*) malloc(sizeof(Simbolo));
    if (!novo) {
        perror("Erro ao alocar símbolo");
        exit(EXIT_FAILURE);
    }
    strcpy(novo->nome, nome);
    strcpy(novo->escopo, escopo);
    novo->tipo = tipo;
    novo->isFuncao = isFuncao;
    novo->linha = linha;
    novo->prox = tabela->simbolos;
    tabela->simbolos = novo;
    
    /* Cria uma cópia do símbolo para a lista global */
    Simbolo *copia = (Simbolo*) malloc(sizeof(Simbolo));
    if (!copia) {
        perror("Erro ao alocar símbolo global");
        exit(EXIT_FAILURE);
    }
    strcpy(copia->nome, nome);
    strcpy(copia->escopo, escopo);
    copia->tipo = tipo;
    copia->isFuncao = isFuncao;
    copia->linha = linha;
    copia->nextGlobal = globalSymbols;
    globalSymbols = copia;
    
    return 1;
}

Simbolo* buscarSimbolo(TabelaSimbolos* tabela, const char* nome) {
    TabelaSimbolos* atual = tabela;
    while (atual != NULL) {
        Simbolo* simbolo = atual->simbolos;
        while (simbolo != NULL) {
            if (strcmp(simbolo->nome, nome) == 0)
                return simbolo;
            simbolo = simbolo->prox;
        }
        atual = atual->anterior;
    }
    return NULL;
}

/*
 * Função recursiva que percorre a AST e realiza a análise semântica.
 * O parâmetro "escopo" indica o escopo corrente (por exemplo, "global" ou o nome da função).
 */
void analisarNo(NoArvore* no, TabelaSimbolos* tabela, const char *escopo) {
    if (no == NULL)
        return;
    
    /* Se o nó é uma declaração (global ou variável local) */
    if (strcmp(no->nome, "declaracao") == 0) {
        int isFuncao = 0;
        char funcNome[50] = "";
        for (int i = 0; i < no->num_filhos; i++) {
            if (strcmp(no->filhos[i]->nome, "funcao_parametros") == 0) {
                isFuncao = 1;
            }
            if (strcmp(no->filhos[i]->nome, "id") == 0) {
                strcpy(funcNome, no->filhos[i]->lexema);
            }
        }
        if (isFuncao) {
            /* Insere a função na tabela (global) */
            if (!inserirSimbolo(tabela, funcNome, (funcNome[0]=='v' ? TIPO_VOID : TIPO_INT), 1, "global", no->linha)) {
                printf("\nERRO SEMÂNTICO: Redeclaração da função '%s'. LINHA: %d\n", funcNome, no->linha);
            }
            /* Para os parâmetros e corpo da função, o novo escopo é o nome da função */
            escopo = funcNome;
        }
        /* Se for declaração de variável (não função) */
        if (!isFuncao) {
            for (int i = 0; i < no->num_filhos; i++) {
                if (strcmp(no->filhos[i]->nome, "id") == 0) {
                    char *id = no->filhos[i]->lexema;
                    Tipo tipo = TIPO_ERRO;
                    for (int j = 0; j < no->num_filhos; j++) {
                        if (strcmp(no->filhos[j]->nome, "tipo") == 0) {
                            if (strcmp(no->filhos[j]->lexema, "int") == 0)
                                tipo = TIPO_INT;
                            else if (strcmp(no->filhos[j]->lexema, "void") == 0)
                                tipo = TIPO_VOID;
                            break;
                        }
                    }
                    if (!inserirSimbolo(tabela, id, tipo, 0, escopo, no->linha)) {
                        printf("\nERRO SEMÂNTICO: Redeclaração da variável '%s'. LINHA: %d\n", id, no->linha);
                    }
                    break;
                }
            }
        }
    }
    
    /* Se o nó representa um parâmetro, insere-o e NÃO processa seus filhos */
    if (strcmp(no->nome, "parametro") == 0) {
        char *tipoStr = NULL;
        char *id = NULL;
        for (int i = 0; i < no->num_filhos; i++) {
            if (strcmp(no->filhos[i]->nome, "tipo") == 0) {
                tipoStr = no->filhos[i]->lexema;
            } else if (strcmp(no->filhos[i]->nome, "var") == 0) {
                id = no->filhos[i]->lexema;
            }
        }
        Tipo tipo = TIPO_ERRO;
        if (tipoStr != NULL) {
            if (strcmp(tipoStr, "int") == 0)
                tipo = TIPO_INT;
            else if (strcmp(tipoStr, "void") == 0)
                tipo = TIPO_VOID;
        }
        if (id != NULL) {
            if (!inserirSimbolo(tabela, id, tipo, 0, escopo, no->linha)) {
                printf("\nERRO SEMÂNTICO: Redeclaração do parâmetro '%s'. LINHA: %d\n", id, no->linha);
            }
        }
        return; // NÃO processa os filhos deste nó
    }
    
    /* Se o nó representa o uso de uma variável */
    if (strcmp(no->nome, "var") == 0) {
        Simbolo* s = buscarSimbolo(tabela, no->lexema);
        if (s == NULL) {
            printf("\nERRO SEMÂNTICO: Variável '%s' não declarada. LINHA: %d\n", no->lexema, no->linha);
        }
    }
    
    /* Se o nó cria um novo escopo (ex.: corpo de função ou bloco composto) */
    int novoEscopo = 0;
    if (strcmp(no->nome, "composto_decl") == 0 || strcmp(no->nome, "funcao_parametros") == 0) {
        tabela = criarTabela(tabela);
        novoEscopo = 1;
    }
    
    /* Percorre recursivamente os filhos */
    for (int i = 0; i < no->num_filhos; i++) {
        analisarNo(no->filhos[i], tabela, escopo);
    }
    
    if (novoEscopo) {
        destruirTabela(tabela);
    }
}

void analiseSemantica(NoArvore *raiz) {
    printf("Iniciando análise semântica...\n");
    TabelaSimbolos* tabelaGlobal = criarTabela(NULL);
    analisarNo(raiz, tabelaGlobal, "global");
    
    /* Imprime a tabela de símbolos no formato solicitado:
       <Nome_ID>;<Escopo>;<Tipo_ID>;<Tipo_dado>;<Linha>
       Onde Tipo_ID é "FUNCAO" se isFuncao==1, caso contrário "VARIAVEL" */
    printf("\n# TABELA DE SÍMBOLOS\n");
    Simbolo* sym = globalSymbols;
    while (sym != NULL) {
        const char* tipoID = sym->isFuncao ? "FUNCAO" : "VARIAVEL";
        const char* tipoDado;
        if (sym->tipo == TIPO_INT)
            tipoDado = "int";
        else if (sym->tipo == TIPO_VOID)
            tipoDado = "void";
        else
            tipoDado = "erro";
        printf("%s;%s;%s;%s;%d\n", sym->nome, sym->escopo, tipoID, tipoDado, sym->linha);
        sym = sym->nextGlobal;
    }
    
    /* Agora libera a tabela global; a lista global (globalSymbols) foi copiada e não será liberada aqui */
    destruirTabela(tabelaGlobal);
    printf("\nAnálise semântica concluída com sucesso!\n");
}
