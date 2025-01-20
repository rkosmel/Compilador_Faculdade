#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    FILE *arquivo = fopen("codigo.c", "r");

    if (arquivo == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    Buffer buffer = allocate_buffer(); // Aloca o buffer

    Token token = next_token(&buffer, arquivo);
    while (token.token != EOF)
    {
        if (token.token == ERRO) {
            // printf("ERRO LEXICO: \"%s\" INVALIDO [linha: %d], COLUNA %d.\n", token.lexema, token.linha, buffer.position);
            tratamento_de_erro(token);
            token = next_token(&buffer, arquivo); // Obtenha o próximo token
        }
        else {
            printf("Lexema: %s, Token: %s, Linha: %d\n", token.lexema, token_names[token.token], token.linha);
            token = next_token(&buffer, arquivo); // Obtenha o próximo token
        }
    }

    fclose(arquivo);

    return 0;
}
