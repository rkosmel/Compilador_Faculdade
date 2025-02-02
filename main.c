#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

void scanner(const char *filename);
void parser(const char *filename);
void analisar_semantica(const char *filename);

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Uso: %s [-1 | -L | -p | -P | -s | -S] <arquivo>\n", argv[0]);
        return 1;
    }

    char *flag = argv[1];
    char *arquivo_nome = argv[2];

    // Escolha a operação com base na flag
    switch (flag[1]) {  // Pegamos o segundo caractere da flag para identificar
        case '1': 
        case 'L': 
            scanner(arquivo_nome);
            break;

        case 'p': 
        case 'P': 
            parser(arquivo_nome);
            break;

        case 's': 
        case 'S': 
            analisar_semantica(arquivo_nome);
            break;

        default:
            printf("Erro: Flag inválida '%s'.\n", flag);
            printf("Uso correto: %s [-1 | -L | -p | -P | -s | -S] <arquivo>\n", argv[0]);
            return 1;
    }

    return 0;
}

// Função para análise léxica
void scanner(const char *filename) {
    FILE *arquivo = fopen(filename, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }

    Buffer *buffer = allocate_buffer(); // Agora retorna um ponteiro

    Token token = next_token(buffer, arquivo); // Passa buffer como ponteiro
    while (token.token != FIM_DE_ARQUIVO) {
        if (token.token == ERRO) {
            tratamento_de_erro(&token, buffer); // Ajustado para ponteiro
        } else {
            printf("Lexema: %s, Token: %s, Linha: %d\n", token.lexema, token_names[token.token], token.linha);
        }
        token = next_token(buffer, arquivo);
    }

    printf("Fim do arquivo. Análise léxica concluída com sucesso!\n");

    fclose(arquivo);
    free(buffer); // Agora correto
}

void parser(const char *filename) {
    FILE *arquivo = fopen(filename, "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", filename);
        return;
    }

    printf("Arquivo %s aberto com sucesso. Endereço: %p\n", filename, (void*)arquivo); // Debugging

    printf("Iniciando análise sintática...\n");
    NoArvore* raiz = parse(arquivo); // Agora passamos o arquivo corretamente

    if (raiz != NULL) {
        imprimir_arvore(raiz, 0);
    } else {
        printf("Erro: A análise sintática falhou e retornou NULL.\n");
    }

    fclose(arquivo);
}

void analisar_semantica(const char *filename) {
    printf("Análise semântica ainda não implementada para o arquivo: %s\n", filename);
}
