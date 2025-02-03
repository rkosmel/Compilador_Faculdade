#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "semantico.h"

void scanner(const char *filename);
void parser_main(const char *filename);
void analisar_semantica(const char *filename);

int main(int argc, char *argv[]) {
    if (argc < 3) { // precisamos da flag e do nome do arquivo
        printf("Uso: %s [-l | -L | -p | -P | -s | -S] <arquivo>\n", argv[0]);
        return 1;
    }
    char *flag = argv[1];
    char *arquivo_nome = argv[2];
    switch(flag[1]) { // flag[0] é o traço
        case 'l':
        case 'L':
            scanner(arquivo_nome);
            break;
        case 'p':
        case 'P':
            parser_main(arquivo_nome);
            break;
        case 's':
        case 'S':
            analisar_semantica(arquivo_nome);
            break;
        default:
            printf("Erro: Flag inválida '%s'.\n", flag);
            printf("Uso: %s [-l | -L | -p | -P | -s | -S] <arquivo>\n", argv[0]);
            return 1;
    }
    return 0;
}

void scanner(const char *filename) {
    FILE *arquivo = fopen(filename, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }
    Buffer *buffer = allocate_buffer();
    Token token = next_token(buffer, arquivo);
    while(token.token != FIM_DE_ARQUIVO) {
        if(token.token == ERRO) {
            tratamento_de_erro(&token, buffer);
        } else {
            printf("Lexema: %s, Token: %s, Linha: %d\n", token.lexema, token_names[token.token], token.linha);
        }
        token = next_token(buffer, arquivo);
    }
    printf("\nFim do arquivo. Análise léxica concluída com sucesso!\n");
    fclose(arquivo);
    free(buffer);
}

void parser_main(const char *filename) {
    FILE *arquivo = fopen(filename, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }
    NoArvore *raiz = parse(arquivo);
    fclose(arquivo);
    printf("Árvore Sintática:\n");
    imprimir_arvore_formatada(raiz, 0);
    destroi_arvore(raiz);
}

void analisar_semantica(const char *filename) {
    FILE *arquivo = fopen(filename, "r");
    if(arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }
    NoArvore *raiz = parse(arquivo);
    fclose(arquivo);
    analiseSemantica(raiz);
    destroi_arvore(raiz);
}
