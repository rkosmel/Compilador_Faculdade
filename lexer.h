#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

typedef struct {
    char buffer[BUFFER_SIZE];
    int position;
    int line_number;
    int coluna;
    int line_advanced;
} Buffer;

Buffer *allocate_buffer();
char get_next_char(Buffer *buffer, FILE *arquivo);
extern const char *token_names[];
int is_symbol(char c);

typedef enum {
    ELSE, IF, INT, RETURN, VOID, WHILE,
    MAIS, MENOS, VEZES, DIVISAO, MENOR, MENOR_IGUAL, MAIOR, MAIOR_IGUAL,
    IGUAL, DIFERENTE, ATRIBUICAO, PONTO_VIRGULA, VIRGULA,
    ABRE_PARENTESES, FECHA_PARENTESES, ABRE_COLCHETES, FECHA_COLCHETES,
    ABRE_CHAVES, FECHA_CHAVES,
    ID, NUM, FIM_DE_ARQUIVO, ERRO
} TokenType;

typedef struct {
    char lexema[65];
    TokenType token;
    int linha;
    int coluna;
} Token;

Token next_token(Buffer *buffer, FILE *arquivo);
void tratamento_de_erro(Token *token, Buffer *buffer);
TokenType buscar_token(const char *str);

#endif
