/* In this file every function related to the preys of the game is implemented */
#include "prey.h"

unsigned prey_num = 10;
unsigned user_prey = 0; /* number of preys for which the position is user defined */
unsigned num_preys = 0;
unsigned playing_preys = 0;

unsigned max_penalty = 7;
unsigned min_penalty = 3;
double deplete = 0.8;	/* the max % of the prey that gets depleted from the predators */

float tot_fit = 0.0; 		/* total fitness/payoff among all players of a game */

float range = 15.0;

extern unsigned pred_num;
extern double dimension;
extern unsigned curr_gen; // probably not needed
extern unsigned curr_flag;
extern unsigned social_choices[3];
extern generation * gens;

prey_pos * posh = NULL;
prey * prey_array = NULL;
trigger * triggerlist;	/* list of every addition of prey that should occur ~ obviously in ascending order */

void add_next(prey * p, unsigned index, strategy_t strat){
	if (p -> num == 0) /* first predator in this game */
		p -> pred_index = malloc(gens[!curr_flag].num * sizeof(unsigned));
	p -> pred_index[p -> num] = index;
	social_choices[strat - 1]++; /* we have the -1 since undifined = 0 in the enumeration */
	p -> num++;
	//print_next(p);
}

void find_in_range(prey * p, unsigned gen){
	unsigned i;
	for (i = 0; i < gens[gen].num; i++){
		if (abs(p -> xaxis -  gens[gen].pred[i].xaxis) <= range
		 && abs(p -> yaxis -  gens[gen].pred[i].yaxis) <= range
		 && gens[gen].pred[i].fitness == 0){ /* in range and not in another game */
			add_next(p, i, gens[gen].pred[i].strategy);
		}
	}
	return;
}

/* ---------------------------------------------------- */

void init_preys(){
	prey_array = malloc(prey_num * sizeof(prey));
	unsigned i;
	prey_pos * tmp = NULL;
	for (i = 0; i < prey_num; i++){
		prey_array[i].id = num_preys++;	/* we  increase the num of preys in the game by one */
		prey_array[i].value = 1;//(rand() % 50 ) + 50;
		prey_array[i].num = 0; /* number of predators playing the game ~ starts with zero */
		if (user_prey > 0){
			prey_array[i].xaxis = posh -> x;
			prey_array[i].yaxis = posh -> y;
			tmp = posh;
			posh = posh -> next;
			free(tmp);
			user_prey--;
		}
		else{
			prey_array[i].xaxis = dimension * (rand() / (float)RAND_MAX);
			prey_array[i].yaxis = dimension * (rand() / (float)RAND_MAX);
		}
		prey_array[i].pred_index = NULL;
	}
	playing_preys+=prey_num;
}
/* ---------------------------------------------------- */

void prey_event_remove(float xaxis, float yaxis){
	fprintf(stderr, "remove\n");
	if (xaxis == -1.0){ /* event calls for a random prey removal */
		remove_prey(rand() % prey_num);
		return;
	}
	unsigned i;
	for (i = 0; i < prey_num; i++){
		if (prey_array[i].xaxis == xaxis && prey_array[i].yaxis == yaxis)
			remove_prey(i);
	}
}

void prey_event_add(float xaxis, float yaxis){
	fprintf(stderr, "add\n");
	prey_array = realloc(prey_array, (prey_num + 1) * sizeof(prey));
	prey_array[prey_num].id = prey_array[prey_num - 1].id + 1;
	prey_array[prey_num].value = 1;
	prey_array[prey_num].num = 0;
	if (xaxis != -1) /* non-random co-ordinates */
		prey_array[prey_num].xaxis = xaxis;
	else
		prey_array[prey_num].xaxis = dimension * (rand() / (float)RAND_MAX);
		if (yaxis != -1) /* non-random co-ordinates */
			prey_array[prey_num].yaxis = yaxis;
		else
			prey_array[prey_num].yaxis = dimension * (rand() / (float)RAND_MAX);
	prey_array[prey_num].pred_index = NULL;
	prey_num++;
}

/* -------------- deplete & remove start -------------- */

void remove_prey(unsigned pos){
	/* "swap" the to-be-removed prey with the last one */
	unsigned i = prey_num - 1;
	prey_array[pos].value = prey_array[i].value;
	prey_array[pos].xaxis = prey_array[i].xaxis;
	prey_array[pos].yaxis = prey_array[i].yaxis;
	prey_num--; /* we don't really realloc the vector we just don't read the whole things */
	prey_array = realloc(prey_array, (prey_num) * sizeof(prey));
}

void deplete_prey(prey * p){ /* we reduct the preys value due to too many predators feasting upon it */
	if (p -> num == 0) /* no predators attacking thus prey value increases */
		printf("need to find how it increases\n");
	p -> value -= p -> num * ( rand() / (float)RAND_MAX) * deplete;
	if (p -> value <= 0) /* prey is completely remove from the game*/
		remove_prey(p -> id);
}

/* --- print start --- */

void print_preys(){
	FILE * f1 = fopen("preytogod.txt","a");
	unsigned i;
	for (i = 0; i < prey_num; i++){
		fprintf(f1, "%u %u [%f %f] %u\n", prey_array[i].id,
		prey_array[i].value, prey_array[i].xaxis, prey_array[i].yaxis, prey_array[i].num);
	}
	fclose(f1);
}

void print_next(prey * p){
	FILE * f1 = fopen("next_gen.txt", "a");
	fprintf(f1, "-------- Gen %d: for prey %d [%lf %lf] --------\n", curr_flag, p -> id, p -> xaxis, p -> yaxis);
	unsigned i;
	for (i = 0; i < p -> num; i++){
		assert(p -> pred_index != NULL);
		fprintf(f1, "pos[%lf %lf]\n",	gens[curr_flag].pred[p -> pred_index[i]].xaxis, gens[curr_flag].pred[p -> pred_index[i]].yaxis);
	}
	fprintf(f1, "------------------------------ \n");
	fclose(f1);
}

void free_prey(){
	unsigned i;
	for (i = 0; i < prey_num; i++){
		if (prey_array[i].pred_index != NULL)
			free(prey_array[i].pred_index);
	}
	free(prey_array);
}

/* ------------------------------------------------ BRIEF EXPLANATION OF THE CODE ------------------------------------------------ */
/*
	The whole project, is a simulation of an evolutionary game where predators try to reproduce by feasting upon certain
preys. Obviously, this cannot happen without preys. In this file, exists every function related to manipulating the preys
of the game by adding them and depleting them. Depleting means reducing the value of that prey due to too many
predators feasting upon it. If the prey is completely depleted(value zero) we remove from the game. In order not to run
out of preys, each time we remove a prey we set a trigger. The trigger means that after some generations/rounds a new
prey will be added to replace the old one. The generation that the trigger will be enabled, is a random point between a
max and a min penalty. The list with every trigger is obviously in ascending order, since they take place in a timely order.
*/
