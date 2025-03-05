/*
	1) V�rios processos leitores podem acessar a base ao mesmo tempo
	2) Vari�vel compartilhada por todos processos leitores controla o n�mero de leitores na base - cont_leitores
	3) V�rios processos acessam cont_leitores � necessidade de exclus�o m�tua � sem�foro mutex
	3) Um �nico processo escritor pode escrever (modificar) a base de dados em um determinado instante
	4) Quando a base est� sendo modificada n�o pode haver processos leitores acessando a base
	6) Necess�rio controlar o acesso de leitores ou escrit�res ao banco de dados - sem�foro banco_de_dados
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define INTERACOES 5      // N�mero de intera��es
#define N_LEITORES 3      // N�mero de leitores
#define N_ESCRITORES 3    // N�mero de escritores

int cont_leitores = 0;   										 // N�mero de leitores ativos no banco de dados
sem_t banco_de_dados;     										 // Controla o acesso ao banco de dados. S� um escritor pode acessar o banco de dados por vez, e leitores n�o podem ler enquanto um escritor estiver escrevendo.
pthread_mutex_t cont_leitores_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para proteger cont_leitores
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; 		     // Mutex global para logs
pthread_cond_t escritor_cond = PTHREAD_COND_INITIALIZER; 		 // Condi��o para o escritor aguardar sua vez de acessar o banco de dados

// Funcao que descreve as etapas percorridas por um processo que deseja ler do banco de dados
void *leitor(void *id_processo) {
    int id = *(int*)id_processo;  // Converte o ponteiro de id para um inteiro que ser� utilizado nas demais opera��es
    
    int j;
    for (j = 0; j < INTERACOES; j++) {
    	printf("Leitor %d esta tentando acessar o contador de leitores.\n", id);
        fflush(stdout);
        pthread_mutex_lock(&cont_leitores_mutex);  // Protege cont_leitores com mutex
        
        cont_leitores++;   // Adiciona 1 leitor ao contador
        
        if (cont_leitores == 1) {
        	printf("Leitor %d eh o primeiro, bloqueando acesso de escrita no banco de dados.\n", id);
        	fflush(stdout);
            sem_wait(&banco_de_dados); // Primeiro leitor bloqueia escrita do banco para manter a integridade dos dados
        }
        
        pthread_mutex_unlock(&cont_leitores_mutex); // Libera o mutex de cont_leitores

        // Regi�o cr�tica: leitura do banco de dados
        printf("Leitor %d esta lendo o banco de dados...\n", id);
        fflush(stdout);
        sleep(2);          // Simula tempo de leitura

        pthread_mutex_lock(&cont_leitores_mutex);  // Protege cont_leitores com mutex
        cont_leitores--;   // Reduz 1 leitor do contador
        
        if (cont_leitores == 0) {
            sem_post(&banco_de_dados); // �ltimo leitor libera escrita ao banco de dados para ser poss�vel escrever
            pthread_cond_broadcast(&escritor_cond); // Acorda escritores que estavam esperando "adormecidos"
        }
        
        pthread_mutex_unlock(&cont_leitores_mutex);  // Libera o mutex de cont_leitores

        pthread_mutex_lock(&log_mutex);  // Bloqueia o mutex de log antes de imprimir para impedir que outras mensagens sejam lan�adas de forma concorrente antes de finalizar
        printf("\n============================\n");
        printf("Leitor %d terminou de ler.", id);
	    printf("\n============================\n\n");
        fflush(stdout);
    	pthread_mutex_unlock(&log_mutex);  // Libera o mutex de log ap�s imprimir a finaliza��o da thread
    	
	    sleep(2);
    }
    pthread_exit(NULL);
}

// Funcao que descreve as etapas percorridas por um processo que deseja escrever no banco de dados
void *escritor(void *id_processo) {
    int id = *(int*)id_processo; // Converte o ponteiro de id para um inteiro que ser� utilizado nas demais opera��es
    
    int j;
    for (j = 0; j < INTERACOES; j++) {
        printf("Escritor %d esta pensando...\n", id);
        fflush(stdout);
        sleep(1); // Simula tempo de processamento

        printf("Escritor %d esta tentando adquirir acesso ao banco de dados para escrever.\n", id);
        fflush(stdout);
        
        pthread_mutex_lock(&cont_leitores_mutex); // Protege cont_leitores com mutex
        while (cont_leitores > 0) {
            printf("Escritor %d esta aguardando... (Leitores ativos)\n", id);
            pthread_cond_wait(&escritor_cond, &cont_leitores_mutex); // Espera at� que n�o haja leitores
        }
        
        if (cont_leitores == 0) {
        	sem_wait(&banco_de_dados); // Garante acesso exclusivo ao banco de dados

	        // Se��o cr�tica: escrita no banco de dados
	        sleep(2); // Simula tempo de escrita
	        printf("Escritor %d esta escrevendo no banco de dados...\n", id);
        	fflush(stdout);
	        sleep(2); // Simula tempo de escrita
	        
	        pthread_mutex_lock(&log_mutex);  // Bloqueia o mutex de log antes de imprimir para impedir que outras mensagens sejam lan�adas de forma concorrente antes de finalizar
	        printf("\n===============================\n");
	        printf("Escritor %d terminou de escrever", id);
	        printf("\n===============================\n\n");
        	fflush(stdout);
	    	pthread_mutex_unlock(&log_mutex);  // Libera o mutex de log ap�s imprimir a finaliza��o da thread
	
	        sem_post(&banco_de_dados); // Libera o banco de dados
	        
	        pthread_mutex_unlock(&cont_leitores_mutex); // Libera o mutex de cont_leitores
	
	        sleep(2);
		}
        
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t leitores[N_LEITORES], escritores[N_ESCRITORES];
    int id_leitores[N_LEITORES], id_escritores[N_ESCRITORES];

    // Inicializa o sem�foro para controle do banco de dados
    sem_init(&banco_de_dados, 0, 1);

    // Cria leitores
    int i;
    for (i = 0; i < N_LEITORES; i++) {
        id_leitores[i] = i + 1;
        pthread_create(&leitores[i], NULL, leitor, &id_leitores[i]);
    }

    // Cria escritores
    for (i = 0; i < N_ESCRITORES; i++) {
        id_escritores[i] = i + 1;
        pthread_create(&escritores[i], NULL, escritor, &id_escritores[i]);
    }

    // Aguarda as threads terminarem
    for (i = 0; i < N_LEITORES; i++) {
        pthread_join(leitores[i], NULL);
    }
    for (i = 0; i < N_ESCRITORES; i++) {
        pthread_join(escritores[i], NULL);
    }

    // Destroi os sem�foros
    sem_destroy(&banco_de_dados);

    printf("Simulacao finalizada.\n");
    return 0;
}

