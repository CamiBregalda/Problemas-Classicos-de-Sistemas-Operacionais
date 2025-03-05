/*
	1) Cinco filósofos estão sentados ao redor de uma mesa circular para o jantar.
	2) Cada filósofo possui um prato para comer macarrão.
	3) Os filósofos dispõem de garfos e é preciso de 2 deles para comer.
	4) Entre cada par de pratos existe apenas um garfo: Eles precisam ser compartilhados de forma sincronizada.
	5) Os filósofos comem e pensam, alternadamente. 
	6) Além disso, os garfos pegos devem ser aqueles logo a esquerda ou direita do prato do filosofo.
*/

#include <stdio.h>   // Funções de entrada/saída
#include <stdlib.h>  // Funções de alocação de memória
#include <pthread.h> // Manipulação de threads.
#include <unistd.h>  // Função sleep() para pausas.

#define QUANT 5                            // Quantidade de filósofos
#define ESQUERDO (i + QUANT - 1) % QUANT   // ID do filósofo à esquerda do filósofo atual da thread
#define DIREITO (i + 1) % QUANT            // ID do filósofo à direita do filósofo atual da thread
#define PENSANDO 0                         // Estado "pensando"
#define FAMINTO 1                          // Estado "com fome"
#define COMENDO 2                          // Estado "comendo"
#define INTERACOES 5                       // Número de interações que cada filósofo executará.

int estadoFilosofo[QUANT];                              // Armazena o estado atual de cada filósofo.
pthread_mutex_t mutex;                                  // Bloqueia o acesso à região crítica (Mutex para exclusão mútua - evita condições de corrida)
pthread_cond_t mutex_filosofos[QUANT];                  // Condição para cada filósofo
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; 	// Mutex global para logs


// Funcao que mostra o estado dos filosofos sem que outras threads interrompam sua exibição.
void exibir_estados() {
    pthread_mutex_lock(&log_mutex);  // Bloqueia o mutex de log antes de imprimir os estados dos filosofos para impedir que outras mensagens sejam lançadas de forma concorrente antes de finalizar

    printf("\n=== Estado Atual da Mesa ===\nFilosofos: ");
    int i;
    for (i = 0; i < QUANT; i++) {
        if (estadoFilosofo[i] == PENSANDO)
            printf("[P] ");
        else if (estadoFilosofo[i] == FAMINTO)
            printf("[F] ");
        else
            printf("[C] ");
    }
    printf("\n============================\n");

    pthread_mutex_unlock(&log_mutex);  // Libera o mutex de log após imprimir os estados de cada filosofo
}

// Cada filosofo pensa por 2 segundos antes da proxima acao
void pensar(int idFilosofo) {
    printf("\nFilosofo %d esta PENSANDO.\n", idFilosofo + 1);
    sleep(2);
}

// Se um filosofo conseguir os dois garfos, ele come por 2 segundos
void comer(int idFilosofo) {
    printf("\nFilosofo %d esta COMENDO.\n", idFilosofo + 1);
    sleep(2);
}

// O filosofo entrará no estado FAMINTO e tentará pegar 2 garfos, se não conseguir, ele entrará em modo de espera até conseguir comer
void pegar_garfo(int i) {
    pthread_mutex_lock(&mutex);             // Entra na região crítica
    estadoFilosofo[i] = FAMINTO;            // Entra no estado "com fome"
    
    printf("\n>>> Filosofo %d esta FAMINTO e tentando pegar os garfos.\n", i + 1);
    tentando_comer(i);                      // Tenta comer

    while (estadoFilosofo[i] != COMENDO) {  
        pthread_cond_wait(&mutex_filosofos[i], &mutex);  // Se não puder comer, aguarda até ser acordado
    }

    pthread_mutex_unlock(&mutex);           // Sai da região crítica
}

// O filosofo voltará no estado PENSANDO após comer e devolverá os garfos, após, ele checa se os filósofos a sua esquerda e a sua direita podem comer
void devolver_garfo(int i) {
    pthread_mutex_lock(&mutex);             // Entra na região crítica
    estadoFilosofo[i] = PENSANDO;           // Volta a pensar
    printf("\n<<< Filosofo %d DEVOLVEU os garfos e voltou a pensar.\n", i + 1);

    tentando_comer(ESQUERDO);               // Checa se o filósofo à esquerda pode comer
    tentando_comer(DIREITO);                // Checa se o filósofo à direita pode comer

    pthread_mutex_unlock(&mutex);           // Sai da região crítica
}


void tentando_comer(int i) {
	// Verifica se o filosofo está FAMINTO e se seus vizinhos não estão COMENDO. Caso seus vizinhos nao estejam comendo, os dois garfos estão liberados para serem pegos
    if (estadoFilosofo[i] == FAMINTO && estadoFilosofo[ESQUERDO] != COMENDO && estadoFilosofo[DIREITO] != COMENDO) {
        estadoFilosofo[i] = COMENDO;               // Se for possível, o filósofo começa a comer
        pthread_cond_signal(&mutex_filosofos[i]);  // Acorda o filósofo e permite que ele coma
    }
}

// A funcao representa o comportamento de um filósofo na simulação do Problema dos Filósofos, cada filósofo é uma thread separada e essa função é executada para cada um deles
void* filosofo(void* id) {
    int i = *(int*)id;      // Converte o ponteiro de id para um inteiro que será utilizado nas demais operações
    free(id);               // Libera a memória alocada para o id para garantir que os recursos alocados dinamicamente sejam liberados após o uso

	int j;
    for (j = 0; j < INTERACOES; j++) { // Repete o ciclo de vida do filósofo INTERACOES vezes
        pensar(i);         // O filósofo começa pensando
        
	    sleep(2);
	    exibir_estados();
	    sleep(2);
	    
        pegar_garfo(i);    // O filósofo tenta pegar os garfos
        comer(i);          // Se conseguir pegar os garfos na função anterior, ele come. Senão, ele se mantem "dormindo" através do mutex_filosofos
        
	    sleep(2);
	    exibir_estados();
	    sleep(2);
	    
        devolver_garfo(i); // O filósofo devolve os garfos e volta a pensar
    }

    return NULL;
}


// Funcao principal que cria as threads, inicia os mutexes e aguardar as threads terminarem.
int main() {
    pthread_t threads_filosofos[QUANT];  // Inicia um array de threads, sendo uma para cada filósofo
    int i;

    // Inicializa mutex de região crítica
    pthread_mutex_init(&mutex, NULL);
    
    // Inicia todos os filósofos com o estado PENSANDO antes de iniciar o código
    for (i = 0; i < QUANT; i++) {
        pthread_cond_init(&mutex_filosofos[i], NULL); // Inicializa uma variável de condicao para cada filosofo
        
        estadoFilosofo[i] = PENSANDO;
	    printf("Filosofo %d esta PENSANDO.\n", i + 1);
	    printf("----------------------------\n");
    }

    // Cria threads para cada filósofo
    for (i = 0; i < QUANT; i++) {
        int* id = malloc(sizeof(int));  // Aloca dinamicamente memória para armazenar o id do filósofo, evitando que ele seja sobrescrito antes que a thread seja criada.
        *id = i;
        pthread_create(&threads_filosofos[i], NULL, filosofo, id); // Inicia a nova thread passando o identificador da thread, seus atributos espeicias, a função a ser executada pela thread e o argumento da função da thread
    }

    // Aguarda todas as threads terminarem
    for (i = 0; i < QUANT; i++) {
        pthread_join(threads_filosofos[i], NULL); // Loop que agurda a finalizacao de todas as threads criadas, bloquando a execução da main até que a thread especificada termine sua execucao
    }

    // Destroi mutex e variáveis de condição para liberar os recursos que foram alocados para elas
    pthread_mutex_destroy(&mutex);
    for (i = 0; i < QUANT; i++) {
        pthread_cond_destroy(&mutex_filosofos[i]);
    }

    printf("Simulacao finalizada.\n");
    return 0;
}

