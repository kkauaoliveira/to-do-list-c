#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#define MAX_TAREFAS 100
#define TAMANHO_DESCRICAO 100

typedef struct {
    char descricao[TAMANHO_DESCRICAO];
    int status;      // 0: A Fazer, 1: Fazendo, 2: Feito
    int prioridade;  // 0: Emergente, 1: Urgente, 2: Importante
} Tarefa;

Tarefa tarefas[MAX_TAREFAS];
int quantidade_tarefas = 0;
int aba_selecionada = 0;     // 0: A Fazer, 1: Fazendo, 2: Feito
int tarefa_selecionada = 0;    // índice relativo às tarefas exibidas (ordenadas por prioridade)

struct termios config_original;

// ================= Persistência (Salvar/Carregar) =================

void salvar_tarefas() {
    FILE *fp = fopen("tarefas.txt", "w");
    if (!fp) {
        perror("Erro ao salvar tarefas");
        return;
    }
    for (int i = 0; i < quantidade_tarefas; i++) {
        // Armazena: status, prioridade e descrição (separados por tab)
        fprintf(fp, "%d\t%d\t%s\n", tarefas[i].status, tarefas[i].prioridade, tarefas[i].descricao);
    }
    fclose(fp);
}

void carregar_tarefas() {
    FILE *fp = fopen("tarefas.txt", "r");
    if (!fp) return;  // Se não existir arquivo, ignora.
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

// Restaura as configurações originais do terminal ao sair.
void restaurarTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &config_original);
}

// Desativa o modo canônico e o echo (modo "raw") para a navegação principal.
void desativarBufferDeEntrada() {
    struct termios config;
    tcgetattr(STDIN_FILENO, &config);
    config_original = config;  // Salva a configuração original
    config.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &config);
}

// Habilita o modo canônico com echo (usado para entrada de texto).
void habilitarModoCanonical() {
    tcsetattr(STDIN_FILENO, TCSANOW, &config_original);
}

// Retorna ao modo não-canônico após a leitura.
void desativarModoCanonical() {
    desativarBufferDeEntrada();
}

void limpar_tela() {
    // ANSI escape para limpar a tela e reposicionar o cursor no canto superior esquerdo.
    printf("\033[2J\033[H");
}

void resetCor() {
    printf("\033[0m");
}

// ================= Funções Auxiliares =================

// Retorna o nome da prioridade conforme o número.
const char* getNomePrioridade(int p) {
    switch(p) {
        case 0: return "Emergente";
        case 1: return "Urgente";
        case 2: return "Importante";
        default: return "Desconhecida";
    }
}

// Cria um array de índices para as tarefas da aba 'tab', ordenadas por prioridade (0 = maior).
// Retorna a quantidade de tarefas encontradas.
int get_sorted_indices(int tab, int indices[]) {
    int count = 0;
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == tab) {
            indices[count++] = i;
        }
    }
    // Ordenação simples (bubble sort) por prioridade (menor número = prioridade mais alta)
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

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ================= Menu de Seleção de Prioridade =================

int selecionar_prioridade() {
    int sel = 0;
    while (1) {
        limpar_tela();
        printf("\033[1;34mSelecione a Prioridade:\033[0m\n\n");
        char *opcoes[] = {"Emergente", "Urgente", "Importante"};
        for (int i = 0; i < 3; i++) {
            if (i == sel)
                // Destaque com vídeo invertido.
                printf("\033[7m%s\033[0m\n", opcoes[i]);
            else
                printf("%s\n", opcoes[i]);
        }
        printf("\nUse as setas (↑/↓) e ENTER para selecionar.");
        int c = getchar();
        if (c == '\033') {
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

// ================= Exibição do Menu Principal =================

void mostrar_menu() {
    limpar_tela();

    // Título
    printf("\033[1;34m=== Gerenciador de Tarefas ===\033[0m\n\n");

    // Exibe as abas
    char *abas[] = {"A Fazer", "Fazendo", "Feito"};
    for (int i = 0; i < 3; i++) {
        if (i == aba_selecionada) {
            // Aba selecionada: fundo amarelo com texto escuro
            printf("\033[1;30;43m %s \033[0m   ", abas[i]);
        } else {
            printf(" %s    ", abas[i]);
        }
    }
    printf("\n\n");

    // Exibe as tarefas da aba selecionada, ordenadas por prioridade
    int indices[MAX_TAREFAS];
    int count = get_sorted_indices(aba_selecionada, indices);
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        if (i == tarefa_selecionada) {
            // Tarefa selecionada: sublinhada
            printf("\033[4m");
            printf("%d: %s [Prioridade: %s]\n", i + 1, tarefas[idx].descricao, getNomePrioridade(tarefas[idx].prioridade));
            resetCor();
        } else {
            printf("%d: %s [Prioridade: %s]\n", i + 1, tarefas[idx].descricao, getNomePrioridade(tarefas[idx].prioridade));
        }
    }

    // Instruções
    printf("\n\033[1;32mTeclas: setas - Navegar | + - Adicionar | e - Editar | s - Mudar status | x - Excluir | q - Sair\033[0m\n");
}

// ================= Funções de Manipulação de Tarefas =================

void adicionar_tarefa() {
    limpar_tela();
    printf("\033[1;34mAdicionar Tarefa\033[0m\n\n");
    char descricao[TAMANHO_DESCRICAO];
    int prioridade;

    // Entrada da descrição com modo canônico (com echo)
    printf("Digite a descrição da tarefa: ");
    habilitarModoCanonical();
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = '\0';
    desativarModoCanonical();

    // Seleção da prioridade via menu
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

    int novo_status;
    printf("Digite o novo status (0: A Fazer, 1: Fazendo, 2: Feito): ");
    habilitarModoCanonical();
    scanf("%d", &novo_status);
    while(getchar() != '\n');
    desativarModoCanonical();

    if (novo_status >= 0 && novo_status <= 2) {
        tarefas[index].status = novo_status;
        printf("\nStatus alterado! Pressione ENTER para continuar...");
    } else {
        printf("\nStatus inválido! Pressione ENTER para continuar...");
    }
    habilitarModoCanonical();
    getchar();
    desativarModoCanonical();
}

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
    printf("Tem certeza que deseja excluir a tarefa: %s ? (y/n): ", tarefas[index].descricao);
    habilitarModoCanonical();
    int op = getchar();
    desativarModoCanonical();

    if (op == 'y' || op == 'Y') {
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

// ================= Main =================

int main() {
    // Configura o terminal para modo raw e garante que as configurações originais serão restauradas ao sair.
    desativarBufferDeEntrada();
    atexit(restaurarTerminal);

    // Carrega as tarefas salvas (se existirem)
    carregar_tarefas();

    while (1) {
        mostrar_menu();
        int tecla = getchar();
        if (tecla == '\033') {  // Sequência de escape para teclas especiais
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
            break;
        }
    }
    // Antes de sair, salva as tarefas
    habilitarModoCanonical();  // Certifica que estamos no modo canônico para a escrita do arquivo
    salvar_tarefas();
    return 0;
}
