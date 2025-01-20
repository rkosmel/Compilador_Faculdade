#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Buffer allocate_buffer() {
    Buffer buffer;
    memset(buffer.buffer, 0, BUFFER_SIZE);
    buffer.position = 0;
    buffer.line_number = 1;
    return buffer;
}

// variável para verificar se é algum símbolo
int is_symbol(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '<' || c == '>' || c == '=' || c == ';' || c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '!';
}

// Função para imprimir mensagem de erro léxico e tratar interação com o usuário
void tratamento_de_erro(Token token) {
    printf("ERRO LEXICO: \"%s\" INVALIDO [linha: %d], COLUNA %d.\n", token.lexema, token.linha, (int)strlen(token.lexema) + 1);

    // char input[10];
    // printf("Deseja encerrar a compilação (F) ou ver mais informações (+)? ");
    // fgets(input, sizeof(input), stdin);
    // input[strcspn(input, "\n")] = '\0'; // Remove o newline do final da entrada

    // if (strcmp(input, "F") == 0 || strcmp(input, "f") == 0) {
    //     printf("Encerrando compilação.\n");
    //     exit(EXIT_FAILURE);
    // } else if (strcmp(input, "+") == 0) {
    //     printf("Opções disponíveis:\n");
    //     printf("1. Ignorar o lexema com erro.\n");
    //     printf("2. Substituir o lexema.\n");

    //     printf("Escolha uma opção (1 ou 2): ");
    //     fgets(input, sizeof(input), stdin);
    //     input[strcspn(input, "\n")] = '\0'; // Remove o newline do final da entrada

    //     if (strcmp(input, "1") == 0) {
    //         printf("Tem certeza que deseja ignorar o lexema com erro? (S/N): ");
    //         fgets(input, sizeof(input), stdin);
    //         input[strcspn(input, "\n")] = '\0'; // Remove o newline do final da entrada

    //         if (strcmp(input, "S") == 0 || strcmp(input, "s") == 0) {
    //             printf("Lexema ignorado.\n");
    //             // Continua a execução
    //         } else {
    //             printf("Operação cancelada.\n");
    //             // Pode adicionar tratamento adicional aqui se necessário
    //         }
    //     } else if (strcmp(input, "2") == 0) {
    //         // Aqui você implementaria a lógica para substituir o lexema com erro
    //         // Exemplo: Solicitar ao usuário que insira o token correto para o lexema
    //         printf("Substituir lexema \"%s\" por: ", token.lexema);
    //         // Implementar lógica para escolha do token correto e substituição
    //         // Pode usar um loop para mostrar opções de tokens disponíveis e aguardar a entrada do usuário
    //         printf("Token substituído com sucesso.\n");
    //         // Continua a execução com o novo token atribuído ao lexema
    //     } else {
    //         printf("Opção inválida. Encerrando compilação.\n");
    //         exit(EXIT_FAILURE);
    //     }
    // } else {
    //     printf("Opção inválida. Encerrando compilação.\n");
    //     exit(EXIT_FAILURE);
    // }
}

char get_next_char(Buffer *buffer, FILE *arquivo) {
    if (buffer->position >= BUFFER_SIZE || buffer->buffer[buffer->position] == '\0') {
        int i = 0;
        char c;
        while (i < BUFFER_SIZE - 1) {
            c = fgetc(arquivo);
            if (c == EOF) {
                if (i == 0) {
                    buffer->buffer[i] = '\0';
                    buffer->position = 0;
                    return EOF;
                }
                break;
            }
            buffer->buffer[i++] = c;
        }
        buffer->buffer[i] = '\0';
        buffer->position = 0;
    }

    char current_char = buffer->buffer[buffer->position];

    if (current_char == '\n') {
        buffer->line_number++;
    }

    buffer->position++;

    return current_char;
}

// para converter o token para string
/*
typedef enum {
    ELSE, IF, INT, RETURN, VOID, WHILE,
    MAIS, MENOS, VEZES, DIVISAO, MENOR, MENOR_IGUAL, MAIOR, MAIOR_IGUAL, IGUAL, DIFERENTE, ATRIBUICAO, PONTO_VIRGULA, VIRGULA, ABRE_PARENTESES, FECHA_PARENTESES, ABRE_COLCHETES, FECHA_COLCHETES, ABRE_CHAVES, FECHA_CHAVES,
    ID, NUM, FIM_DE_ARQUIVO, ERRO
} TokenType;
*/
const char *token_names[] = {
    "ELSE", "IF", "INT", "RETURN", "VOID", "WHILE",
    "MAIS", "MENOS", "VEZES", "DIVISAO", "MENOR", "MENOR_IGUAL", "MAIOR", "MAIOR_IGUAL", "IGUAL", "DIFERENTE", "ATRIBUICAO", "PONTO_VIRGULA", "VIRGULA", "ABRE_PARENTESES", "FECHA_PARENTESES", "ABRE_COLCHETES", "FECHA_COLCHETES", "ABRE_CHAVES", "FECHA_CHAVES",
    "ID", "NUM", "FIM_DE_ARQUIVO", "ERRO"
};

Token next_token(Buffer *buffer, FILE *arquivo) {
    Token token;
    char lexema[65] = "";
    int i = 0;
    int c;
    c = get_next_char(buffer, arquivo);

    // Encerrar a função se o arquivo acabar
    if (c == EOF) {
        token.token = EOF;
        return token;
    }

    // Antes de entrar no automato, precisamos verificar se o caractere é um espaço em branco, tabulação ou quebra de linha
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        c = get_next_char(buffer, arquivo);
    }

    int estado = 0;
    // Nossos delimitadores do lexemas serão espaços em branco, tabulações e quebras de linha (quando não estiverem dentro de um comentário)
    while ((c != ' ' && c != '\t' && c != '\n' && c != '\r') || estado == 8) {

        /* Implementação usando cases aninhados a partir do automato finito desenvolvido
        Teremos 11 estados, sendo q0 o inicial, q10 estado armadilha (erro)
        q1, q2, q3, q4, q5 e q7 estados finais
        */
        switch (estado) {
            case 0: {
                // resetamos as variáveis pois pode ter voltado de um comentário
                i = 0;
                memset(lexema, 0, 65);

                if (isalpha(c)) {
                    estado = 1;
                } else if (isdigit(c)) {
                    estado = 2;
                } else if (c == '+' || c == '-' || c == ';' || c == ','  || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}') {
                    estado = 3;
                } else if (c == '<' || c == '>' || c == '=') {
                    estado = 4;
                } else if (c == '/') {
                    estado = 7;
                } else if (c == '!') {
                    estado = 6;
                } else {
                    estado = 10;
                }
                break;
            }

            case 1: {
                if (isalpha(c))
                    estado = 1;
                else
                    estado = 10;
                break;
            }

            case 2: {
                if (isdigit(c))
                    estado = 2;
                else
                    estado = 10;
                break;
            }

            case 3: {
                // nunca chegará aqui, pois já teremos encerrado o automato
                break;
            }

            case 4: {
                if (c == '=')
                    estado = 5;
                else
                    estado = 10;
                break;
            }

            case 5: {
                estado = 10;
                break;
            }

            case 6: {
                if (c == '=')
                    estado = 5;
                else
                    estado = 10;
                break;
            }

            case 7: {
                if (c == '*')
                    estado = 8;
                else
                    estado = 10;
                break;
            }

            case 8: {
                if (c == '*')
                    estado = 9;
                break;
            }

            case 9: {
                if (c == '/') {
                    estado = 0;
                    // buffer->position++; // para não pegar o / do final do comentário
                    c = get_next_char(buffer, arquivo);
                }
                else
                    estado = 8;
                break;
            }

            case 10: {
                estado = 10;
                break;
            }

        }
        lexema[i++] = c;
        c = get_next_char(buffer, arquivo);
        // verificar se acabou o arquivo
        if (c == EOF) {
            break;
        }
        // se for um símbolo e estiver nos estados 0, 1, 2 ou 3, encerramos o automato
        if (is_symbol(c) && (estado == 0 || estado == 1 || estado == 2 || estado == 3)) {
            // sairemos do automato e retornamos o caractere para o buffer
            buffer->position--;
            break;
        }
    }

    // agora precisamos atribuir o token certo a cada lexema, de acordo com o estado final
    switch (estado) {
        case 1: {
            strcpy(token.lexema, lexema);
            if (strcmp(lexema, "else") == 0) {
                token.token = ELSE;
            } else if (strcmp(lexema, "if") == 0) {
                token.token = IF;
            } else if (strcmp(lexema, "int") == 0) {
                token.token = INT;
            } else if (strcmp(lexema, "return") == 0) {
                token.token = RETURN;
            } else if (strcmp(lexema, "void") == 0) {
                token.token = VOID;
            } else if (strcmp(lexema, "while") == 0) {
                token.token = WHILE;
            } else {
                token.token = ID;
            }
            break;
        }

        case 2: {
            strcpy(token.lexema, lexema);
            token.token = NUM;
            break;
        }

        case 3: {
            strcpy(token.lexema, lexema);
            switch (lexema[0]) {
                case '+':
                    token.token = MAIS;
                    break;
                case '-':
                    token.token = MENOS;
                    break;
                case '*':
                    token.token = VEZES;
                    break;
                case '/':
                    token.token = DIVISAO;
                    break;
                case '<':
                    token.token = MENOR;
                    break;
                case '>':
                    token.token = MAIOR;
                    break;
                case '=':
                    token.token = IGUAL;
                    break;
                case ';':
                    token.token = PONTO_VIRGULA;
                    break;
                case ',':
                    token.token = VIRGULA;
                    break;
                case '(':
                    token.token = ABRE_PARENTESES;
                    break;
                case ')':
                    token.token = FECHA_PARENTESES;
                    break;
                case '[':
                    token.token = ABRE_COLCHETES;
                    break;
                case ']':
                    token.token = FECHA_COLCHETES;
                    break;
                case '{':
                    token.token = ABRE_CHAVES;
                    break;
                case '}':
                    token.token = FECHA_CHAVES;
                    break;
            }
            break;
        }

        case 4: {
            strcpy(token.lexema, lexema);
            if (lexema[0] == '=') {
                token.token = ATRIBUICAO;
            } else if (lexema[0] == '>') {
                token.token = MAIOR;
            } else if (lexema[0] == '<') {
                token.token = MENOR;
            }
            break;
        }

        case 5: {
            strcpy(token.lexema, lexema);
            if (strcmp(lexema, "==") == 0) {
                token.token = IGUAL;
            } else if (strcmp(lexema, ">=") == 0) {
                token.token = MAIOR_IGUAL;
            } else if (strcmp(lexema, "<=") == 0) {
                token.token = MENOR_IGUAL;
            } else if (strcmp(lexema, "!=") == 0) {
                token.token = DIFERENTE;
            }
            break;
        }

        case 6: {
            strcpy(token.lexema, lexema); // copiar o lexema para a estrutura token
            token.token = ERRO;
            break;
        }

        case 7: {
            strcpy(token.lexema, lexema); // copiar o lexema para a estrutura token
            token.token = DIVISAO;
            break;
        }

        case 10: {
            strcpy(token.lexema, lexema); // copiar o lexema para a estrutura token
            token.token = ERRO;
            break;
        }
    }   

    token.linha = buffer->line_number;
    return token;
}
