#include <stdio.h>      // Biblioteca padrão: entrada/saída (printf, scanf, etc)
#include <stdlib.h>     // Biblioteca padrão: funções gerais (malloc, exit, etc)
#include <string.h>     // Manipulação de strings (strcpy, strlen, etc)
#include <unistd.h>     // Funções POSIX (sleep, etc)
#include <termios.h>    // Controle do terminal (modos de entrada, echo, etc)
#include <signal.h>     // Tratamento de sinais

// Define os limites para o número de tarefas e tamanho da descrição
#define MAX_TAREFAS 100
#define TAMANHO_DESCRICAO 100

// Estrutura que define uma tarefa, contendo descrição, status e prioridade
typedef struct {
    char descricao[TAMANHO_DESCRICAO];  // Texto descritivo da tarefa
    int status;      // Status: 0 = A Fazer, 1 = Fazendo, 2 = Feito
    int prioridade;  // Prioridade: 0 = Emergente, 1 = Urgente, 2 = Importante
} Tarefa;

// Vetor global para armazenar as tarefas e variáveis de controle
Tarefa tarefas[MAX_TAREFAS];
int quantidade_tarefas = 0;
int aba_selecionada = 0;     // 0: A Fazer, 1: Fazendo, 2: Feito (abas do menu)
int tarefa_selecionada = 0;    // Índice relativo da tarefa exibida (ordenada por prioridade)

// Variável para armazenar a configuração original do terminal
struct termios config_original;

// ================= Persistência (Salvar/Carregar) =================

// Salva as tarefas em um arquivo "tarefas.txt"       //PRIORIDADE//STATUS//DESCRICAO
void salvar_tarefas() {
    FILE *fp = fopen("tarefas.txt", "w");
    if (!fp) {
        perror("Erro ao salvar tarefas");
        return;
    }
    // Para cada tarefa, escreve status, prioridade e descrição (separados por tabulação)
    for (int i = 0; i < quantidade_tarefas; i++) {
        fprintf(fp, "%d\t%d\t%s\n", tarefas[i].status, tarefas[i].prioridade, tarefas[i].descricao);
    }
    fclose(fp);
}

// Carrega as tarefas do arquivo "tarefas.txt" (se existir)
void carregar_tarefas() {
    FILE *fp = fopen("tarefas.txt", "r");
    if (!fp) return;  // Se o arquivo não existir, ignora
    quantidade_tarefas = 0;
    while (quantidade_tarefas < MAX_TAREFAS &&
           fscanf(fp, "%d\t%d\t%[^\n]\n", &tarefas[quantidade_tarefas].status,
                  &tarefas[quantidade_tarefas].prioridade,
                  tarefas[quantidade_tarefas].descricao) == 3) {
        quantidade_tarefas++;
    }
    fclose(fp);
}

// ================= Controle do Terminal =================

// Restaura a configuração original do terminal (usado ao sair)
void restaurarTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &config_original);
}

// Desativa o modo canônico e o echo (modo "raw") para a navegação com setas
void desativarBufferDeEntrada() {
    struct termios config;
    tcgetattr(STDIN_FILENO, &config);
    config_original = config;  // Salva configuração original
    config.c_lflag &= ~(ICANON | ECHO);  // Desativa modo canônico e echo
    tcsetattr(STDIN_FILENO, TCSANOW, &config);
}

// Habilita o modo canônico com echo para leitura de entradas de texto (ex: descrição)
void habilitarModoCanonical() {
    tcsetattr(STDIN_FILENO, TCSANOW, &config_original);
}

// Retorna ao modo raw (não-canônico) após a leitura
void desativarModoCanonical() {
    desativarBufferDeEntrada();
}

// Limpa a tela usando códigos ANSI e reposiciona o cursor no topo
void limpar_tela() {
    printf("\033[2J\033[H");
}

// Reseta as cores/formatos ANSI (remove formatação)
void resetCor() {
    printf("\033[0m");
}

// ================= Funções Auxiliares =================

// Retorna o nome da prioridade (ex: "Emergente") conforme o número (0,1 ou 2)
const char* getNomePrioridade(int p) {
    switch(p) {
        case 0: return "Emergente";
        case 1: return "Urgente";
        case 2: return "Importante";
        default: return "Desconhecida";
    }
}

// Cria um array de índices das tarefas da aba 'tab' (status igual a tab),
// ordena por prioridade (menor número = maior prioridade) e retorna a quantidade encontrada.
int get_sorted_indices(int tab, int indices[]) {
    int count = 0;
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == tab) {
            indices[count++] = i;
        }
    }
    // Ordenação simples (bubble sort - ap1)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (tarefas[indices[i]].prioridade > tarefas[indices[j]].prioridade) {
                int temp = indices[i];
                indices[i] = indices[j];
                indices[j] = temp;
            }
        }
    }
    return count;
}

// Limpa o buffer de entrada para evitar caracteres residuais
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ================= Menus de Seleção com Setas =================

// Menu para selecionar a prioridade usando setas (↑/↓) e ENTER.
// Retorna 0 (Emergente), 1 (Urgente) ou 2 (Importante)
int selecionar_prioridade() {
    int sel = 0;
    while (1) {
        limpar_tela();
        printf("\033[1;34mSelecione a Prioridade:\033[0m\n\n");
        char *opcoes[] = {"Emergente", "Urgente", "Importante"};
        for (int i = 0; i < 3; i++) {
            if (i == sel)
                printf("\033[7m%s\033[0m\n", opcoes[i]);  // Destaca a opção selecionada (vídeo invertido)
            else
                printf("%s\n", opcoes[i]);
        }
        printf("\nUse as setas (↑/↓) e ENTER para selecionar.");
        int c = getchar();
        if (c == '\033') { // Código de escape
            getchar(); // ignora o '['
            char d = getchar();
            if (d == 'A') {         // seta para cima
                if (sel > 0)
                    sel--;
            } else if (d == 'B') {  // seta para baixo
                if (sel < 2)
                    sel++;
            }
        } else if (c == '\n' || c == '\r') {
            return sel;
        }
    }
}

// Menu para selecionar o status (A Fazer, Fazendo, Feito) usando setas e ENTER.
int selecionar_status() {
    int sel = 0;
    while (1) {
        limpar_tela();
        printf("\033[1;34mSelecione o Status:\033[0m\n\n");
        char *opcoes[] = {"A Fazer", "Fazendo", "Feito"};
        for (int i = 0; i < 3; i++) {
            if (i == sel)
                printf("\033[7m%s\033[0m\n", opcoes[i]);  // Destaca a opção selecionada
            else
                printf("%s\n", opcoes[i]);
        }
        printf("\nUse as setas (↑/↓) e ENTER para selecionar.");
        int c = getchar();
        if (c == '\033') {
            getchar(); // ignora o '['
            char d = getchar();
            if (d == 'A') {
                if (sel > 0)
                    sel--;
            } else if (d == 'B') {
                if (sel < 2)
                    sel++;
            }
        } else if (c == '\n' || c == '\r') {
            return sel;
        }
    }
}

// Menu para confirmar a exclusão com as opções "Sim" e "Não" usando setas e ENTER.
// Retorna 0 para "Sim" e 1 para "Não".
int selecionar_confirmacao() {
    int sel = 0;  // Inicialmente "Sim"
    while (1) {
        limpar_tela();
        printf("\033[1;34mConfirma a exclusão?\033[0m\n\n");
        char *opcoes[] = {"Sim", "Não"};
        for (int i = 0; i < 2; i++) {
            if (i == sel)
                printf("\033[7m%s\033[0m\n", opcoes[i]);  // Destaca a opção selecionada
            else
                printf("%s\n", opcoes[i]);
        }
        printf("\nUse as setas (←/→ ou ↑/↓) e ENTER para selecionar.");
        int c = getchar();
        if (c == '\033') {
            getchar(); // ignora o '['
            char d = getchar();
            // Usamos tanto setas para cima/para a esquerda para diminuir, e para baixo/direita para aumentar
            if (d == 'A' || d == 'D') {
                if (sel > 0)
                    sel--;
            } else if (d == 'B' || d == 'C') {
                if (sel < 1)
                    sel++;
            }
        } else if (c == '\n' || c == '\r') {
            return sel;  // 0 para Sim, 1 para Não
        }
    }
}

// ================= Exibição do Menu Principal =================

// Mostra o menu principal, as abas e as tarefas ordenadas por prioridade
void mostrar_menu() {
    limpar_tela();

    // Exibe o título
    printf("\033[1;34m=== Gerenciador de Tarefas ===\033[0m\n\n");

    // Exibe as abas (A Fazer, Fazendo, Feito)
    char *abas[] = {"A Fazer", "Fazendo", "Feito"};
    for (int i = 0; i < 3; i++) {
        if (i == aba_selecionada)
            printf("\033[1;30;43m %s \033[0m   ", abas[i]);  // Aba selecionada com fundo amarelo
        else
            printf(" %s    ", abas[i]);
    }
    printf("\n\n");

    // Exibe as tarefas da aba selecionada, ordenadas por prioridade
    int indices[MAX_TAREFAS];
    int count = get_sorted_indices(aba_selecionada, indices);
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        if (i == tarefa_selecionada) {
            printf("\033[4m");  // Sublinha a tarefa selecionada
            printf("%d: %s [Prioridade: %s]\n", i + 1, tarefas[idx].descricao, getNomePrioridade(tarefas[idx].prioridade));
            resetCor();
        } else {
            printf("%d: %s [Prioridade: %s]\n", i + 1, tarefas[idx].descricao, getNomePrioridade(tarefas[idx].prioridade));
        }
    }

    // Exibe as instruções para o usuário
    printf("\n\033[1;32mTeclas: setas - Navegar | + - Adicionar | e - Editar | s - Mudar status | x - Excluir | q - Sair\033[0m\n");
}

// ================= Funções de Manipulação de Tarefas =================

// Adiciona uma nova tarefa
void adicionar_tarefa() {
    limpar_tela();
    printf("\033[1;34mAdicionar Tarefa\033[0m\n\n");
    char descricao[TAMANHO_DESCRICAO];
    int prioridade;

    // Lê a descrição com o modo canônico (com echo) para que o usuário veja o que digita
    printf("Digite a descrição da tarefa: ");
    habilitarModoCanonical();
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = '\0';
    desativarModoCanonical();

    // Permite a seleção da prioridade via menu com setas
    printf("Selecione a prioridade:\n");
    prioridade = selecionar_prioridade();

    if (quantidade_tarefas < MAX_TAREFAS) {
        tarefas[quantidade_tarefas].status = aba_selecionada;
        tarefas[quantidade_tarefas].prioridade = prioridade;
        strcpy(tarefas[quantidade_tarefas].descricao, descricao);
        quantidade_tarefas++;
        printf("\nTarefa adicionada! Pressione ENTER para continuar...");
    } else {
        printf("\nLimite de tarefas atingido! Pressione ENTER para continuar...");
    }
    habilitarModoCanonical();
    getchar();
    desativarModoCanonical();
}

// Edita uma tarefa já existente
void editar_tarefa() {
    int indices[MAX_TAREFAS];
    int count = get_sorted_indices(aba_selecionada, indices);
    if (count == 0 || tarefa_selecionada >= count) {
        printf("\nNenhuma tarefa selecionada! Pressione ENTER para continuar...");
        habilitarModoCanonical();
        getchar();
        desativarModoCanonical();
        return;
    }
    int index = indices[tarefa_selecionada];

    limpar_tela();
    printf("\033[1;34mEditar Tarefa\033[0m\n\n");
    printf("Tarefa atual: %s\n", tarefas[index].descricao);

    char descricao[TAMANHO_DESCRICAO];
    int prioridade;
    printf("Digite a nova descrição: ");
    habilitarModoCanonical();
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = '\0';
    desativarModoCanonical();

    printf("Selecione a nova prioridade:\n");
    prioridade = selecionar_prioridade();

    strcpy(tarefas[index].descricao, descricao);
    tarefas[index].prioridade = prioridade;
    printf("\nTarefa editada! Pressione ENTER para continuar...");
    habilitarModoCanonical();
    getchar();
    desativarModoCanonical();
}

// Altera o status de uma tarefa usando menu com setas
void mudar_status() {
    int indices[MAX_TAREFAS];
    int count = get_sorted_indices(aba_selecionada, indices);
    if (count == 0 || tarefa_selecionada >= count) {
        printf("\nNenhuma tarefa selecionada! Pressione ENTER para continuar...");
        habilitarModoCanonical();
        getchar();
        desativarModoCanonical();
        return;
    }
    int index = indices[tarefa_selecionada];

    limpar_tela();
    printf("\033[1;34mMudar Status\033[0m\n\n");
    printf("Tarefa: %s\n", tarefas[index].descricao);

    // Permite a seleção do novo status via menu com setas
    int novo_status = selecionar_status();
    tarefas[index].status = novo_status;
    printf("\nStatus alterado! Pressione ENTER para continuar...");
    habilitarModoCanonical();
    getchar();
    desativarModoCanonical();
}

// Exclui uma tarefa após confirmação via menu com setas (Sim/Não)
void excluir_tarefa() {
    int indices[MAX_TAREFAS];
    int count = get_sorted_indices(aba_selecionada, indices);
    if (count == 0 || tarefa_selecionada >= count) {
        printf("\nNenhuma tarefa selecionada! Pressione ENTER para continuar...");
        habilitarModoCanonical();
        getchar();
        desativarModoCanonical();
        return;
    }
    int index = indices[tarefa_selecionada];

    limpar_tela();
    printf("\033[1;34mExcluir Tarefa\033[0m\n\n");
    printf("Excluir a tarefa: %s?\n", tarefas[index].descricao);
    // Menu de confirmação: seleciona com setas "Sim" ou "Não"
    int confirm = selecionar_confirmacao();
    if (confirm == 0) { // 0 significa Sim
        for (int i = index; i < quantidade_tarefas - 1; i++) {
            tarefas[i] = tarefas[i + 1];
        }
        quantidade_tarefas--;
        if (tarefa_selecionada > 0)
            tarefa_selecionada--;
        printf("\nTarefa excluída! Pressione ENTER para continuar...");
    } else {
        printf("\nOperação cancelada. Pressione ENTER para continuar...");
    }
    habilitarModoCanonical();
    getchar();
    desativarModoCanonical();
}

// ================= Função Main =================

int main() {
    // Configura o terminal para modo raw (não-canônico, sem echo)
    // e garante que a configuração original será restaurada ao sair
    desativarBufferDeEntrada();
    atexit(restaurarTerminal);

    // Carrega as tarefas salvas (se houver)
    carregar_tarefas();

    // Loop principal do programa
    while (1) {
        mostrar_menu();
        int tecla = getchar();
        if (tecla == '\033') {  // Se a tecla for a sequência de escape (setas)
            getchar(); // ignora o '['
            char direcao = getchar();
            if (direcao == 'A') {         // seta para cima
                if (tarefa_selecionada > 0)
                    tarefa_selecionada--;
            } else if (direcao == 'B') {   // seta para baixo
                int indices[MAX_TAREFAS];
                int count = get_sorted_indices(aba_selecionada, indices);
                if (tarefa_selecionada < count - 1)
                    tarefa_selecionada++;
            } else if (direcao == 'C') {   // próxima aba
                if (aba_selecionada < 2) {
                    aba_selecionada++;
                    tarefa_selecionada = 0;
                }
            } else if (direcao == 'D') {   // aba anterior
                if (aba_selecionada > 0) {
                    aba_selecionada--;
                    tarefa_selecionada = 0;
                }
            }
        } else if (tecla == '+') {
            adicionar_tarefa();
        } else if (tecla == 'e') {
            editar_tarefa();
        } else if (tecla == 's') {
            mudar_status();
        } else if (tecla == 'x') {
            excluir_tarefa();
        } else if (tecla == 'q') {
            break;  // Sai do loop principal e encerra o programa
        }
    }
    // Antes de sair, retorna ao modo canônico e salva as tarefas
    habilitarModoCanonical();
    salvar_tarefas();
    return 0;
}
