#include <gsl/gsl_rng.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#define MAXPOP 3
#define fileLocation "parameters.txt"

//variaveis globais
double etc[512][16];
double localmakespan[512];
int tasks;
int machines;
gsl_rng *r;
char path[1000]; //= "/Users/josejunio/Documents/Mestrado/Distribute_MultiObjective_Scheduling_C/u_c_hihi.0";
int popSize;
int maxGen;
int crossover_mask[512];

struct Solution{
  int Mapping[512];   // chromosome
  double Load[16];    // load of the machines
  double fit;         // fitness of the solution
  double makespan;
  double varload;
  double ct[512][16];
};

int max(int num1, int num2){
    return (num1 > num2 ) ? num1 : num2;
}

int min(int num1, int num2){
    return (num1 > num2 ) ? num2 : num1;
}

void initialize_params(){
    /*tasks = 512;
    machines = 16;
    popSize = atoi("300");
    maxGen = atoi("2000");
*/

    for(int i=0; i< tasks; i++){
        localmakespan[i] = 0.0;
    }

    FILE *fr;

    fr = fopen("/Users/josejunio/Desktop/Algoritmos/multiobjeticve-algorithm/parameters.txt", "r");
    //fr = fopen(fileLocation, "r");

    char tmpstr1[16];
    char tmpstr2[400];

    char tempbuff[500];

    while(!feof(fr)){
     if (fgets(tempbuff,500,fr)) {

        sscanf(tempbuff, "%s : %s", tmpstr1, tmpstr2);

        if (strcmp(tmpstr1,"fileName")==0) {
             strcpy(path, tmpstr2) ;
        } else if (strcmp(tmpstr1,"popSize")==0) {
             popSize = atoi(tmpstr2);
        } else if (strcmp(tmpstr1,"maxGen")==0) {
             maxGen = atoi(tmpstr2);
        } else if (strcmp(tmpstr1,"tasks")==0) {
             tasks = atoi(tmpstr2);
        } else if (strcmp(tmpstr1,"machines")==0) {
             machines = atoi(tmpstr2);
        }
     }
   }

    fclose(fr);
}

double makespan(double *load){
  int j;

  double max = -1;

  for (j = 0; j < machines; j++) {
    //printf("machine: %d\n", j);
    //printf("value: %lf\n", load[j]);
    if (load[j] > max) {
      max = load[j];
    }
  }

  //return -max;
  return max;
}

double diffload(double *load){
  int j;
  double min, max;

  min = (double) INT_MAX;
  max = -1;

  for (j = 0; j < machines; j++) {
    if (load[j] > max){
       max = load[j];
    }

    if (load[j] < min){
       min = load[j];
    }
  }

  return -(max - min);
}

void compute_load(struct Solution *l){
  int i, j;
  double load;

  for (j = 0; j < machines; j++){
    load = 0.0;
    for (i = 0; i < tasks; i++){
      if (l->Mapping[i] == j){
        load += etc[i][j];
      }
    }
    l->Load[j] = load;
  }
}

struct Solution *copySol(struct Solution *l1){
  int i;
  struct Solution *l;

  l = (struct Solution *)malloc(sizeof(struct Solution));

  for (i = 0; i < tasks; i++){
    l->Mapping[i] = l1->Mapping[i];
  }

  l->fit = l1->fit;
  l->makespan = l1->makespan;
  l->varload = diffload(l->Load);

  return l;
}

struct Solution *getMinMakespan(struct Solution *population[]){

    	struct Solution *min = population[0];

        for(int i=1; i<MAXPOP; i++){
            if(population[i] != NULL){
                //printf("%d\n", i);
                //printf("%f\n", population[i]->makespan);
                if(population[i]->makespan < min->makespan){
                    min = copySol(population[i]);
                }
            }
       }

       return min;
}

struct Solution *getMaxMakespan(struct Solution *population[]){

    	struct Solution *max = population[0];

        for(int i=1; i<MAXPOP; i++){
            if(population[i] != NULL){
                //printf("%d\n", i);
                //printf("%f\n", population[i]->makespan);
                if(population[i]->makespan > max->makespan){
                    max = copySol(population[i]);
                }
            }
       }

       return max;
}

void NeighborhoodChange(struct Solution *x, struct Solution *x1,  int k){

    if(x1->makespan < x->makespan){
            printf("#################### NeighborhoodChange - antes %d #############\n", k);
            printf("x: %f \n", x->makespan);
            printf("x1: %f \n", x1->makespan);
            x = copySol(x1);
            compute_load(x);
			 x->makespan = makespan(x->Load);

            printf("#################### NeighborhoodChange - depois %d #############\n", k);
            printf("x: %f \n", x->makespan);
            printf("x1: %f \n", x1->makespan);

        k=1;
    } else{
        k++;
    }
}

struct Solution *NeighborhoodChange_TESTE(struct Solution *x, struct Solution *x1,  int k){

    if(x1->makespan < x->makespan){
            printf("#################### NeighborhoodChange - antes %d #############\n", k);
            printf("x: %f \n", x->makespan);
            printf("x1: %f \n", x1->makespan);
            x = copySol(x1);
            compute_load(x);
			 x->makespan = makespan(x->Load);

            printf("#################### NeighborhoodChange - depois %d #############\n", k);
            printf("x: %f \n", x->makespan);
            printf("x1: %f \n", x1->makespan);
        k=1;
    } else{
        k++;
    }

    return x;
}

struct Solution *VND(struct Solution *population[], struct Solution *x, int kMax1){

    	int k = 1;

		 struct Solution *x2;

		do {
            x2 = getMinMakespan(population);
           // NeighborhoodChange(x, x2, k);
           x = NeighborhoodChange_TESTE(x, x2, k);
        } while(k == kMax1);

       // printf("x2: %f \n", x2->makespan);

		return x2;

}

struct Solution *Shake(struct Solution *x,  int k){

        struct Solution *x1 = copySol(x);

        int aux = 0;

       do {

           int firstPosition = gsl_rng_uniform_int(r, tasks);

			 int secondPosition = gsl_rng_uniform_int(r, tasks);

            int value = x1->Mapping[firstPosition];
            x1->Mapping[secondPosition] = x1->Mapping[firstPosition];
            x1->Mapping[secondPosition] = value;

			compute_load(x1);
			x1->makespan = makespan(x1->Load);

			aux++;

		} while(k == aux);

        return x1;
}

//Others heuritics
struct Solution *SwapMove(struct Solution *x){

        struct Solution *x1 = copySol(x);

         int firstPosition = gsl_rng_uniform_int(r, tasks);
         int secondPosition = gsl_rng_uniform_int(r, tasks);
         int thirdPosition = gsl_rng_uniform_int(r, tasks);

         x1->Mapping[firstPosition] = gsl_rng_uniform_int(r, machines);
         x1->Mapping[secondPosition] = gsl_rng_uniform_int(r, machines);
         x1->Mapping[thirdPosition] = gsl_rng_uniform_int(r, machines);

        compute_load(x1);
        x1->makespan = makespan(x1->Load);

        return x1;
}

struct Solution *InsertMove(struct Solution *x){

    struct Solution *x1 = copySol(x);

     int taskRandom = gsl_rng_uniform_int(r, tasks);
     int machineRandom = gsl_rng_uniform_int(r, machines);

     x1->Load[machineRandom] = etc[taskRandom][machineRandom];
     x1->Mapping[taskRandom] = machineRandom;

     compute_load(x1);
	 x1->makespan = makespan(x1->Load);

     return x1;

};

double calculateFitness(struct Solution *l){

    double busyTime[16];

    for(int i=0; i<machines; i++){
        busyTime[i] = 0;
    }

    for(int j=0; j<tasks; j++){
        int machine = l->Mapping[j];
        busyTime[machine] = busyTime[machine] + etc[j][machine];
    }

    double flowTime = 0;

    for(int i=0; i<machines; i++){
        flowTime += busyTime[i];
    }

    //double fitness = (0.75 * l->makespan) + (0.25*  (flowTime / machines));
    double fitness = l->makespan;

    //a × Makespan + (1 − a) × (Flowtime/m)

    return fitness;
}

struct Solution *BestInsertionMove(struct Solution *l, struct Solution *population[]){
    l->fit = calculateFitness(l);

    struct Solution *heavy = getMaxMakespan(population);

     double minValue = (double) INT_MAX;
     int machine = 0;
     int task = 0;

     for(int i=0; i<tasks; i++){
        for(int j=0; j<machines; j++){
           if(heavy->Mapping[i] == j && etc[i][j]  < minValue){
                minValue = etc[i][j];
                machine = j;
                task = j;
           }
        }
    }

    l->Mapping[task] = machine;
    l->Load[machine] = minValue;

    compute_load(l);
	 l->makespan = makespan(l->Load);

    return l;

}

struct Solution *MakespanInsertionMove(struct Solution *l, struct Solution *population[]){
    l->fit = calculateFitness(l);

    struct Solution *heavy = getMaxMakespan(population);

    struct Solution *light = getMinMakespan(population);

    int taskRandom = gsl_rng_uniform_int(r, tasks);

    int machine = heavy->Mapping[taskRandom];

    l->Mapping[taskRandom] = machine;
    l->Load[machine] = light->Load[machine];

    compute_load(l);
	 l->makespan = makespan(l->Load);

    return l;

};

struct Solution *WeightedmakespanInsertionMove(struct Solution *l){

      struct Solution *x1 = copySol(l);

      x1->fit = calculateFitness(x1);

      int LR, HR = 0;

      do{
            LR = gsl_rng_uniform_int(r, tasks);
            HR = gsl_rng_uniform_int(r, tasks);
      } while((localmakespan[LR] <= (0.25 * l->fit)) && (localmakespan[HR] >= (0.75 * l->fit)));

     int taskRandom = gsl_rng_uniform_int(r, tasks);

     x1->Load[x1->Mapping[taskRandom]] = localmakespan[LR];

     compute_load(x1);
	 x1->makespan = makespan(x1->Load);

    return x1;
};

void calculateLocalMakespan(struct Solution *l){
    for(int i=1; i<tasks; i++){
        localmakespan[l->Mapping[i]] = localmakespan[l->Mapping[i]] + etc[i][l->Mapping[i]];
    }
}

//Others heuritics

struct Solution *BestImprovement(struct Solution *x, struct Solution *population[]){

    struct Solution *x1 = copySol(x);

    do{
            x1 = copySol(x);

            x = getMinMakespan(population);

    } while(x1->makespan > x->makespan);

    return x;

}

void GVNS(struct Solution *population[], int kMax, int k1Max, int alpha){

    for(int i=1; i<alpha; i++){
            int k=1;

            do{
                //population[1] = Shake(population[0], k);
                population[1] = SwapMove(population[0]);
                //population[2] = VND(population, population[1] ,k1Max);
                population[1] = InsertMove(population[0]);
                population[2] = BestInsertionMove(population[1], population);
                population[2] = MakespanInsertionMove(population[1], population);
                //population[2] = WeightedmakespanInsertionMove(population[1]);

                //NeighborhoodChange(population[0], population[2], k);
                population[0] = NeighborhoodChange_TESTE(population[0], population[2], k);
            } while(k == kMax);

            printf("#################### %d #############\n", k);
            printf("population[0]: %f \n", population[0]->makespan);
            printf("population[1]: %f \n", population[1]->makespan);
            printf("population[2]: %f \n", population[2]->makespan);

    }

}

void BVNS(struct Solution *population[], int kMax, int beta){

      for(int i=1; i<beta; i++){
            int k=1;

            do{
                population[1] = Shake(population[0], k);
                population[2] = BestImprovement(population[1], population);
                population[0] = NeighborhoodChange_TESTE(population[0], population[2], k);
            } while(k == kMax);

            printf("#################### %d #############\n", k);
            printf("population[0]: %f \n", population[0]->makespan);
            printf("population[1]: %f \n", population[1]->makespan);
            printf("population[2]: %f \n", population[2]->makespan);
    }

}

struct Solution *min_min(){

    struct Solution *l;

    l = (struct Solution *)malloc(sizeof(struct Solution));

    for(int i=0; i<tasks; i++){
        for(int j=0; j<machines; j++){
            l->ct[i][j] = etc[i][j];
            //fprintf(stdout, "%d\t %d\t %lf\n", i, j, l->ct[i][j]);
        }
    }

    int numTasks = tasks;

    bool isRemoved[tasks];

    //inicializa com false
    for(int i=0; i<tasks; i++){
        isRemoved[i] = false;
    }

    do{

        double minValue = (double) INT_MAX;
        int machine=-1;
        int task=-1;

        for(int i=0; i<tasks; i++){

            if(isRemoved[i]){
             continue;
            }

            for(int j=0; j<machines; j++){
                if(l->ct[i][j] < minValue){
                    minValue = l->ct[i][j];
                    machine = j;
                    task = i;
                }
            }
        }

        l->Load[machine] = minValue;
        l->Mapping[task] = machine;
        isRemoved[task] = true;

        for(int i=0; i<tasks; i++){
            if(isRemoved[i]){
                continue;
            }

            l->ct[i][machine] = etc[i][machine] + l->Load[machine];
        }

       numTasks--;
    } while(numTasks > 0);


    l->makespan = makespan(l->Load);
    l->varload = diffload(l->Load);
    l->fit = l->makespan;

    return l;
};

struct Solution *RandomMap(){
  struct Solution *l;
  int i;

  l = (struct Solution *)malloc(sizeof(struct Solution));

  for (i = 0; i < tasks; i++){
    l->Mapping[i] = gsl_rng_uniform_int(r, machines);
  }

  compute_load(l);
  l->makespan = makespan(l->Load);
  l->varload = diffload(l->Load);
  l->fit = l->makespan;

  return l;
}

struct Solution *ForcaBruta(){
      struct Solution *l;

      //int v[512] = {6,0,0,0,0,0,0,3,1,0,0,0,8,6,6,0,0,1,0,5,0,11,7,3,0,5,0,0,10,0,0,0,0,1,4,0,6,5,7,13,0,1,8,1,7,0,4,1,1,0,12,0,1,0,0,2,4,3,15,3,3,5,6,8,0,0,0,0,7,0,1,0,5,3,1,0,0,0,2,0,0,15,8,0,0,0,5,0,4,11,0,0,0,0,3,0,0,1,4,0,2,0,0,1,1,0,14,0,0,0,0,0,0,3,1,0,0,7,2,9,3,1,0,0,0,6,0,1,2,1,0,8,1,14,3,0,0,0,6,2,14,4,11,0,5,3,0,0,8,10,1,2,0,0,14,0,2,0,1,2,15,0,9,1,1,0,2,10,0,0,5,2,3,1,1,1,0,3,13,8,6,1,0,0,4,0,0,1,1,0,5,7,0,2,2,3,0,3,0,2,10,0,0,0,2,1,0,13,2,1,0,2,2,0,0,1,0,4,10,0,0,2,1,0,0,1,5,0,11,9,3,0,4,0,0,0,10,0,0,7,1,0,0,0,9,5,0,0,4,0,11,2,4,10,0,1,0,0,1,2,0,6,0,1,1,0,1,3,7,3,0,0,1,1,3,0,0,0,1,0,1,0,0,0,14,0,2,12,0,9,2,13,1,0,0,1,0,1,0,6,0,0,0,0,0,1,1,0,14,11,6,0,0,1,2,0,0,0,1,0,0,0,0,1,8,1,0,13,0,0,0,0,0,0,0,10,12,1,0,1,1,0,5,1,0,5,2,0,5,0,4,0,1,0,0,0,0,4,3,1,0,2,4,2,0,0,4,1,5,1,2,4,1,0,0,8,1,2,1,1,0,0,0,3,0,2,0,0,1,0,2,13,2,0,0,2,0,0,8,3,0,8,0,0,1,1,0,3,11,0,0,2,9,4,7,0,2,3,5,2,9,0,10,2,0,0,0,1,0,0,0,2,0,1,13,1,6,4,1,2,0,0,1,0,0,9,0,0,12,0,3,5,0,10,0,1,0,3,13,1,0,0,4,1,4,0,0,3,0,0,0,2,0,0,2,1,1,2,0,1,0,3,3,0,2,14,1,0,3,0,6,2,1,3,3,0,1,0,4,0,8,0,0,6,12,0,0,0,0,1,9,4};
      int v[512] = {11,0,0,0,0,0,0,3,1,0,0,0,8,6,6,0,0,1,0,5,0,12,7,3,0,5,0,0,10,0,0,0,0,1,5,0,6,5,7,13,0,1,8,1,7,0,4,1,1,0,12,0,1,0,0,2,4,3,15,3,9,5,6,8,0,0,0,0,7,0,1,0,5,3,1,0,0,0,2,0,0,15,8,0,0,0,5,0,4,11,0,0,0,0,3,0,0,1,4,0,2,0,0,1,1,0,14,0,0,0,0,0,0,3,1,0,0,7,2,9,3,1,0,0,0,6,0,1,7,1,0,8,1,14,3,0,0,0,6,2,15,4,11,2,5,3,0,0,8,10,1,2,0,0,14,0,12,0,1,2,15,0,9,1,1,0,2,10,0,0,5,2,3,1,1,1,0,3,13,8,6,1,0,0,4,0,0,1,1,0,5,7,0,2,2,3,0,3,0,2,10,0,0,0,2,1,0,13,2,1,0,2,2,0,0,1,0,4,10,0,0,2,1,0,0,1,5,0,11,9,3,0,4,0,0,0,10,0,0,7,1,0,0,0,9,5,0,0,4,0,11,2,4,10,0,1,0,0,1,2,0,6,0,1,1,0,1,3,7,3,0,0,1,1,3,0,0,0,1,0,1,0,0,0,14,0,2,12,0,9,2,13,1,0,0,1,0,1,0,6,0,0,0,0,0,1,1,0,14,11,6,0,0,1,2,0,0,0,1,0,0,0,0,1,8,1,0,13,0,0,0,0,0,0,0,10,12,1,0,1,1,0,8,1,0,5,2,0,5,0,4,0,1,0,0,0,0,4,3,1,0,2,4,2,0,0,4,1,5,1,2,4,1,0,0,8,1,2,1,1,0,0,0,3,0,2,0,0,1,0,2,13,2,0,0,2,0,0,8,3,0,8,0,0,1,1,0,3,11,0,0,2,9,4,7,0,2,3,5,2,9,0,10,2,0,0,0,1,0,0,0,2,0,1,13,1,6,4,1,2,0,0,1,0,0,9,0,0,12,0,3,5,0,10,0,1,0,3,13,1,0,0,4,1,4,0,0,3,0,0,0,2,0,0,2,1,1,2,0,1,0,3,3,0,2,14,1,0,3,0,6,2,1,3,3,0,1,0,4,0,8,0,0,6,12,0,0,0,0,1,9,4};

      l = (struct Solution *) malloc(sizeof(struct Solution));

      int i=0;

      for(i=0; i<512; i++){
        l->Mapping[i] = v[i];
      }

      compute_load(l);

      l->makespan = makespan(l->Load);

      return l;

};

//Melhor individuo que o minMin ( N‹o esquecer de - 1)
/*

   7  1  1  1  1  1  1  4  2  1  1  1  9  7  7  1  1  2  1  6  1 12  8  4  1  6  1  1 11  1  1  1  1  2  5  1  7  6  8 14  1  2  9  2  8 15  2  2  1 13  1  2  1  1  3  5  4 16  4  4  6  7  9  1  1  1  1  8  1  2  1  6  4  2  1  1  1  3  1  1 16  9  1  1  1  6  1  5 12  1  1  1  1  4  1  1  2  5  1  3  1  1  2  2  1 15  1  1  1  1  1  1  4  2  1  1  8  3 10  4  2  1  1  1  7  1  2  3  2  1  9  2 15  4  1  1  1  7  3 15  5 12  1  6  4  1  1  9 11  2  3  1  1 15  1  3  1  2  3 16  1 10  2  2  1  3

*/

int main(int argc, char *argv[]){

    initialize_params();

    static struct Solution *PopA[MAXPOP];
    static struct Solution *PopB[MAXPOP];
    static struct Solutions *temp;

    if((r = gsl_rng_alloc(gsl_rng_mt19937)) == NULL) {
        fprintf(stderr, "ERROR: Could not create random number generator\n");
        return EXIT_FAILURE;
    }

    gsl_rng_set(r, time(NULL));

    FILE *fp;

    fp = fopen(path, "r");

    if (fp == NULL){
        perror(path);
    }

    int a, b;

    for (a = 0; a < tasks; a++) {
        for (b = 0; b < machines; b++){
          fscanf(fp, "%lf", &etc[a][b]);
          //fprintf(stdout, "%d\t %d\t %lf\n", a, b, etc[a][b]);
        }
    }

    fclose(fp);

    PopA[0] = ForcaBruta();

    int alpha = 1800;
    int beta = 1600;
    int kMax = 90;
    int k1Max = 100;

    //calculateLocalMakespan(PopA[0]);

    printf("######################## GVNS - inicio ########################\n");
    GVNS(PopA, kMax, k1Max, alpha);
    printf("######################## GVNS - fim ########################\n");

    printf("######################## BVNS - inicio ########################\n");
    BVNS(PopA, kMax, beta);
    printf("######################## BVNS - fim ########################\n");

    /*int i = 0;

    for(i=0; i<tasks; i++){
        printf("%d, ", PopA[0]->Mapping[i]);
    }

    printf("################################################\n");

    for(i=0; i<tasks; i++){
        printf("%d, ",  PopA[1]->Mapping[i]);
    }

    printf("################################################\n");

    for(i=0; i<tasks; i++){
        printf("%d, ", PopA[2]->Mapping[i]);
    }*/

    return EXIT_SUCCESS;
}

