#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

Token token;
Buffer *buffer;
FILE *arquivo;

NoArvore* parse(FILE *arquivo_passado) {
    arquivo = arquivo_passado;
    buffer = allocate_buffer();
    printf("Arquivo atribuído corretamente no parser: %p\n", (void*)arquivo);
    avance();  // Lê o primeiro token
    NoArvore* raiz = programa();
    printf("Análise sintática concluída com sucesso!\n");
    return raiz;
}

NoArvore* criar_no(const char *nome) {
    NoArvore* no = (NoArvore*) malloc(sizeof(NoArvore));
    strcpy(no->nome, nome);
    no->filhos = NULL;
    no->num_filhos = 0;
    return no;
}

void adicionar_filho(NoArvore *pai, NoArvore *filho) {
    if (pai == NULL || filho == NULL) return;
    pai->num_filhos++;
    pai->filhos = (NoArvore**) realloc(pai->filhos, pai->num_filhos * sizeof(NoArvore*));
    pai->filhos[pai->num_filhos - 1] = filho;
}

void imprimir_arvore(NoArvore *raiz, int nivel) {
    if (raiz == NULL) return;
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("%s(\n", raiz->nome);
    for (int i = 0; i < raiz->num_filhos; i++) imprimir_arvore(raiz->filhos[i], nivel + 1);
    for (int i = 0; i < nivel; i++) printf("    ");
    printf(")\n");
}

void avance() {
    printf("[DEBUG] Avançando token: %s (Lexema: %s) na linha %d\n",
           token_names[token.token], token.lexema, buffer->line_number);
    token = next_token(buffer, arquivo);
}

void casa(TokenType esperado) {
    if (token.token == esperado) {
        avance();
    } else {
        printf("Erro de sintaxe: token esperado %s, token encontrado %s. LINHA: %d\n", token_names[esperado], token_names[token.token], buffer->line_number);
        exit(EXIT_FAILURE);
    }
}

NoArvore* programa() {
    NoArvore* no = criar_no("programa");
    adicionar_filho(no, declaracao_lista());
    casa(FIM_DE_ARQUIVO);
    return no;
}

NoArvore* declaracao_lista() {
    NoArvore* no = criar_no("declaracao_lista");
    while (token.token == INT || token.token == VOID) { // pois podem ter várias declarações
        adicionar_filho(no, declaracao());
    }
    return no;
}

NoArvore* declaracao() {
    NoArvore* no = criar_no("declaracao");
    if (token.token == INT)
        casa(INT);
    else if (token.token == VOID)
        casa(VOID);
    else {
        printf("Erro de sintaxe: token esperado INT ou VOID, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }

    // agora o token que esperamos é um ID
    if (token.token == ID) {
        casa(ID);
    } else {
        printf("Erro de sintaxe: token esperado ID, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }

    // agora pode ser um ';' um array ou uma função
    if (token.token == PONTO_VIRGULA) {
        casa(PONTO_VIRGULA);
    } else if (token.token == ABRE_COLCHETES) {
        casa(ABRE_COLCHETES);
        casa(NUM);
        casa(FECHA_COLCHETES);
        casa(PONTO_VIRGULA);
    } else if (token.token == ABRE_PARENTESES) {
        adicionar_filho(no, funcao_parametros());
    } else {
        printf("Erro de sintaxe: token esperado ;, [ ou (, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }

    return no;
}

NoArvore* funcao_parametros() {
    NoArvore* no = criar_no("funcao_parametros");
    casa(ABRE_PARENTESES);
    if (token.token == VOID) {
        casa(VOID);
    } else if (token.token == INT) { // pois pode ser uma fecha parenteses (caso não tenha parâmetros)
        casa(INT);
        adicionar_filho(no, lista_parametros());
    }
    casa(FECHA_PARENTESES);
    adicionar_filho(no, composto_decl());
    return no;
}

NoArvore* lista_parametros() {
    NoArvore* no = criar_no("lista_parametros");
    adicionar_filho(no, parametro());
    while (token.token == VIRGULA) {
        casa(VIRGULA);
        adicionar_filho(no, parametro());
    }
    return no;
}

NoArvore* parametro() {
    NoArvore* no = criar_no("parametro");
    if (token.token == INT) {
        casa(INT);
        adicionar_filho(no, var());
    } else if (token.token == ID) {
        casa(ID);
    } else if (token.token == NUM) {
        casa(NUM);
    } else {
        printf("Erro de sintaxe: token esperado INT, ID ou NUM, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

NoArvore* composto_decl() {
    NoArvore* no = criar_no("composto_decl");
    casa(ABRE_CHAVES);
    adicionar_filho(no, local_declaracoes());
    adicionar_filho(no, statement_lista()); // !
    casa(FECHA_CHAVES);
    return no;
}

NoArvore* local_declaracoes() {
    NoArvore* no = criar_no("local_declaracoes");
    while (token.token == INT || token.token == VOID) {
        adicionar_filho(no, declaracao());
    }
    return no;
}

NoArvore* statement_lista() {
    NoArvore* no = criar_no("statement_lista");
    while (token.token == IF || token.token == WHILE || token.token == RETURN || token.token == ABRE_CHAVES || token.token == ID || token.token == NUM || token.token == PONTO_VIRGULA) {
        adicionar_filho(no, statement());
    }
    return no;
}

NoArvore* statement() {
    NoArvore* no = criar_no("statement");
    if (token.token == IF) {
        adicionar_filho(no, selecao_decl());
    } else if (token.token == WHILE) {
        adicionar_filho(no, iteracao_decl());
    } else if (token.token == RETURN) {
        adicionar_filho(no, retorno_decl());
    } else if (token.token == ABRE_CHAVES) {
        adicionar_filho(no, composto_decl());
    } else if (token.token == ID || token.token == NUM) {
        adicionar_filho(no, expressao());
    } else if (token.token == PONTO_VIRGULA) {
        casa(PONTO_VIRGULA);
    } else {
        printf("Erro de sintaxe: token esperado IF, WHILE, RETURN, {, ID, NUM ou ;, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

NoArvore* selecao_decl() {
    NoArvore* no = criar_no("selecao_decl");
    casa(IF);
    casa(ABRE_PARENTESES);
    adicionar_filho(no, expressao());
    casa(FECHA_PARENTESES);
    adicionar_filho(no, statement());
    if (token.token == ELSE) {
        casa(ELSE);
        adicionar_filho(no, statement());
    }
    return no;
}

NoArvore* iteracao_decl() {
    NoArvore* no = criar_no("iteracao_decl");
    casa(WHILE);
    casa(ABRE_PARENTESES);
    adicionar_filho(no, expressao());
    casa(FECHA_PARENTESES);
    adicionar_filho(no, statement());
    return no;
}

NoArvore* retorno_decl() {
    NoArvore* no = criar_no("retorno_decl");
    casa(RETURN);
    if (token.token == PONTO_VIRGULA) {
        casa(PONTO_VIRGULA);
    } else {
        adicionar_filho(no, var());
        casa(PONTO_VIRGULA);
    }
    return no;
}

NoArvore* expressao() {
    NoArvore* no = criar_no("expressao");

    if (token.token != NUM) {
        adicionar_filho(no, var());
    }

    if (token.token == ATRIBUICAO) {
        casa(ATRIBUICAO);
        adicionar_filho(no, expressao());
    } else if (token.token == ABRE_PARENTESES) {
        adicionar_filho(no, args());
    } else {
        adicionar_filho(no, expressao_simples());
    }
    return no;
}

NoArvore* expressao_simples() {
    NoArvore* no = criar_no("expressao_simples");
    adicionar_filho(no, soma());
    if (token.token == MENOR || token.token == MENOR_IGUAL || token.token == MAIOR || token.token == MAIOR_IGUAL || token.token == IGUAL || token.token == DIFERENTE || token.token == MENOS || token.token == MAIS) {
        avance();
        adicionar_filho(no, soma());
    }
    return no;
}

NoArvore* soma() {
    NoArvore* no = criar_no("soma");
    adicionar_filho(no, termo());
    while (token.token == MAIS || token.token == MENOS) {
        avance();
        adicionar_filho(no, termo());
    }
    return no;
}

NoArvore* termo() {
    NoArvore* no = criar_no("termo");
    adicionar_filho(no, fator());
    while (token.token == VEZES || token.token == DIVISAO) {
        avance();
        adicionar_filho(no, fator());
    }
    return no;
}

NoArvore* fator() {
    NoArvore* no = criar_no("fator");
    if (token.token == ABRE_PARENTESES) {
        casa(ABRE_PARENTESES);
        adicionar_filho(no, expressao());
        casa(FECHA_PARENTESES);
    } else if (token.token == ID) {
        avance();
        if (token.token == ABRE_COLCHETES) {
            avance();
            adicionar_filho(no, expressao());
            casa(FECHA_COLCHETES);
        } else if (token.token == ABRE_PARENTESES) {
            adicionar_filho(no, args());
        }
    } else if (token.token == NUM) {
        casa(NUM);
    } else {
        adicionar_filho(no, relacional());
    }
    return no;
}

NoArvore* relacional() {
    NoArvore* no = criar_no("relacional");
    switch (token.token) {
        case MENOR:
            casa(MENOR);
            break;
        case MENOR_IGUAL:
            casa(MENOR_IGUAL);
            break;
        case MAIOR:
            casa(MAIOR);
            break;
        case MAIOR_IGUAL:
            casa(MAIOR_IGUAL);
            break;
        case IGUAL:
            casa(IGUAL);
            break;
        case DIFERENTE:
            casa(DIFERENTE);
            break;
        default:
            printf("Erro de sintaxe: token esperado <, <=, >, >=, == ou !=, token encontrado %s.\n", token_names[token.token]);
            exit(EXIT_FAILURE);
    }
    adicionar_filho(no, expressao());
    return no;
}

NoArvore* args() {
    NoArvore* no = criar_no("args");
    casa(ABRE_PARENTESES);
    if (token.token != FECHA_PARENTESES) {
        adicionar_filho(no, lista_args());
    }
    casa(FECHA_PARENTESES);
    return no;
}

NoArvore* lista_args() {
    NoArvore* no = criar_no("lista_args");
    adicionar_filho(no, expressao());
    while (token.token == VIRGULA) {
        casa(VIRGULA);
        if (token.token == NUM) {
            casa(NUM);
        } else {
            adicionar_filho(no, expressao());
        }
    }
    return no;
}

NoArvore* var() {
    NoArvore* no = criar_no("var");
    casa(ID);

    if (token.token == ABRE_COLCHETES) {
        casa(ABRE_COLCHETES);
        adicionar_filho(no, expressao());
        casa(FECHA_COLCHETES);
    }
    return no;
}