#include "predator.h"

//extern strategy_t choose_strategy(predator * p, strategy_t social);

extern generation * gens;

extern unsigned curr_gen;
extern double dimension;
extern float range;
extern unsigned social_choices[3];
extern float payoff_matrix[3];
extern float tot_fit;
double recombination_rate = 0.1;
double mutation_rate = 0.1;
extern unsigned rounds;

/* ------ phenotype expression ------ */
unsigned phenotype_size = 5; 	/* determines how many bases affect the strategy of each predator */
float com_rate = 0.65;	 			/* determines below which threshold does the strategy become competitive */
float syn_rate = 0.35;	 			/* determines below which threshold does the strategy become synergetic */
unsigned migration_size = 5;	/* determines how many bases affect the migration pattern of each predator */
/* ---------------------------------- */

char const_size = 0; 					/* determines whether the number of predators remains constant in each generation */
unsigned pred_num = 10;				/* initial number of predators ~ Default 10 unless user defined */
unsigned pred_pid = 0;
unsigned nsyn = 0;						/* number of user-defined synergetic predators */
unsigned nign = 0;						/* number of user-defined ignoring predators */
unsigned ncom = 0;						/* nubmer of user-defined competitive predators */

double not_play = 0;					/* default fitness for predators prior to their partipation to any game */
unsigned unable;	    				/* determines the number of predators that did not play a game ~ starting value is the total number of predators for each generation */

unsigned * People; 						/* number of predators in each generation */
next_gen * chosen = NULL; 		/* list of predators that are going to reproduce from each subgame */

/* ram reduction */
unsigned cram = -1; 					/* marks the generation for which the tree is pruned */
ram * ram_h = NULL;

double mean(double x, double y){
	return (x + y)/(float)2;
}

void print_phenotype(predator * p){
	assert(p != NULL);
	assert(p -> pheno != NULL);
	FILE * f1 = fopen("phenotype.txt", "a");
	unsigned i;
	if (curr_gen)
		fprintf(f1,"Gen %u, pred: %u w/ p1: %u and p2 %u:\n", curr_gen+1, p->prid, p->parent1-> prid, p->parent2->prid);
	for (i = 0; i < phenotype_size; i++)
		fprintf(f1, "%c", p->pheno[i]);
	fprintf(f1, "\n");
	fclose(f1);
}

predator * recombine(predator * p){
	unsigned cutting_point = rand() % phenotype_size; /* determine where the recombination happened */
	p -> pheno = (char*)calloc(sizeof(char), phenotype_size);
	if (rand() / (double)RAND_MAX < 0.5){ /* parent1 gives the left part */
		memcpy(p -> pheno, p -> parent1 -> pheno, (cutting_point+1) * sizeof(char));
		if (cutting_point != phenotype_size-1)
			memcpy(p -> pheno, p -> parent2 -> pheno + (cutting_point+1)*sizeof(char),
		 (phenotype_size - cutting_point - 2) * sizeof(char));
	}
	else{
		memcpy(p -> pheno, p -> parent2 -> pheno, (cutting_point+1) * sizeof(char));
		if (cutting_point != phenotype_size-1)
			memcpy(p -> pheno, p -> parent1 -> pheno + (cutting_point+1)*sizeof(char),
			 (phenotype_size - cutting_point-2) * sizeof(char));
	}
	return p;
}

predator * set_pheno(predator * p){
	/* we first need to see whether a recombination event takes place */
	double random = ((float)rand()/(float)(RAND_MAX));
	if (random < recombination_rate)
		return recombine(p);
	p -> pheno = (char*)malloc(sizeof(char) * phenotype_size);
	random = ((float)rand()/(float)(RAND_MAX));
	if (random < 0.5)
		memcpy(p -> pheno, p->parent1->pheno, phenotype_size*sizeof(char));
	else
		memcpy(p -> pheno, p->parent2->pheno, phenotype_size*sizeof(char));

	/* check for mutations */
	random = ((float)rand()/(float)(RAND_MAX));
	if (random < mutation_rate){
		//fprintf(stderr,"mutation\n");
		unsigned i = rand()%phenotype_size;
		p -> pheno[i] = !p->pheno[i];
	}
	return p;
}

predator * set_position(predator * p){
	p -> xaxis = dimension * ((double)rand()/RAND_MAX);//mean(p -> parent1 -> xaxis, p -> parent2 -> xaxis);
	p -> yaxis = dimension * ((double)rand()/RAND_MAX);//mean(p -> parent1 -> yaxis, p -> parent2 -> yaxis);
	return p;
}

unsigned find_parent(next_gen * ng, double index, unsigned min, unsigned max){
	if (max == min) /* single element in the array, thus the one we are looking for */
		return max;
	unsigned mid = (max- min) / 2; /* find the middle of the array */
	if (max - min < 3){
		if (ng[min].fitness >= index)
			return max;
		return min;
	}
	if (ng[mid-1].fitness <= index
		 && ng[mid+1].fitness >= index) /* found it */
		return mid;
	else if (ng[mid].fitness < index) /* search the right side of the map */
		return find_parent(ng, index, mid+1, max);
	else if (ng[mid].fitness > index) /* search the left side of the map */
		return find_parent(ng, index, min, mid-1);
	else
		assert(0);
	return 0;
}

predator * choose_parents(predator * p, next_gen * ng, unsigned num){
	/* after deciding the parents the rest fall into place */
	double p1 = tot_fit * (rand() / (double)RAND_MAX);
	double p2 = tot_fit * (rand() / (double)RAND_MAX);
	assert(ng != NULL);
	unsigned index = find_parent(ng, p1, 0, num-1);
	p -> parent1 = &gens[curr_gen].pred[index];
	index = find_parent(ng, p2, 0, num-1);
	p -> parent2 = &gens[curr_gen].pred[index];
	p = set_position(p);
	p = set_pheno(p);
}

/* mallon tha prepei na xei ta stoixeia tou pred ayto */
void add_predator(next_gen * ng, unsigned num){
	assert(ng != NULL);
	predator * p = calloc(1, sizeof(predator));
	p = choose_parents(p, ng, num);
	unsigned gen = curr_gen + 1;
	unsigned x = gens[gen].num;
	if (x == 0 && gens[gen].pred == NULL)
		gens[gen].pred = calloc(pred_num, sizeof(predator));

	gens[gen].pred[x].prid = ++pred_pid;
	gens[gen].pred[x].fitness = 0.0; /* is determined after the game */
	gens[gen].pred[x].pheno = malloc(phenotype_size*sizeof(char));
	memcpy(gens[gen].pred[x].pheno, p -> pheno, phenotype_size*sizeof(char));
	unsigned i;
	gens[gen].pred[x].strategy = choose_strategy(gens[gen].pred[x].pheno);
	gens[gen].pred[x].parent1 = p -> parent1;
	gens[gen].pred[x].parent1 = p -> parent2;
	gens[gen].pred[x].xaxis = p -> xaxis;
	gens[gen].pred[x].yaxis = p -> yaxis;
	gens[gen].pred[x].flag = 0; /* not yet reproduced */
	gens[gen].num++;
	free(p -> pheno);
	free(p);
}

/* ------------------------------- */

//not required if no rewind
void remove_predator(unsigned gen, unsigned index){ /* free predator & realloc the array */
	/* we don't care anymore about having a sorted array so no need to waste resources there */
	predator * tmp = &gens[gen].pred[index];
	gens[gen].pred[index] = gens[gen].pred[gens[gen].num -1];
	gens[gen].num--;
	gens[gen].pred = realloc(gens[gen].pred, sizeof(predator)*gens[gen].num);
	tmp -> parent1 = NULL;
	tmp -> parent2 = NULL;
	free(tmp -> pheno);
	free(tmp);
}

//not required if no rewind
unsigned darwinism(unsigned gen){
	unsigned length = gens[gen].num;
	unsigned i;
	for (i = 0; i < length; i++){
		if (!gens[gen].pred[i].flag){ /* did not leave a present day ancestor */
			gens[gen].pred[i].parent1 -> flag--;
			if (gens[gen].pred[i].parent1 -> prid != gens[gen].pred[i].parent2 -> prid) /* not single parented */
				gens[gen].pred[i].parent2 -> flag--;
			remove_predator(gen, i);
		}
	}
	return 0;
}

//not required if no rewind
void prune(){
	unsigned removed = 0;
	unsigned i;
	for (i = curr_gen -1; i <= 0; i++)
		removed += darwinism(i);
	fprintf(stderr,"Predators pruned from tree in generation %u: %u\n", curr_gen, removed);
}

void print_predators(unsigned gen){
	FILE * f1 = fopen("wirsinddiejaeger.txt", "a");
	unsigned j = 0;
	fprintf(f1,"gen: %u with %u predators\n", gen,gens[gen].num);
	for (j = 0; j < gens[gen].num; j++){
		fprintf(f1, "%u %u [%lf %lf]\n",gens[gen].pred[j].prid,
		 gens[gen].pred[j].strategy, gens[gen].pred[j].xaxis, gens[gen].pred[j].yaxis);
	}
	fprintf(f1, "\n");
	fclose(f1);
}

/* ------------------ */

void init_predator(){
	unsigned i =0;
	unsigned x = gens[curr_gen].num;
	if (gens[curr_gen].pred == NULL){ /* first entry in the current generation */
		gens[curr_gen].pred = malloc(pred_num * sizeof(predator));
		gens[curr_gen].num = 0;
		x = 0;
		return;
	}
	gens[curr_gen].pred[x].prid = ++pred_pid;
	gens[curr_gen].pred[x].fitness = 0.0; /* is determined after the game */
	gens[curr_gen].pred[x].parent1 = NULL;
	gens[curr_gen].pred[x].parent1 = NULL;
	gens[curr_gen].pred[x].flag = 0; /* not yet reproduced */
	gens[curr_gen].pred[x].xaxis = ((double)rand()/(double)RAND_MAX) * dimension;
	gens[curr_gen].pred[x].yaxis = ((double)rand()/(double)RAND_MAX) * dimension;
	gens[curr_gen].num++;

	/* now we need to initialize both the phenotype and the strategy */
		gens[curr_gen].pred[x].pheno = malloc(sizeof(char) * phenotype_size);
	if (nsyn > 0){ /* user-defined synergistic predators */
		gens[curr_gen].pred[x].strategy = synergy;
		nsyn--;
		return NULL;
	}
	if (nign > 0){
		gens[curr_gen].pred[x].strategy = ignore;
		nign--;
		return NULL;
	}
	if (ncom > 0){
		gens[curr_gen].pred[x].strategy = competition;
		ncom--;
		return NULL;
	}

	for (i = 0; i < phenotype_size; i++) //needs to be changed
		gens[curr_gen].pred[x].pheno[i] = '1';
	gens[curr_gen].pred[x].strategy = choose_strategy(gens[curr_gen].pred[x].pheno);

	print_phenotype(&gens[curr_gen].pred[0]);
	/* need to short the array */
}
