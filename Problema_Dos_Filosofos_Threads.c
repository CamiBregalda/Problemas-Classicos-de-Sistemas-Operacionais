/*
	1) Cinco fil�sofos est�o sentados ao redor de uma mesa circular para o jantar.
	2) Cada fil�sofo possui um prato para comer macarr�o.
	3) Os fil�sofos disp�em de garfos e � preciso de 2 deles para comer.
	4) Entre cada par de pratos existe apenas um garfo: Eles precisam ser compartilhados de forma sincronizada.
	5) Os fil�sofos comem e pensam, alternadamente. 
	6) Al�m disso, os garfos pegos devem ser aqueles logo a esquerda ou direita do prato do filosofo.
*/

#include <stdio.h>   // Fun��es de entrada/sa�da
#include <stdlib.h>  // Fun��es de aloca��o de mem�ria
#include <pthread.h> // Manipula��o de threads.
#include <unistd.h>  // Fun��o sleep() para pausas.

#define QUANT 5                            // Quantidade de fil�sofos
#define ESQUERDO (i + QUANT - 1) % QUANT   // ID do fil�sofo � esquerda do fil�sofo atual da thread
#define DIREITO (i + 1) % QUANT            // ID do fil�sofo � direita do fil�sofo atual da thread
#define PENSANDO 0                         // Estado "pensando"
#define FAMINTO 1                          // Estado "com fome"
#define COMENDO 2                          // Estado "comendo"
#define INTERACOES 5                       // N�mero de intera��es que cada fil�sofo executar�.

int estadoFilosofo[QUANT];                              // Armazena o estado atual de cada fil�sofo.
pthread_mutex_t mutex;                                  // Bloqueia o acesso � regi�o cr�tica (Mutex para exclus�o m�tua - evita condi��es de corrida)
pthread_cond_t mutex_filosofos[QUANT];                  // Condi��o para cada fil�sofo
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; 	// Mutex global para logs


// Funcao que mostra o estado dos filosofos sem que outras threads interrompam sua exibi��o.
void exibir_estados() {
    pthread_mutex_lock(&log_mutex);  // Bloqueia o mutex de log antes de imprimir os estados dos filosofos para impedir que outras mensagens sejam lan�adas de forma concorrente antes de finalizar

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

    pthread_mutex_unlock(&log_mutex);  // Libera o mutex de log ap�s imprimir os estados de cada filosofo
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

// O filosofo entrar� no estado FAMINTO e tentar� pegar 2 garfos, se n�o conseguir, ele entrar� em modo de espera at� conseguir comer
void pegar_garfo(int i) {
    pthread_mutex_lock(&mutex);             // Entra na regi�o cr�tica
    estadoFilosofo[i] = FAMINTO;            // Entra no estado "com fome"
    
    printf("\n>>> Filosofo %d esta FAMINTO e tentando pegar os garfos.\n", i + 1);
    tentando_comer(i);                      // Tenta comer

    while (estadoFilosofo[i] != COMENDO) {  
        pthread_cond_wait(&mutex_filosofos[i], &mutex);  // Se n�o puder comer, aguarda at� ser acordado
    }

    pthread_mutex_unlock(&mutex);           // Sai da regi�o cr�tica
}

// O filosofo voltar� no estado PENSANDO ap�s comer e devolver� os garfos, ap�s, ele checa se os fil�sofos a sua esquerda e a sua direita podem comer
void devolver_garfo(int i) {
    pthread_mutex_lock(&mutex);             // Entra na regi�o cr�tica
    estadoFilosofo[i] = PENSANDO;           // Volta a pensar
    printf("\n<<< Filosofo %d DEVOLVEU os garfos e voltou a pensar.\n", i + 1);

    tentando_comer(ESQUERDO);               // Checa se o fil�sofo � esquerda pode comer
    tentando_comer(DIREITO);                // Checa se o fil�sofo � direita pode comer

    pthread_mutex_unlock(&mutex);           // Sai da regi�o cr�tica
}


void tentando_comer(int i) {
	// Verifica se o filosofo est� FAMINTO e se seus vizinhos n�o est�o COMENDO. Caso seus vizinhos nao estejam comendo, os dois garfos est�o liberados para serem pegos
    if (estadoFilosofo[i] == FAMINTO && estadoFilosofo[ESQUERDO] != COMENDO && estadoFilosofo[DIREITO] != COMENDO) {
        estadoFilosofo[i] = COMENDO;               // Se for poss�vel, o fil�sofo come�a a comer
        pthread_cond_signal(&mutex_filosofos[i]);  // Acorda o fil�sofo e permite que ele coma
    }
}

// A funcao representa o comportamento de um fil�sofo na simula��o do Problema dos Fil�sofos, cada fil�sofo � uma thread separada e essa fun��o � executada para cada um deles
void* filosofo(void* id) {
    int i = *(int*)id;      // Converte o ponteiro de id para um inteiro que ser� utilizado nas demais opera��es
    free(id);               // Libera a mem�ria alocada para o id para garantir que os recursos alocados dinamicamente sejam liberados ap�s o uso

	int j;
    for (j = 0; j < INTERACOES; j++) { // Repete o ciclo de vida do fil�sofo INTERACOES vezes
        pensar(i);         // O fil�sofo come�a pensando
        
	    sleep(2);
	    exibir_estados();
	    sleep(2);
	    
        pegar_garfo(i);    // O fil�sofo tenta pegar os garfos
        comer(i);          // Se conseguir pegar os garfos na fun��o anterior, ele come. Sen�o, ele se mantem "dormindo" atrav�s do mutex_filosofos
        
	    sleep(2);
	    exibir_estados();
	    sleep(2);
	    
        devolver_garfo(i); // O fil�sofo devolve os garfos e volta a pensar
    }

    return NULL;
}


// Funcao principal que cria as threads, inicia os mutexes e aguardar as threads terminarem.
int main() {
    pthread_t threads_filosofos[QUANT];  // Inicia um array de threads, sendo uma para cada fil�sofo
    int i;

    // Inicializa mutex de regi�o cr�tica
    pthread_mutex_init(&mutex, NULL);
    
    // Inicia todos os fil�sofos com o estado PENSANDO antes de iniciar o c�digo
    for (i = 0; i < QUANT; i++) {
        pthread_cond_init(&mutex_filosofos[i], NULL); // Inicializa uma vari�vel de condicao para cada filosofo
        
        estadoFilosofo[i] = PENSANDO;
	    printf("Filosofo %d esta PENSANDO.\n", i + 1);
	    printf("----------------------------\n");
    }

    // Cria threads para cada fil�sofo
    for (i = 0; i < QUANT; i++) {
        int* id = malloc(sizeof(int));  // Aloca dinamicamente mem�ria para armazenar o id do fil�sofo, evitando que ele seja sobrescrito antes que a thread seja criada.
        *id = i;
        pthread_create(&threads_filosofos[i], NULL, filosofo, id); // Inicia a nova thread passando o identificador da thread, seus atributos espeicias, a fun��o a ser executada pela thread e o argumento da fun��o da thread
    }

    // Aguarda todas as threads terminarem
    for (i = 0; i < QUANT; i++) {
        pthread_join(threads_filosofos[i], NULL); // Loop que agurda a finalizacao de todas as threads criadas, bloquando a execu��o da main at� que a thread especificada termine sua execucao
    }

    // Destroi mutex e vari�veis de condi��o para liberar os recursos que foram alocados para elas
    pthread_mutex_destroy(&mutex);
    for (i = 0; i < QUANT; i++) {
        pthread_cond_destroy(&mutex_filosofos[i]);
    }

    printf("Simulacao finalizada.\n");
    return 0;
}

