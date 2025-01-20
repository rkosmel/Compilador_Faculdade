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
            tratamento_de_erro(&token, &buffer);
            token = next_token(&buffer, arquivo); // Obtenha o próximo token
        }
        else {
            printf("Lexema: %s, Token: %s, Linha: %d\n", token.lexema, token_names[token.token], token.linha);
            token = next_token(&buffer, arquivo); // Obtenha o próximo token
        }
    }

    printf("Fim do arquivo. Análise léxica concluída com sucesso!\n");

    fclose(arquivo);

    return 0;
}
