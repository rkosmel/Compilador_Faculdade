#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

void scanner(const char *filename);
void analisar_sintatica(const char *filename);
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
            analisar_sintatica(arquivo_nome);
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
    while (token.token != EOF) {
        if (token.token == ERRO) {
            tratamento_de_erro(&token, buffer); // Ajustado para ponteiro
        } else {
            printf("Lexema: %s, Token: %s, Linha: %d\n", token.lexema, token_names[token.token], token.linha);
        }
        token = next_token(buffer, arquivo); // Correção aqui também
    }

    printf("Fim do arquivo. Análise léxica concluída com sucesso!\n");

    fclose(arquivo);
    free(buffer); // Agora correto
}

// Funções de análise sintática e semântica (devem ser implementadas depois)
void analisar_sintatica(const char *filename) {
    printf("Análise sintática ainda não implementada para o arquivo: %s\n", filename);
}

void analisar_semantica(const char *filename) {
    printf("Análise semântica ainda não implementada para o arquivo: %s\n", filename);
}
