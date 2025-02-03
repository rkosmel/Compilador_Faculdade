#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

/* Usamos uma variável estática para acompanhar a linha corrente */
static int currentLine = 1;

Buffer *allocate_buffer() {
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    if (!buffer) {
        perror("Erro ao alocar buffer");
        exit(EXIT_FAILURE);
    }
    memset(buffer->buffer, 0, BUFFER_SIZE);
    buffer->position = 0;
    currentLine = 1;
    buffer->line_number = currentLine;
    buffer->coluna = 0;
    buffer->line_advanced = 0;
    return buffer;
}

int is_symbol(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '<' || c == '>' || c == '=' || c == ';' ||
           c == ',' || c == '(' || c == ')' || c == '[' ||
           c == ']' || c == '{' || c == '}' || c == '!';
}

const char *token_names[] = {
    "ELSE", "IF", "INT", "RETURN", "VOID", "WHILE",
    "MAIS", "MENOS", "VEZES", "DIVISAO", "MENOR", "MENOR_IGUAL",
    "MAIOR", "MAIOR_IGUAL", "IGUAL", "DIFERENTE", "ATRIBUICAO",
    "PONTO_VIRGULA", "VIRGULA", "ABRE_PARENTESES", "FECHA_PARENTESES",
    "ABRE_COLCHETES", "FECHA_COLCHETES", "ABRE_CHAVES", "FECHA_CHAVES",
    "ID", "NUM", "FIM_DE_ARQUIVO", "ERRO"
};

TokenType buscar_token(const char *str) {
    int n = sizeof(token_names) / sizeof(token_names[0]);
    for (int i = 0; i < n; i++) {
        if (strcmp(str, token_names[i]) == 0)
            return (TokenType)i;
    }
    return ERRO;
}

void tratamento_de_erro(Token *token, Buffer *buffer) {
    printf("ERRO LÉXICO: \"%s\" INVALIDO [linha: %d], COLUNA %d.\n",
           token->lexema, token->linha, token->coluna);
    exit(EXIT_FAILURE);
}

char get_next_char(Buffer *buffer, FILE *arquivo) {
    if (arquivo == NULL) {
        printf("Erro: Tentativa de leitura com arquivo NULL.\n");
        exit(1);
    }
    if (buffer->position >= BUFFER_SIZE || buffer->buffer[buffer->position] == '\0') {
        int i = 0;
        int c;
        while (i < BUFFER_SIZE - 1 && (c = fgetc(arquivo)) != EOF) {
            buffer->buffer[i++] = (char)c;
        }
        buffer->buffer[i] = '\0';
        buffer->position = 0;
    }
    if (buffer->position >= BUFFER_SIZE)
        return EOF;
    char current_char = buffer->buffer[buffer->position++];
    if (current_char == '\n') {
        currentLine++;
        buffer->line_number = currentLine;
        buffer->coluna = 0;
        buffer->line_advanced = 1;
    } else {
        buffer->coluna++;
        buffer->line_advanced = 0;
    }
    return current_char;
}

Token next_token(Buffer *buffer, FILE *arquivo) {
    if (arquivo == NULL) {
        printf("Erro: Tentativa de chamar next_token() com arquivo NULL.\n");
        exit(1);
    }
    Token token;
    int linhaToken = currentLine;  // Captura a linha corrente
    char lexema[65] = "";
    int i = 0;
    int c = get_next_char(buffer, arquivo);
    if (c == EOF) {
        token.token = FIM_DE_ARQUIVO;
        token.linha = linhaToken;
        token.coluna = buffer->coluna; // ou 0, se preferir
        return token;
    }
    // Pula espaços em branco, tabulações e quebras de linha
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = get_next_char(buffer, arquivo);
        linhaToken = currentLine;
    }
    // Registra a coluna onde o token inicia
    int token_coluna = buffer->coluna;  
    int estado = 0;
    while ((c != ' ' && c != '\t' && c != '\n' && c != '\r') || estado == 8 || estado == 9) {
        switch (estado) {
            case 0:
                if (isalpha(c))
                    estado = 1;
                else if (isdigit(c))
                    estado = 2;
                else if (c == '+' || c == '-' || c == ';' || c == ',' ||
                         c == '(' || c == ')' || c == '[' || c == ']' ||
                         c == '{' || c == '}' || c == '*')
                    estado = 3;
                else if (c == '<' || c == '>' || c == '=')
                    estado = 4;
                else if (c == '/')
                    estado = 7;
                else if (c == '!')
                    estado = 6;
                else
                    estado = 10;
                break;
            case 1:
                if (isalpha(c))
                    estado = 1;
                else
                    estado = 10;
                break;
            case 2:
                if (isdigit(c))
                    estado = 2;
                else
                    estado = 10;
                break;
            case 3:
                break;
            case 4:
                if (c == '=')
                    estado = 5;
                else
                    estado = 10;
                break;
            case 5:
                break;
            case 6:
                if (c == '=')
                    estado = 5;
                else
                    estado = 10;
                break;
            case 7:
                if (c == '*')
                    estado = 8;
                break;
            case 8:
                if (c == '*')
                    estado = 9;
                break;
            case 9:
                if (c == '/')
                    return next_token(buffer, arquivo);
                else
                    estado = 8;
                break;
            case 10:
                estado = 10;
                break;
        }
        if (estado != 8)
            lexema[i++] = c;
        c = get_next_char(buffer, arquivo);
        if (c == EOF) {
            token.token = FIM_DE_ARQUIVO;
            token.linha = linhaToken;
            token.coluna = token_coluna;
            return token;
        }
        if (is_symbol(c) && (estado == 1 || estado == 2 || estado == 10)) {
            if (c != '\n')
                buffer->position--;
            break;
        }
        if ((estado == 3 || estado == 5) || (estado == 7 && c != '*')) {
            if (c != '\n')
                buffer->position--;
            break;
        }
    }
    // Preenche os campos do token
    switch (estado) {
        case 1:
            strcpy(token.lexema, lexema);
            if (strcmp(lexema, "else") == 0)
                token.token = ELSE;
            else if (strcmp(lexema, "if") == 0)
                token.token = IF;
            else if (strcmp(lexema, "int") == 0)
                token.token = INT;
            else if (strcmp(lexema, "return") == 0)
                token.token = RETURN;
            else if (strcmp(lexema, "void") == 0)
                token.token = VOID;
            else if (strcmp(lexema, "while") == 0)
                token.token = WHILE;
            else
                token.token = ID;
            break;
        case 2:
            strcpy(token.lexema, lexema);
            token.token = NUM;
            break;
        case 3:
            strcpy(token.lexema, lexema);
            switch (lexema[0]) {
                case '+': token.token = MAIS; break;
                case '-': token.token = MENOS; break;
                case '*': token.token = VEZES; break;
                case '/': token.token = DIVISAO; break;
                case '<': token.token = MENOR; break;
                case '>': token.token = MAIOR; break;
                case '=': token.token = IGUAL; break;
                case ';': token.token = PONTO_VIRGULA; break;
                case ',': token.token = VIRGULA; break;
                case '(': token.token = ABRE_PARENTESES; break;
                case ')': token.token = FECHA_PARENTESES; break;
                case '[': token.token = ABRE_COLCHETES; break;
                case ']': token.token = FECHA_COLCHETES; break;
                case '{': token.token = ABRE_CHAVES; break;
                case '}': token.token = FECHA_CHAVES; break;
            }
            break;
        case 4:
            strcpy(token.lexema, lexema);
            if (lexema[0] == '=')
                token.token = ATRIBUICAO;
            else if (lexema[0] == '>')
                token.token = MAIOR;
            else if (lexema[0] == '<')
                token.token = MENOR;
            break;
        case 5:
            strcpy(token.lexema, lexema);
            if (strcmp(lexema, "==") == 0)
                token.token = IGUAL;
            else if (strcmp(lexema, ">=") == 0)
                token.token = MAIOR_IGUAL;
            else if (strcmp(lexema, "<=") == 0)
                token.token = MENOR_IGUAL;
            else if (strcmp(lexema, "!=") == 0)
                token.token = DIFERENTE;
            break;
        case 6:
            strcpy(token.lexema, lexema);
            token.token = ERRO;
            break;
        case 7:
            strcpy(token.lexema, lexema);
            token.token = DIVISAO;
            break;
        case 10:
            strcpy(token.lexema, lexema);
            token.token = ERRO;
            break;
        default:
            break;
    }
    token.linha = linhaToken;
    token.coluna = token_coluna;  // Armazena a coluna de início do token
    int len = strlen(buffer->buffer);
    if (token.token == ERRO && buffer->position >= len) {
        token.token = FIM_DE_ARQUIVO;
    }
    return token;
}
