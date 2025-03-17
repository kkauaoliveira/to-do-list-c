#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define MAX_TAREFAS 100
#define TAMANHO_DESCRICAO 100

typedef struct {
    char descricao[TAMANHO_DESCRICAO];
    int status; // 0: A Fazer, 1: Fazendo, 2: Feito
    int prioridade; // 0: Emergente, 1: Urgente, 2: Importante
} Tarefa;

Tarefa tarefas[MAX_TAREFAS];
int quantidade_tarefas = 0;
int aba_selecionada = 0; // 0: A Fazer, 1: Fazendo, 2: Feito
int tarefa_selecionada = 0;

void desativarBufferDeEntrada() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);  // Desativa entrada canônica e eco
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void ativarBufferDeEntrada() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON | ECHO;  // Ativa entrada canônica e eco
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void limpar_tela() {
    #ifdef _WIN32
        system("cls"); // Para Windows
    #else
        system("clear"); // Para Linux/Mac
    #endif
}

void mostrar_tarefas() {
    limpar_tela();
    printf("\n--- Lista de Tarefas ---\n");
    printf("A Fazer:\n");
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == 0) {
            printf("%d: %s [Prioridade: %d]\n", i + 1, tarefas[i].descricao, tarefas[i].prioridade);
        }
    }
    printf("\nFazendo:\n");
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == 1) {
            printf("%d: %s [Prioridade: %d]\n", i + 1, tarefas[i].descricao, tarefas[i].prioridade);
        }
    }
    printf("\nFeito:\n");
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == 2) {
            printf("%d: %s [Prioridade: %d]\n", i + 1, tarefas[i].descricao, tarefas[i].prioridade);
        }
    }
    printf("\n");
}

void adicionar_tarefa() {
    if (quantidade_tarefas >= MAX_TAREFAS) {
        printf("Limite de tarefas atingido!\n");
        return;
    }

    char descricao[TAMANHO_DESCRICAO];
    int prioridade;

    printf("Digite a descrição da tarefa: ");
    getchar(); // Limpa o buffer do teclado
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = 0; // Remove a nova linha

    printf("Digite a prioridade (0: Emergente, 1: Urgente, 2: Importante): ");
    scanf("%d", &prioridade);

    tarefas[quantidade_tarefas].status = aba_selecionada;
    tarefas[quantidade_tarefas].prioridade = prioridade;
    strcpy(tarefas[quantidade_tarefas].descricao, descricao);
    quantidade_tarefas++;

    printf("Tarefa adicionada com sucesso!\n");
}

void editar_tarefa() {
    int numero_tarefa;
    printf("Digite o número da tarefa que deseja editar: ");
    scanf("%d", &numero_tarefa);

    if (numero_tarefa < 1 || numero_tarefa > quantidade_tarefas) {
        printf("Número de tarefa inválido!\n");
        return;
    }

    char descricao[TAMANHO_DESCRICAO];
    int prioridade;

    printf("Digite a nova descrição da tarefa: ");
    getchar(); // Limpa o buffer do teclado
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = 0; // Remove a nova linha

    printf("Digite a nova prioridade (0: Emergente, 1: Urgente, 2: Importante): ");
    scanf("%d", &prioridade);

    strcpy(tarefas[numero_tarefa - 1].descricao, descricao);
    tarefas[numero_tarefa - 1].prioridade = prioridade;
    printf("Tarefa editada com sucesso!\n");
}

void mudar_status() {
    int numero_tarefa, novo_status;
    printf("Digite o número da tarefa que deseja alterar o status: ");
    scanf("%d", &numero_tarefa);

    if (numero_tarefa < 1 || numero_tarefa > quantidade_tarefas) {
        printf("Número de tarefa inválido!\n");
        return;
    }

    printf("Digite o novo status (0: A Fazer, 1: Fazendo, 2: Feito): ");
    scanf("%d", &novo_status);

    if (novo_status < 0 || novo_status > 2) {
        printf("Status inválido!\n");
        return;
    }

    tarefas[numero_tarefa - 1].status = novo_status;
    printf("Status da tarefa alterado com sucesso!\n");
}

void excluir_tarefa() {
    int numero_tarefa;
    printf("Digite o número da tarefa que deseja excluir: ");
    scanf("%d", &numero_tarefa);

    if (numero_tarefa < 1 || numero_tarefa > quantidade_tarefas) {
        printf("Número de tarefa inválido!\n");
        return;
    }

    for (int i = numero_tarefa - 1; i < quantidade_tarefas - 1; i++) {
        tarefas[i] = tarefas[i + 1];
    }

    quantidade_tarefas--;
    printf("Tarefa excluída com sucesso!\n");
}

void mostrar_menu() {
    limpar_tela();
    printf("\n--- Menu ---\n");
    printf("Use as setas para navegar e Enter para selecionar.\n");
    printf("A Fazer | Fazendo | Feito\n");
    printf("+ Adicionar Tarefa\n");
    mostrar_tarefas();
}

int main() {
    int teclaPressionada;
    char k = 0;

    desativarBufferDeEntrada();
    
    while (1) {
        mostrar_menu();
        teclaPressionada = getchar();  // Lê o caractere

        if (teclaPressionada == '\e') {  // Se a tecla for uma sequência de escape
            getchar();  // Ignora o caractere '['
            
            k = getchar();
            if (k == 'A') {  // Seta para cima
                if (tarefa_selecionada > 0) tarefa_selecionada--;
            } else if (k == 'B') {  // Seta para baixo
                if (tarefa_selecionada < quantidade_tarefas - 1) tarefa_selecionada++;
            } else if (k == 'C') {  // Seta para a direita
                if (aba_selecionada < 2) aba_selecionada++;
            } else if (k == 'D') {  // Seta para a esquerda
                if (aba_selecionada > 0) aba_selecionada--;
            }
        } 
        else if (teclaPressionada == 10) {  // Enter
            if (tarefa_selecionada == quantidade_tarefas) {
                adicionar_tarefa();
            } else {
                // Aqui você pode adicionar a lógica para editar ou mudar o status da tarefa selecionada
                printf("Tarefa selecionada: %d\n", tarefa_selecionada + 1);
            }
        }
    }
    
    ativarBufferDeEntrada();
    return 0;
}