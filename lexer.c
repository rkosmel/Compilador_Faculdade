#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

// Variável estática para armazenar a linha atual (inicializada em 1)
static int currentLine = 1;

Buffer *allocate_buffer() {
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    if (!buffer) {
        perror("Erro ao alocar buffer");
        exit(EXIT_FAILURE);
    }
    memset(buffer->buffer, 0, BUFFER_SIZE);
    buffer->position = 0;
    currentLine = 1;               // Reinicia a linha
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

/* Definindo o array de nomes dos tokens com o mesmo nome declarado em lexer.h */
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
        if (strcmp(str, token_names[i]) == 0) {
            return (TokenType)i;
        }
    }
    return ERRO;
}

void tratamento_de_erro(Token *token, Buffer *buffer) {
    printf("ERRO LÉXICO: \"%s\" INVÁLIDO [linha: %d], COLUNA %zu.\n",
           token->lexema, token->linha, (size_t)(buffer->coluna - strlen(token->lexema) - 1));

    char input[50];
    printf("Deseja encerrar a compilação (F) ou ver mais informações (+)? ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "F") == 0 || strcmp(input, "f") == 0) {
        printf("Encerrando compilação.\n");
        exit(EXIT_FAILURE);
    } else if (strcmp(input, "+") == 0) {
        printf("Opções disponíveis:\n");
        printf("1. Ignorar o lexema com erro.\n");
        printf("2. Definir manualmente seu token.\n");
        printf("Escolha uma opção (1 ou 2): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "1") == 0) {
            printf("Ignorar o lexema pode levar a futuros erros sintáticos e/ou semânticos.\n");
            printf("Tem certeza que deseja ignorar o lexema com erro? (S/N): ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';
            if (strcmp(input, "S") == 0 || strcmp(input, "s") == 0) {
                printf("Lexema ignorado.\n");
            } else {
                printf("Operação cancelada. Encerrando compilação.\n");
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(input, "2") == 0) {
            printf("Opções disponíveis:\n");
            int n = sizeof(token_names) / sizeof(token_names[0]);
            for (int i = 0; i < n; i++) {
                printf("%s%s", token_names[i],
                       (i % 6 == 5 || i == n - 1) ? "\n" : ", ");
            }
            printf("Digite exatamente como nas opções disponíveis o token correto para o lexema \"%s\": ", token->lexema);
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';

            TokenType novoToken = buscar_token(input);
            if (novoToken != ERRO) {
                token->token = novoToken;
                printf("Token \"%s\" atribuído ao lexema \"%s\".\n", token_names[novoToken], token->lexema);
            } else {
                printf("Token inválido. Encerrando compilação.\n");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Opção inválida. Encerrando compilação.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("Opção inválida. Encerrando compilação.\n");
        exit(EXIT_FAILURE);
    }
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

    if (buffer->position >= BUFFER_SIZE) {
        return EOF;
    }

    char current_char = buffer->buffer[buffer->position++];
    if (current_char == '\n') {
        currentLine++;  // Incrementa a linha global
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
    // Use a variável estática currentLine para capturar a linha do token
    int linhaToken = currentLine;
    char lexema[65] = "";
    int i = 0;
    int c = get_next_char(buffer, arquivo);

    if (c == EOF) {
        token.token = FIM_DE_ARQUIVO;
        token.linha = linhaToken;
        return token;
    }

    // Pula espaços, tabulações e quebras de linha e atualiza a linha conforme necessário
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = get_next_char(buffer, arquivo);
        linhaToken = currentLine;
    }

    int estado = 0;
    while ((c != ' ' && c != '\t' && c != '\n' && c != '\r') || estado == 8 || estado == 9) {
        switch (estado) {
            case 0:
                if (isalpha(c))
                    estado = 1;
                else if (isdigit(c))
                    estado = 2;
                else if (c == '+' || c == '-' || c == ';' || c == ',' || c == '(' ||
                         c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '*')
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

        if (estado != 8) {
            lexema[i++] = c;
        }

        c = get_next_char(buffer, arquivo);
        if (c == EOF) {
            token.token = FIM_DE_ARQUIVO;
            token.linha = linhaToken;
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

    int len = strlen(buffer->buffer);
    if (token.token == ERRO && buffer->position >= len) {
        token.token = FIM_DE_ARQUIVO;
    }
    return token;
}
