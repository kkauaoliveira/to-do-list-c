#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TAREFAS 100
#define TAMANHO_DESCRICAO 100

typedef struct {
    char descricao[TAMANHO_DESCRICAO];
    int status; // 0: A Fazer, 1: Fazendo, 2: Feito
} Tarefa;

Tarefa tarefas[MAX_TAREFAS];
int quantidade_tarefas = 0;

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
            printf("%d: %s\n", i + 1, tarefas[i].descricao);
        }
    }
    printf("\nFazendo:\n");
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == 1) {
            printf("%d: %s\n", i + 1, tarefas[i].descricao);
        }
    }
    printf("\nFeito:\n");
    for (int i = 0; i < quantidade_tarefas; i++) {
        if (tarefas[i].status == 2) {
            printf("%d: %s\n", i + 1, tarefas[i].descricao);
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
    printf("Digite a descrição da tarefa: ");
    getchar(); // Limpa o buffer do teclado
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = 0; // Remove a nova linha

    tarefas[quantidade_tarefas].status = 0; // A Fazer
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
    printf("Digite a nova descrição da tarefa: ");
    getchar(); // Limpa o buffer do teclado
    fgets(descricao, TAMANHO_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = 0; // Remove a nova linha

    strcpy(tarefas[numero_tarefa - 1].descricao, descricao);
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

int main() {
    int opcao;

    while (1) {
        limpar_tela();
        printf("\n--- Menu ---\n");
        printf("1. Visualizar tarefas\n");
        printf("2. Adicionar tarefa\n");
        printf("3. Editar tarefa\n");
        printf("4. Mudar status da tarefa\n");
        printf("5. Excluir tarefa\n");
        printf("6. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                mostrar_tarefas();
                break;
            case 2:
                adicionar_tarefa();
                break;
            case 3:
                editar_tarefa();
                break;
            case 4:
                mudar_status();
                break;
            case 5:
                excluir_tarefa();
                break;
            case 6:
                printf("Saindo...\n");
                exit(0);
            default:
                printf("Opção inválida! Tente novamente.\n");
        }

        // Pausa para o usuário visualizar a mensagem antes de limpar a tela
        printf("\nPressione Enter para continuar...");
        getchar(); // Limpa o buffer do teclado
        getchar(); // Aguarda o Enter
    }

    return 0;
}