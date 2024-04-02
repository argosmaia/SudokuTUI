#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <ncurses.h>
#include <time.h>

/**
 * Definição MACRO do tamanho do sudoku e do valor vazio
*/
#define SIZE 9
#define VAZIO 0

typedef struct {
    int sudoku[SIZE][SIZE];
} Sudoku;

//Macros para mensagens de erro
#define NUMERO_INVALIDO = "Erro, número inválido"
#define NUMERO_VALIDO = "Número válido"
#define CAMINHO_INVALIDO = "Erro, caminho inválido"
#define NUMERO_REPETIDO = "Erro, número repetido"
#define GANHOU = "Fim de jogo, você ganhou!"
#define PERDEU = "Fim de jogo, você perdeu!"


//Definição globais para as funções
WINDOW *menuJanela(int altura, int largura, int inicioY, int inicioX);
WINDOW *jogoJanela(int altura, int largura, int inicioY, int inicioX);
WINDOW *infoJanela(int altura, int largura, int inicioY, int inicioX);
WINDOW *comandoJanela(int altura, int largura, int inicioY, int inicioX);
void imprimeSudoku(Sudoku *sudoku, WINDOW *mainjanela);
bool numeroValido(int numero);
bool caminhoValido(Sudoku *sudoku, int linha, int coluna, int numero);
void insereNumero(Sudoku *sudoku, char *posicao, int numero, WINDOW *mainjanela, WINDOW *infojanela, WINDOW *codjanela);
void sair(Sudoku *sudoku, WINDOW *mainjanela, WINDOW *infojanela, WINDOW *codjanela);
void verificaJogo(Sudoku *sudoku, WINDOW *infojanela);
void salvaJogo(Sudoku *sudoku);
void csvToMatrix(Sudoku *sudoku);
void carregaJogo(Sudoku *sudoku, WINDOW *mainjanela);
bool jogoResolvido(Sudoku *sudoku, WINDOW *infojanela, Sudoku *resultado);
bool pontoOcupado(Sudoku *sudoku, char *posicao);

/** 
 * Janela de Menu que será exibida ao usuário um Menu e levará as janela de jogoJanela, infoJanela e comandoJanela ( o Jogo)
 * se digitado 1, o jogo será iniciado
 * se digitado 2, o jogo será carregado a a partir de um save
 * se digitado 3, o jogo será encerrado
*/
WINDOW *menuJanela(int altura, int largura, int inicioY, int inicioX) {
    WINDOW *menujanela;
    int opc = 0;
    while (true) {
        menujanela = newwin(altura, largura, inicioY, inicioX);
        box(menujanela, 0, 0);
        mvwprintw(menujanela, 1, 1, "1 - Novo Jogo");
        mvwprintw(menujanela, 2, 1, "2 - Carregar Jogo");
        mvwprintw(menujanela, 3, 1, "3 - Sair");
        wrefresh(menujanela);
        opc = wgetch(menujanela);
        if (opc >= '1' && opc <= '3') {
            break; // Opção válida, sair do loop
        } else {
            mvwprintw(menujanela, 4, 1, "Opção inválida! Pressione 1, 2 ou 3.");
            wrefresh(menujanela);
        }
    }
    delwin(menujanela);
    return opc;
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
    mvwprintw(infojanela, 1, 1, "Logs de jogo");
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

/**
 * Gera numeros aleatórios e válidos para o sudoku e insere aleatoriamente na matriz (A1 até I9, somente)
 * Geralmente 3 ou 4 números não repetidos são suficientes por quadrante
*/
void geraNumPosAleatoria(Sudoku *sudoku) {
    memset(sudoku->sudoku, VAZIO, sizeof(sudoku->sudoku));
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int numero;
            do {
                numero = rand() % SIZE + 1;
            } while (!caminhoValido(sudoku, i, j, numero));
            sudoku->sudoku[i][j] = numero;
        }
    }
}

// Responsável por imprimir o sudoku na mainjanela
/**
 * Imprime o sudoku na tela principal do jogo junto com as divisões de quadrantes e os números aleatorios gerados pela função geraNumPosAleatoria() além das posições vazias (VAZIO)
*/
void imprimeSudoku(Sudoku *sudoku, WINDOW *mainjanela) {
    wclear(mainjanela);
    box(mainjanela, 0, 0);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            mvwprintw(mainjanela, i + 1, j * 2 + 1, "%d", sudoku->sudoku[i][j]);
        }
    }
    wrefresh(mainjanela);
}

/**
 * Verifica se o número inserido pelo usuário é válido
 * Se o número for menor que 0 ou maior que 9, imprime a mensagem de erro em infojanela
 * Se o número for válido, imprime a mensagem de sucesso em infojanela
 * Se o número for 0, salvar o jogo (já impĺementando dentro de sair()) e saimos do jogo
*/
bool numeroValido(int numero) {
    if (numero < 0 || numero > 9) {
        return false;
    }
    return true;
}

/**
 * Verifica se o caminho de imprimeSudoku() já não está ocupado por outro número em uma posição já ocupada por um número
 * Se o número já estiver na linha, coluna ou quadrante, retorna falso
 * Se o número não estiver na linha, coluna ou quadrante, retorna verdadeiro e insere o número na posição
*/
bool caminhoValido(Sudoku *sudoku, int linha, int coluna, int numero) {
    // Verifica se o número já está na linha, coluna ou quadrante
    for (int i = 0; i < SIZE; i++) {
        if (sudoku->sudoku[linha][i] == numero) { // Verifica se o número já está na linha
            return false;
        }
        if (sudoku->sudoku[i][coluna] == numero) { // Verifica se o número já está na coluna
            return false;
        }
    }

    int linhaInicial = linha - linha % 3;
    int colunaInicial = coluna - coluna % 3;
    
    for (int i = linhaInicial; i < linhaInicial + 3; i++) { // Verifica se o número já está no quadrante
        for (int j = colunaInicial; j < colunaInicial + 3; j++) { // Verifica se o número já está no quadrante
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
 * O input do usuario será recebido por WINDOW *codjanela
 * E será impresso em mainjanela
 * A janela de informações será atualizada com mensagens de erro e numeros inseridos
*/
void insereNumero(Sudoku *sudoku, char *posicao, int numero, WINDOW *mainjanela, WINDOW *infojanela, WINDOW *codjanela) {
    int linha = posicao[0] - 'A';
    int coluna = posicao[1] - '1';
    if (!numeroValido(numero)) {
        mvwprintw(infojanela, 1, 1, "Erro, número inválido");
        wrefresh(infojanela);
        return;
    }
    if (pontoOcupado(sudoku, posicao)) {
        mvwprintw(infojanela, 1, 1, "Erro, ponto ocupado");
        wrefresh(infojanela);
        return;
    }
    sudoku->sudoku[linha][coluna] = numero;
    imprimeSudoku(sudoku, mainjanela);
    mvwprintw(infojanela, 1, 1, "Número inserido: %d", numero);
    wrefresh(infojanela);
}

/**
 * Implemente a função sair para a janela principal do jogo
*/
void sair(Sudoku *sudoku, WINDOW *mainjanela, WINDOW *infojanela, WINDOW *codjanela) {
    // Salva o jogo antes de sair
    salvaJogo(&sudoku);
    endwin();
    exit(0);
    destruirJanelas(codjanela, infojanela, mainjanela);
}

/**
 * Implementa a lógica do jogo, algortimo para checar se todas as colunas e todos os quadrantes estão com numeros únicos
 * se houver número repetido, imprime a mensagem de erro em infojanela
 * se todas as colunas e quadrantes estiverem corretos, imprime a mensagem de sucesso em infojanela
 * coloca-se a função dentro 
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




/**
 * Determina se dado ponto (E1, E4, A2, etc...) está ocupado por um número que foi inserido pelo usuário ou pelo proprio jogo
*/
bool pontoOcupado(Sudoku *sudoku, char *posicao) {
    int linha = posicao[0] - 'A';
    int coluna = posicao[1] - '1';
    if (sudoku->sudoku[linha][coluna] != VAZIO) {
        return true;
    }
    return false;
}

/**
 * 
 * Função principal que chama todas as outras funções
 * Inicializa o ncurses e cria as janelas
 * Inicializa o sudoku e gera números aleatórios
 * Enquanto o usuário não sair, o jogo continua
 * O usuário pode inserir números, verificar o jogo, salvar o jogo, carregar o jogo e sair
 * Ao sair, as janelas são destruídas e o ncurses é finalizado
 * 
*/

// Função principal
int main() {
    // Inicialização do ncurses
    initscr();
    noecho();
    curs_set(0);
    srand(time(NULL));

    // Criação das janelas
    WINDOW *menujanela = menuJanela(5, 20, 0, 0);
    WINDOW *mainjanela = jogoJanela(11, 19, 5, 0);
    WINDOW *infojanela = infoJanela(5, 19, 16, 0);
    WINDOW *codjanela = comandoJanela(3, 19, 20, 0);
    wrefresh(menujanela);
    wrefresh(mainjanela);
    wrefresh(infojanela);
    wrefresh(codjanela);

    // Criação do sudoku e geração de números aleatórios
    Sudoku sudoku;
    Sudoku resultado;
    geraNumPosAleatoria(&sudoku);
    geraNumPosAleatoria(&resultado);
    imprimeSudoku(&sudoku, mainjanela);

    // Criação das janelas
    char opcao = menuJanela(5, 20, 0, 0);
    switch (opcao) {
        case '1':
            // Código para iniciar um novo jogo
            break;
        case '2':
            // Código para carregar um jogo
            break;
        case '3':
            // Código para sair do jogo
            break;
        default:
            // Isso não deve acontecer, mas se acontecer, pode lidar com isso aqui
            break;
    }

    // Loop principal
    char opc; // Alteração do tipo de variável

    while (true) {
        opc = wgetch(codjanela);
        // Converta opc para char para comparar corretamente
        switch (opc) {
            case '1':
                insereNumero(&sudoku, "A1", 1, mainjanela, infojanela, codjanela);
                break;
            case '2':
                verificaJogo(&sudoku, infojanela);
                break;
            case '3':
                salvaJogo(&sudoku);
                break;
            case '4':
                sair(&sudoku, mainjanela, infojanela, codjanela);
                break;
            case '5':
                carregaJogo(&sudoku, mainjanela);
                break;
            case '6':
                jogoResolvido(&sudoku, infojanela, &resultado);
                break;
            default:
                break;
        }
    }

    // Destruição das janelas
    destruirJanelas(codjanela, infojanela, mainjanela);

    // Finalização do ncurses
    endwin();
    return 0;
}
