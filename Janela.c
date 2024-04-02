#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <ncurses.h>

#define SIZE 9
#define VAZIO 0

// Enumeração de erros para serem exibidos na infoJanela
typedef enum {
    NUMERO_INVALIDO,
    CAMINHO_INVALIDO,

} Erros;

typedef struct {
    int sudoku[SIZE][SIZE];
} Sudoku;

/** 
 * Janela de Menu que será exibida ao usuário um Menu e levará as janela de jogoJanela, infoJanela e comandoJanela ( o Jogo)
 * se digitado 1, o jogo será iniciado
 * se digitado 2, o jogo será carregado a a partir de um save
 * se digitado 3, o jogo será encerrado
*/
WINDOW *menuJanela(int altura, int largura, int inicioY, int inicioX) {
    WINDOW *menujanela;
    menujanela = newwin(altura, largura, inicioY, inicioX);
    box(menujanela, 0, 0);
    mvwprintw(menujanela, 1, 1, "1 - Novo Jogo");
    mvwprintw(menujanela, 2, 1, "2 - Carregar Jogo");
    mvwprintw(menujanela, 3, 1, "3 - Sair");
    wrefresh(menujanela);
    return menujanela;
}

// Janela principal do jogo
WINDOW *jogoJanela(int altura, int largura, int inicioY, int inicioX) {
    WINDOW *mainjanela;
    mainjanela = newwin(altura, largura, inicioY, inicioX);
    box(mainjanela, 0, 0);
    wrefresh(mainjanela);
    return mainjanela;
}

// Info de jogadas
WINDOW *infoJanela(int altura, int largura, int inicioY, int inicioX) {
    WINDOW *infojanela;
    infojanela = newwin(altura, largura, inicioY, inicioX);
    box(infojanela, 0, 0);
    wrefresh(infojanela);
    return infojanela;
}

/// @brief 
/// @param altura 
/// @param largura 
/// @param inicioY 
/// @param inicioX 
/// @return 
// Responsável por criar a janela de funções simples onde o usuário pode digitar comandos
WINDOW *comandoJanela(int altura, int largura, int inicioY, int inicioX) {
    WINDOW *codjanela;
    codjanela = newwin(altura, largura, inicioY, inicioX);
    box(codjanela, 0, 0);
    wrefresh(codjanela);
    return codjanela;
}

// Responsável por destruir as janelas codjanela, infojanela e janela ao final do jogo
void destruirJanelas(WINDOW *codjanela, WINDOW *infojanela, WINDOW *janela) {
    delwin(codjanela);
    delwin(infojanela);
    delwin(janela);
}

// Responsável por imprimir o sudoku na mainjanela
/**
 * Imprime o sudoku na tela principal do jogo
*/
void imprimeSudoku(Sudoku *sudoku, WINDOW *mainjanela) {
    for (int i = 0; i < SIZE; i++) {
        if (i % 3 == 0 && i != 0) {
            mvwprintw(mainjanela, i + 1, 0, "------+-------+------");
        }
        for (int j = 0; j < SIZE; j++) {
            if (j % 3 == 0 && j != 0) {
                mvwprintw(mainjanela, i + 1, j * 2, "| ");
            }
            if (sudoku->sudoku[i][j] == VAZIO) {
                mvwprintw(mainjanela, i + 1, j * 2, ". ");
            } else {
                mvwprintw(mainjanela, i + 1, j * 2, "%d ", sudoku->sudoku[i][j]);
            }
        }
    }
    wrefresh(mainjanela);
}

bool numeroValido(int numero) {
    if (numero < 1 || numero > 9) {
        printw("Número inválido\n");
        return false;
    }
    return true;
}

/**
 * Verifica se o caminho de imprimeSudoku() já não está ocupado por outro número
*/
bool caminhoValido(Sudoku *sudoku, int linha, int coluna, int numero) {
    for (int i = 0; i < SIZE; i++) {
        if (sudoku->sudoku[linha][i] == numero) {
            return false;
        }
        if (sudoku->sudoku[i][coluna] == numero) {
            return false;
        }
    }
    int linhaInicial = linha - linha % 3;
    int colunaInicial = coluna - coluna % 3;
    for (int i = linhaInicial; i < linhaInicial + 3; i++) {
        for (int j = colunaInicial; j < colunaInicial + 3; j++) {
            if (sudoku->sudoku[i][j] == numero) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Insere numero valido num caminho valido que não esteja ocupado por outro numero em mainjanela
 * Em infojanela, imprime a mensagem de erro caso o numero seja invalido
 * O input do usuario será recebido por codjanela
*/
void insereNumero(Sudoku *sudoku, char *posicao, int numero, WINDOW *mainjanela, WINDOW *infojanela, WINDOW *codjanela) {
    int linha = posicao[0] - 'A';
    int coluna = posicao[1] - '1';
    if (numeroValido(numero) && caminhoValido(sudoku, linha, coluna, numero)) {
        sudoku->sudoku[linha][coluna] = numero;
    } else {
        mvwprintw(infojanela, 1, 1, "Número inválido");
        wrefresh(infojanela);
    }
    mvwprintw(infojanela, 1, 1, "Valido");
    imprimeSudoku(sudoku, mainjanela);
}

/**
 * Implemente a função sair para a janela principal do jogo
*/
void sair() {
    endwin();
    exit(0);
}

/**
 * Implementa a lógica do jogo, algortimo para checar se todas as colunas e todos os quadrantes estão com numeros únicos
 * se houver número repetido, imprime a mensagem de erro em infojanela
 * se todas as colunas e quadrantes estiverem corretos, imprime a mensagem de sucesso em infojanela
*/
void verificaJogo(Sudoku *sudoku, WINDOW *infojanela) {
    for (int i = 0; i < SIZE; i++) {
        int linha[SIZE] = {0};
        int coluna[SIZE] = {0};
        int quadrante[SIZE] = {0};
        for (int j = 0; j < SIZE; j++) {
            linha[sudoku->sudoku[i][j] - 1]++;
            coluna[sudoku->sudoku[j][i] - 1]++;
            quadrante[sudoku->sudoku[(i / 3) * 3 + j / 3][(i % 3) * 3 + j % 3] - 1]++;
        }
        for (int j = 0; j < SIZE; j++) {
            if (linha[j] != 1 || coluna[j] != 1 || quadrante[j] != 1) {
                mvwprintw(infojanela, 1, 1, "Erro, número repetido");
                wrefresh(infojanela);
                return;
            }
        }
    }
    mvwprintw(infojanela, 1, 1, "Sucesso");
    wrefresh(infojanela);
}

/**
 * Salva o jogo em um arquivo .csv salvo na pasta ./Downloads do Linux
*/
void salvaJogo(Sudoku *sudoku) {
    FILE *arquivo = fopen("/home/Downloads/sudoku.csv", "w");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fprintf(arquivo, "%d", sudoku->sudoku[i][j]);
            if (j < SIZE - 1) {
                fprintf(arquivo, ",");
            }
        }
        fprintf(arquivo, "\n");
    }
    fclose(arquivo);
}

/**
 * Tratamento para transformar um arquivo .csv em uma matriz em C
 * 1, 2, 3, 
 * 4, 5, 6, 
 * 7, 8, 9,
 * para 
 * [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
*/

void csvToMatrix(Sudoku *sudoku) {
    FILE *arquivo = fopen("/home/Downloads/sudoku.csv", "r");
    char linha[1024];
    int i = 0, j = 0;

    // Lê cada linha do arquivo
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Quebra a linha em tokens
        char *token = strtok(linha, ",");

        // Armazena o valor na matriz
        sudoku->sudoku[i][j] = *token;

        // Avança para o próximo token
        token = strtok(NULL, ",");

        // Incrementa o índice da coluna
        j++;

        // Se chegou ao final da linha, reinicia o índice da coluna e incrementa o índice da linha
        if (token == NULL) {
            j = 0;
            i++;
        }
    }

    // Fecha o arquivo
    fclose(arquivo);
}

/**
 * Carrega o jogo a partir de um arquivo .csv salvo na pasta ./Downloads do Linux e manda para a mainjanela
*/
void carregaJogo(Sudoku *sudoku, WINDOW *mainjanela) {
    csvToMatrix(sudoku);
    imprimeSudoku(sudoku, mainjanela);
}

// Função se verifica se o jogo foi ganhado corretamente
//compara a matriz resultado com a matriz jogo e se igual retorna true, senão falso
// por ideia coloque a função veri

bool jogoResolvido(Sudoku *sudoku, WINDOW *infojanela, Sudoku *resultado) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (sudoku->sudoku[i][j] == VAZIO) {
                return false;
            }
        }
    }
//comparar duas matrizes
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (sudoku->sudoku[i][j] != resultado->sudoku[i][j]) {
                mvwprintw(infojanela, 1, 1, "Fim de jogo, você perdeu!");
                return false;
            }
        }
    }
     mvwprintw(infojanela, 1, 1, "Fim de jogo, você ganhou!");
    return true;
}

int main(int argc, int *argv[]) {
    initscr(); // Inicia o modo ncurses
    Sudoku sudoku;
    Sudoku resultado;
    WINDOW *menujanela, *mainjanela, *infojanela, *codjanela;
    int opcao;
    while (1) {
        menujanela = menuJanela(5, 20, 0, 0);
        refresh();
        opcao = wgetch(menujanela);
        if (opcao == '1') {
            mainjanela = jogoJanela(20, 20, 0, 20);
            infojanela = infoJanela(5, 40, 0, 40);
            codjanela = comandoJanela(5, 40, 20, 40);
            break;
        } else if (opcao == '2') {
            carregaJogo(&sudoku, mainjanela);
            mainjanela = jogoJanela(20, 20, 0, 20);
            infojanela = infoJanela(5, 40, 0, 40);
            codjanela = comandoJanela(5, 40, 20, 40);
            break;
        } else if (opcao == '3') {
            sair();
        }
    }
    while (1) {
        imprimeSudoku(&sudoku, mainjanela);
        int numero = wgetch(codjanela);
        char posicao[2];
        mvwprintw(infojanela, 1, 1, "Digite a posição: ");
        wrefresh(infojanela);
        wgetnstr(infojanela, posicao, 2);
        if (posicao[0] == 'S' && posicao[1] == 'A') {
            sair();
        }
        insereNumero(&sudoku, posicao, numero - '0', mainjanela, infojanela, codjanela);
        verificaJogo(&sudoku, infojanela);
        if (jogoResolvido(&sudoku, infojanela, &resultado)) {
            break;
        }
    }
    destruirJanelas(codjanela, infojanela, mainjanela);
    endwin();
    return 0;
}