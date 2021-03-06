#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

int Resource,Proc;
int *res;
int **allo;
int **maxReq;
int **need;
int *safeSeq;
int nProcRan = 0;

pthread_mutex_t lockRes;
pthread_cond_t cond;
bool getSafeSeq();
void* processCode(void* );

int main(int argc, char** argv) {
	int i,j;
	srand(time(NULL));

        printf("\nNumber of processes? ");
        scanf("%d", &Proc);

        printf("\nNumber of resources? ");
        scanf("%d", &Resource);

        res = (int *)malloc(Resource * sizeof(*res));
        printf("\nCurrently Available resources (R1 R2 ...)? ");
        for(i=0; i<Resource; i++)
                scanf("%d", &res[i]);

        allo = (int **)malloc(Proc * sizeof(*allo));
        for( i=0; i<Proc; i++)
                allo[i] = (int )malloc(Resource * sizeof(*allo));

        maxReq = (int **)malloc(Proc * sizeof(*maxReq));
        for(i=0; i<Proc; i++)
                maxReq[i] = (int )malloc(Resource * sizeof(*maxReq));
        printf("\n");
        for(i=0; i<Proc; i++) {
                printf("\nResource allocated to process %d (R1 R2 ...)? ", i+1);
                for(j=0; j<Resource; j++)
                        scanf("%d", &allo[i][j]);
        }
        printf("\n");
        for(i=0; i<Proc; i++) {
                printf("\nMaximum resource required by process %d (R1 R2 ...)? ", i+1);
                for(j=0; j<Resource; j++)
                        scanf("%d", &maxReq[i][j]);
        }
        printf("\n");
        need = (int **)malloc(Proc * sizeof(*need));
        for(i=0; i<Proc; i++)
                need[i] = (int )malloc(Resource * sizeof(*need));

        for(i=0; i<Proc; i++)
                for(j=0; j<Resource; j++)
                        need[i][j] = maxReq[i][j] - allo[i][j];
	safeSeq = (int *)malloc(Proc * sizeof(*safeSeq));
        for(i=0; i<Proc; i++) safeSeq[i] = -1;

        if(!getSafeSeq()) {
                printf("\nUnsafe State! The processes leads the system to a unsafe state.\n\n");
                exit(-1);
        }

        printf("\n\nSafe Sequence Found : ");
        for(i=0; i<Proc; i++) {
                printf("%-3d", safeSeq[i]+1);
        }

        printf("\nProcesses Exexcuting...\n\n");
        sleep(1);
	pthread_t processes[Proc];
        pthread_attr_t attr;
        pthread_attr_init(&attr);

	int processNumber[Proc];
	for(i=0; i<Proc; i++) processNumber[i] = i;

        for(i=0; i<Proc; i++)
                pthread_create(&processes[i], &attr, processCode, (void *)(&processNumber[i]));

        for(i=0; i<Proc; i++)
                pthread_join(processes[i], NULL);

        printf("\nAll Finished Processes\n");	
        free(res);
        for(i=0; i<Proc; i++) {
                free(allo[i]);
                free(maxReq[i]);
		free(need[i]);
        }
        free(allo);
        free(maxReq);
	free(need);
        free(safeSeq);
}


bool getSafeSeq() {
		int i,k,j;
        int tempRes[Resource];
        for(i=0; i<Resource ;i++) tempRes[i] = res[i];

        bool finished[Proc];
        for(i=0; i<Proc; i++) finished[i] = false;
        int nfinished=0;
        while(nfinished < Proc) {
                bool safe = false;

                for(i=0; i<Proc; i++) {
                        if(!finished[i]) {
                                bool possible = true;

                                for(j=0; j<Resource; j++)
                                        if(need[i][j] > tempRes[j]) {
                                                possible = false;
                                                break;
                                        }

                                if(possible) {
                                        for(j=0; j<Resource; j++)
                                                tempRes[j] += allo[i][j];
                                        safeSeq[nfinished] = i;
                                        finished[i] = true;
                                        ++nfinished;
                                        safe = true;
                                }
                        }
                }

                if(!safe) {
                        for(k=0; k<Proc; k++) safeSeq[k] = -1;
                        return false; 
                }
        }
        return true; 
}

void* processCode(void *arg) {
		int i;
        int p = *((int *) arg);
        pthread_mutex_lock(&lockRes);
        while(p != safeSeq[nProcRan])
                pthread_cond_wait(&cond, &lockRes);
        printf("\n--> Process %d", p+1);
        printf("\n\tAllocated : ");
        for(i=0; i<Resource; i++)
                printf("%3d", allo[p][i]);

        printf("\n\tNeeded    : ");
        for(i=0; i<Resource; i++)
                printf("%3d", need[p][i]);

        printf("\n\tAvailable : ");
        for(i=0; i<Resource; i++)
                printf("%3d", res[i]);

        printf("\n"); sleep(1);

        printf("\tAllocated resources!");
        printf("\n"); sleep(1);
        printf("\tProcess Code Running...");
        printf("\n"); sleep(rand()%3 + 2);
        printf("\tProcess Code Completed...");
        printf("\n"); sleep(1);
        printf("\tProcess Releasing Resource...");
        printf("\n"); sleep(1);
        printf("\tResource Released!");

	for(i=0; i<Resource; i++)
                res[i] += allo[p][i];

        printf("\n\tAvailable Resources: ");
        for(i=0; i<Resource; i++)
                printf("%3d", res[i]);
        printf("\n\n");

        sleep(1);
        nProcRan++;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lockRes);
	pthread_exit(NULL);
}
