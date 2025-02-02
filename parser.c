#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"

/* Variáveis globais do parser */
Token token;
Buffer *buffer;
FILE *arquivo;

/* Função auxiliar para criar um nó da AST com a linha explicitada */
NoArvore* criar_no_line(const char *nome, const char *lexema, int linha) {
    NoArvore* no = (NoArvore*) malloc(sizeof(NoArvore));
    if (!no) {
        perror("Erro ao alocar nó da AST");
        exit(EXIT_FAILURE);
    }
    strcpy(no->nome, nome);
    if (lexema != NULL)
        strcpy(no->lexema, lexema);
    else
        no->lexema[0] = '\0';
    no->linha = linha;
    no->filhos = NULL;
    no->num_filhos = 0;
    return no;
}

/* Adiciona um filho ao nó pai */
void adicionar_filho(NoArvore *pai, NoArvore *filho) {
    if (pai == NULL || filho == NULL)
        return;
    pai->num_filhos++;
    pai->filhos = (NoArvore**) realloc(pai->filhos, pai->num_filhos * sizeof(NoArvore*));
    pai->filhos[pai->num_filhos - 1] = filho;
}

/* Imprime a AST (usada para debug ou visualização da análise sintática) */
void imprimir_arvore(NoArvore *raiz, int nivel) {
    if (raiz == NULL)
        return;
    for (int i = 0; i < nivel; i++)
        printf("    ");
    printf("%s", raiz->nome);
    if (strlen(raiz->lexema) > 0)
        printf("(%s)", raiz->lexema);
    printf(" [Linha: %d]\n", raiz->linha);
    for (int i = 0; i < raiz->num_filhos; i++)
        imprimir_arvore(raiz->filhos[i], nivel + 1);
}

/* Avança para o próximo token */
void avance() {
    token = next_token(buffer, arquivo);
}

/* Verifica se o token atual é o esperado; se sim, avança; se não, imprime erro e encerra */
void casa(TokenType esperado) {
    if (token.token == esperado) {
        avance();
    } else {
        printf("Erro de sintaxe: token esperado %s, token encontrado %s. LINHA: %d\n",
               token_names[esperado], token_names[token.token], token.linha);
        exit(EXIT_FAILURE);
    }
}

/* --- Funções da Gramática --- */

/* programa -> declaracao_lista FIM_DE_ARQUIVO */
NoArvore* programa() {
    int linhaProg = token.linha;
    NoArvore* no = criar_no_line("programa", "", linhaProg);
    adicionar_filho(no, declaracao_lista());
    casa(FIM_DE_ARQUIVO);
    return no;
}

/* declaracao_lista -> { declaracao } */
NoArvore* declaracao_lista() {
    int linhaDeclList = token.linha;
    NoArvore* no = criar_no_line("declaracao_lista", "", linhaDeclList);
    while (token.token == INT || token.token == VOID) {
        adicionar_filho(no, declaracao());
    }
    return no;
}

/* declaracao -> (INT | VOID) ID { ('[' expressao ']' | '(' funcao_parametros ')') | ';' } */
NoArvore* declaracao() {
    int linhaDecl = token.linha;
    NoArvore* no = criar_no_line("declaracao", "", linhaDecl);
    
    if (token.token == INT) {
        int linhaTipo = token.linha;
        adicionar_filho(no, criar_no_line("tipo", "int", linhaTipo));
        casa(INT);
    } else if (token.token == VOID) {
        int linhaTipo = token.linha;
        adicionar_filho(no, criar_no_line("tipo", "void", linhaTipo));
        casa(VOID);
    } else {
        printf("Erro de sintaxe: token esperado INT ou VOID, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    
    if (token.token == ID) {
        int linhaID = token.linha;
        NoArvore* id_no = criar_no_line("id", token.lexema, linhaID);
        adicionar_filho(no, id_no);
        casa(ID);
    } else {
        printf("Erro de sintaxe: token esperado ID, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    
    if (token.token == PONTO_VIRGULA) {
        int linhaPV = token.linha;
        adicionar_filho(no, criar_no_line("ponto_virgula", ";", linhaPV));
        casa(PONTO_VIRGULA);
    } else if (token.token == ABRE_COLCHETES) {
        int linhaAB = token.linha;
        adicionar_filho(no, criar_no_line("abre_colchetes", "[", linhaAB));
        casa(ABRE_COLCHETES);
        adicionar_filho(no, expressao());
        int linhaFC = token.linha;
        adicionar_filho(no, criar_no_line("fecha_colchetes", "]", linhaFC));
        casa(FECHA_COLCHETES);
        int linhaPV = token.linha;
        adicionar_filho(no, criar_no_line("ponto_virgula", ";", linhaPV));
        casa(PONTO_VIRGULA);
    } else if (token.token == ABRE_PARENTESES) {
        adicionar_filho(no, funcao_parametros());
    } else {
        printf("Erro de sintaxe: token esperado ;, [ ou (, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

/* funcao_parametros -> '(' [ lista_parametros ] ')' composto_decl */
NoArvore* funcao_parametros() {
    int linhaFP = token.linha;
    NoArvore* no = criar_no_line("funcao_parametros", "", linhaFP);
    casa(ABRE_PARENTESES);
    if (token.token != FECHA_PARENTESES) {
        adicionar_filho(no, lista_parametros());
    }
    casa(FECHA_PARENTESES);
    adicionar_filho(no, composto_decl());
    return no;
}

/* lista_parametros -> parametro { ',' parametro } */
NoArvore* lista_parametros() {
    int linhaLP = token.linha;
    NoArvore* no = criar_no_line("lista_parametros", "", linhaLP);
    adicionar_filho(no, parametro());
    while (token.token == VIRGULA) {
        int linhaVirg = token.linha;
        adicionar_filho(no, criar_no_line("virgula", ",", linhaVirg));
        casa(VIRGULA);
        adicionar_filho(no, parametro());
    }
    return no;
}

/* parametro -> (INT var) | var | NUM */
/* Nota: A análise semântica tratará o nó "parametro" de forma especial para não processar seus filhos novamente */
NoArvore* parametro() {
    int linhaParam = token.linha;
    NoArvore* no = criar_no_line("parametro", "", linhaParam);
    if (token.token == INT) {
        int linhaTipo = token.linha;
        adicionar_filho(no, criar_no_line("tipo", "int", linhaTipo));
        casa(INT);
        adicionar_filho(no, var());
    } else if (token.token == ID) {
        adicionar_filho(no, var());
    } else if (token.token == NUM) {
        int linhaNum = token.linha;
        adicionar_filho(no, criar_no_line("num", token.lexema, linhaNum));
        casa(NUM);
    } else {
        printf("Erro de sintaxe: token esperado INT, ID ou NUM, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

/* composto_decl -> '{' local_declaracoes statement_lista '}' */
NoArvore* composto_decl() {
    int linhaComp = token.linha;
    NoArvore* no = criar_no_line("composto_decl", "", linhaComp);
    casa(ABRE_CHAVES);
    adicionar_filho(no, local_declaracoes());
    adicionar_filho(no, statement_lista());
    casa(FECHA_CHAVES);
    return no;
}

/* local_declaracoes -> { declaracao } */
NoArvore* local_declaracoes() {
    int linhaLocal = token.linha;
    NoArvore* no = criar_no_line("local_declaracoes", "", linhaLocal);
    while (token.token == INT || token.token == VOID) {
        adicionar_filho(no, declaracao());
    }
    return no;
}

/* statement_lista -> { statement } */
NoArvore* statement_lista() {
    int linhaStmtList = token.linha;
    NoArvore* no = criar_no_line("statement_lista", "", linhaStmtList);
    while (token.token == IF || token.token == WHILE || token.token == RETURN ||
           token.token == ABRE_CHAVES || token.token == ID || token.token == NUM ||
           token.token == PONTO_VIRGULA) {
        adicionar_filho(no, statement());
    }
    return no;
}

/* statement -> selecao_decl | iteracao_decl | retorno_decl | composto_decl | expressao | ';' */
NoArvore* statement() {
    int linhaStmt = token.linha;
    NoArvore* no = criar_no_line("statement", "", linhaStmt);
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
        int linhaPV = token.linha;
        adicionar_filho(no, criar_no_line("ponto_virgula", ";", linhaPV));
        casa(PONTO_VIRGULA);
    } else {
        printf("Erro de sintaxe: token esperado IF, WHILE, RETURN, {, ID, NUM ou ;, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

/* selecao_decl -> IF '(' expressao ')' statement [ ELSE statement ] */
NoArvore* selecao_decl() {
    int linhaSelecao = token.linha;
    NoArvore* no = criar_no_line("selecao_decl", "", linhaSelecao);
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

/* iteracao_decl -> WHILE '(' expressao ')' statement */
NoArvore* iteracao_decl() {
    int linhaIteracao = token.linha;
    NoArvore* no = criar_no_line("iteracao_decl", "", linhaIteracao);
    casa(WHILE);
    casa(ABRE_PARENTESES);
    adicionar_filho(no, expressao());
    casa(FECHA_PARENTESES);
    adicionar_filho(no, statement());
    return no;
}

/* retorno_decl -> RETURN [ expressao ] ';' */
NoArvore* retorno_decl() {
    int linhaRetorno = token.linha;
    NoArvore* no = criar_no_line("retorno_decl", "", linhaRetorno);
    casa(RETURN);
    if (token.token == PONTO_VIRGULA) {
        int linhaPV = token.linha;
        adicionar_filho(no, criar_no_line("ponto_virgula", ";", linhaPV));
        casa(PONTO_VIRGULA);
    } else {
        adicionar_filho(no, var());
        int linhaPV = token.linha;
        adicionar_filho(no, criar_no_line("ponto_virgula", ";", linhaPV));
        casa(PONTO_VIRGULA);
    }
    return no;
}

/* expressao -> [ var ATRIBUICAO expressao_simples ] | expressao_simples */
NoArvore* expressao() {
    int linhaExpr = token.linha;
    NoArvore* no = criar_no_line("expressao", "", linhaExpr);
    if (token.token == ID) {
        NoArvore* varNode = var();
        adicionar_filho(no, varNode);
        if (token.token == ATRIBUICAO) {
            int linhaAtrib = token.linha;
            adicionar_filho(no, criar_no_line("atribuicao", "=", linhaAtrib));
            casa(ATRIBUICAO);
            adicionar_filho(no, expressao_simples());
        } else if (token.token == ABRE_PARENTESES) {
            adicionar_filho(no, args());
        } else if (token.token == MENOR || token.token == MENOR_IGUAL ||
                   token.token == MAIOR || token.token == MAIOR_IGUAL ||
                   token.token == IGUAL || token.token == DIFERENTE) {
            adicionar_filho(no, relacional());
            adicionar_filho(no, expressao_simples());
        }
    } else {
        adicionar_filho(no, expressao_simples());
    }
    return no;
}

/* expressao_simples -> soma [ operador soma ] */
NoArvore* expressao_simples() {
    int linhaExprSimples = token.linha;
    NoArvore* no = criar_no_line("expressao_simples", "", linhaExprSimples);
    adicionar_filho(no, soma());
    if (token.token == MENOR || token.token == MENOR_IGUAL ||
        token.token == MAIOR || token.token == MAIOR_IGUAL ||
        token.token == IGUAL || token.token == DIFERENTE ||
        token.token == MENOS || token.token == MAIS) {
        char op[3] = "";
        if (token.token == MENOS)
            strcpy(op, "-");
        else if (token.token == MAIS)
            strcpy(op, "+");
        else if (token.token == MENOR)
            strcpy(op, "<");
        else if (token.token == MENOR_IGUAL)
            strcpy(op, "<=");
        else if (token.token == MAIOR)
            strcpy(op, ">");
        else if (token.token == MAIOR_IGUAL)
            strcpy(op, ">=");
        else if (token.token == IGUAL)
            strcpy(op, "==");
        else if (token.token == DIFERENTE)
            strcpy(op, "!=");
        int linhaOp = token.linha;
        adicionar_filho(no, criar_no_line("operador", op, linhaOp));
        avance();
        adicionar_filho(no, soma());
    }
    return no;
}

/* soma -> termo { ('+' | '-') termo } */
NoArvore* soma() {
    int linhaSoma = token.linha;
    NoArvore* no = criar_no_line("soma", "", linhaSoma);
    adicionar_filho(no, termo());
    while (token.token == MAIS || token.token == MENOS) {
        char op[2] = "";
        if (token.token == MAIS)
            strcpy(op, "+");
        else if (token.token == MENOS)
            strcpy(op, "-");
        int linhaOp = token.linha;
        adicionar_filho(no, criar_no_line("operador", op, linhaOp));
        avance();
        adicionar_filho(no, termo());
    }
    return no;
}

/* termo -> fator { ('*' | '/') fator } */
NoArvore* termo() {
    int linhaTermo = token.linha;
    NoArvore* no = criar_no_line("termo", "", linhaTermo);
    adicionar_filho(no, fator());
    while (token.token == VEZES || token.token == DIVISAO) {
        char op[2] = "";
        if (token.token == VEZES)
            strcpy(op, "*");
        else if (token.token == DIVISAO)
            strcpy(op, "/");
        int linhaOp = token.linha;
        adicionar_filho(no, criar_no_line("operador", op, linhaOp));
        avance();
        adicionar_filho(no, fator());
    }
    return no;
}

/* fator -> '(' expressao ')' | ID [ ('[' expressao ']') | ('(' args ')') ] | NUM */
NoArvore* fator() {
    int linhaFator = token.linha;
    NoArvore* no = criar_no_line("fator", "", linhaFator);
    if (token.token == ABRE_PARENTESES) {
        int linhaAB = token.linha;
        adicionar_filho(no, criar_no_line("abre_paren", "(", linhaAB));
        casa(ABRE_PARENTESES);
        adicionar_filho(no, expressao());
        int linhaFC = token.linha;
        adicionar_filho(no, criar_no_line("fecha_paren", ")", linhaFC));
        casa(FECHA_PARENTESES);
    } else if (token.token == ID) {
        int linhaID = token.linha;
        NoArvore* id_node = criar_no_line("id", token.lexema, linhaID);
        adicionar_filho(no, id_node);
        casa(ID);
        if (token.token == ABRE_COLCHETES) {
            int linhaAB = token.linha;
            adicionar_filho(id_node, criar_no_line("abre_colchetes", "[", linhaAB));
            casa(ABRE_COLCHETES);
            if (token.token != FECHA_COLCHETES) {
                adicionar_filho(id_node, expressao());
            }
            int linhaFC = token.linha;
            adicionar_filho(id_node, criar_no_line("fecha_colchetes", "]", linhaFC));
            casa(FECHA_COLCHETES);
        } else if (token.token == ABRE_PARENTESES) {
            adicionar_filho(id_node, args());
        }
    } else if (token.token == NUM) {
        int linhaNum = token.linha;
        adicionar_filho(no, criar_no_line("num", token.lexema, linhaNum));
        casa(NUM);
    } else {
        printf("Erro de sintaxe: token esperado (, ID ou NUM, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

/* relacional -> ('<' | '<=' | '>' | '>=' | '==' | '!=') */
NoArvore* relacional() {
    int linhaRel = token.linha;
    NoArvore* no = criar_no_line("relacional", "", linhaRel);
    if (token.token == MENOR) {
        adicionar_filho(no, criar_no_line("operador", "<", token.linha));
        avance();
    } else if (token.token == MENOR_IGUAL) {
        adicionar_filho(no, criar_no_line("operador", "<=", token.linha));
        avance();
    } else if (token.token == MAIOR) {
        adicionar_filho(no, criar_no_line("operador", ">", token.linha));
        avance();
    } else if (token.token == MAIOR_IGUAL) {
        adicionar_filho(no, criar_no_line("operador", ">=", token.linha));
        avance();
    } else if (token.token == IGUAL) {
        adicionar_filho(no, criar_no_line("operador", "==", token.linha));
        avance();
    } else if (token.token == DIFERENTE) {
        adicionar_filho(no, criar_no_line("operador", "!=", token.linha));
        avance();
    } else {
        printf("Erro de sintaxe: token esperado <, <=, >, >=, == ou !=, token encontrado %s.\n", token_names[token.token]);
        exit(EXIT_FAILURE);
    }
    return no;
}

/* args -> '(' [ lista_args ] ')' */
NoArvore* args() {
    int linhaArgs = token.linha;
    NoArvore* no = criar_no_line("args", "", linhaArgs);
    casa(ABRE_PARENTESES);
    if (token.token != FECHA_PARENTESES) {
        adicionar_filho(no, lista_args());
    }
    casa(FECHA_PARENTESES);
    return no;
}

/* lista_args -> expressao { ',' expressao } */
NoArvore* lista_args() {
    int linhaLA = token.linha;
    NoArvore* no = criar_no_line("lista_args", "", linhaLA);
    adicionar_filho(no, expressao());
    while (token.token == VIRGULA) {
        int linhaVirg = token.linha;
        adicionar_filho(no, criar_no_line("virgula", ",", linhaVirg));
        casa(VIRGULA);
        adicionar_filho(no, expressao());
    }
    return no;
}

/* var -> ID [ '[' [ expressao ] ']' ] */
NoArvore* var() {
    int linhaVar = token.linha;
    NoArvore* no = criar_no_line("var", token.lexema, linhaVar);
    casa(ID);
    if (token.token == ABRE_COLCHETES) {
        int linhaAB = token.linha;
        adicionar_filho(no, criar_no_line("abre_colchetes", "[", linhaAB));
        casa(ABRE_COLCHETES);
        if (token.token != FECHA_COLCHETES) {
            adicionar_filho(no, expressao());
        }
        int linhaFC = token.linha;
        adicionar_filho(no, criar_no_line("fecha_colchetes", "]", linhaFC));
        casa(FECHA_COLCHETES);
    }
    return no;
}

/* parse -> inicia a análise sintática e retorna a AST */
NoArvore* parse(FILE *arquivo_passado) {
    arquivo = arquivo_passado;
    buffer = allocate_buffer();
    avance();  // Lê o primeiro token
    NoArvore* raiz = programa();
    printf("Análise sintática concluída com sucesso!\n");
    return raiz;
}
