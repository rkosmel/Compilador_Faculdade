# Meu Compilador

Este projeto é um compilador simples escrito em C que implementa três etapas principais do processo de compilação:

- **Análise Léxica:**  
  Lê o arquivo de entrada e divide-o em tokens.

- **Análise Sintática:**  
  Constrói a Árvore Sintática Abstrata (AST) a partir dos tokens produzidos pelo lexer. Cada nó da AST armazena o tipo do nó, o lexema e a linha em que o token foi reconhecido.

- **Análise Semântica:**  
  Percorre a AST, gera uma tabela de símbolos e realiza verificações semânticas básicas. A tabela de símbolos é impressa no formato:
  **Nome_ID:** Nome do identificador.
  **Escopo:** Escopo em que o símbolo foi declarado (ex.: "global" ou o nome da função).
  **Tipo_ID:** "FUNCAO" se o símbolo representar uma função ou "VARIAVEL" caso contrário.
  **Tipo_dado:** Tipo do símbolo (ex.: "int" ou "void").
  **Linha:** Linha do arquivo onde o símbolo foi declarado.

> **Observação:**  
> Este compilador foi desenvolvido para fins educacionais e possui uma implementação simplificada, servindo como base para futuras extensões (por exemplo, verificação de tipos mais robusta ou geração de código).

---

## Estrutura do Projeto

- **main.c:**  
Contém a função `main` que interpreta as flags de linha de comando e invoca a etapa desejada (análise léxica, sintática ou semântica).

- **lexer.h / lexer.c:**  
Módulo responsável pela análise léxica. Contém funções para:
- Alocar e gerenciar o buffer de leitura.
- Ler caracteres do arquivo e atualizar o número da linha.
- Identificar tokens a partir do fluxo de caracteres.

- **parser.h / parser.c:**  
Módulo responsável pela análise sintática. Constrói a AST a partir dos tokens.  
As funções terminais foram modificadas para capturar o número da linha do token no momento da criação dos nós, usando a função auxiliar `criar_no_line()`.

- **semantico.h / semantico.c:**  
Módulo responsável pela análise semântica. Percorre a AST, insere os símbolos em uma tabela de símbolos (mantida também em uma lista global para posterior impressão) e exibe a tabela de símbolos no formato especificado.

---

## Compilação

Para compilar o projeto, certifique-se de ter o GCC (GNU Compiler Collection) instalado. Após executar o comando na raiz do projeto, o executável "compilador" aceita uma flag que determina qual etapa do compilador será executada, seguida do nome do arquivo fonte a ser compilado. A sintaxe de uso é:
./compilador <flag> <arquivo>

- Flags Disponíveis

Análise Léxica:
-1 ou -L

Análise Sintática:
-p ou -P

Análise Semântica:
-s ou -S

```bash
gcc main.c lexer.c parser.c semantico.c -o compilador