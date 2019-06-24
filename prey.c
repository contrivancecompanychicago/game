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

float range = 50.0; /* switch to 20, add two opposite sided preys */

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
		//if (abs(p -> xaxis -  gens[gen].pred[i].xaxis) <= range
		// && abs(p -> yaxis -  gens[gen].pred[i].yaxis) <= range
		// && gens[gen].pred[i].fitness == 0){ /* in range and not in another game */
			add_next(p, i, gens[gen].pred[i].strategy);
		//}
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
		prey_array[i].value = 10;//(rand() % 50 ) + 50;
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

void enable_trigger(){ /* the trigger that is enabled is always the first */
	// unsigned i;
	// for ( i = 0; i < triggerlist -> prey_num; i++) /* we add as many preys as the trigger suggests */
	// 	add_prey();
	// if (triggerlist -> next == NULL) /* sole trigger */
	// 	free(triggerlist);
	// else{
	// 	trigger * tmp = triggerlist;
	// 	triggerlist = triggerlist -> next;
	// 	free(tmp);
	// }
}

void set_new_trigger(){ /* at that moment(round - generation) in time a new prey will be added */
	unsigned re_entry_time = curr_gen  + (rand() % (max_penalty - min_penalty) + min_penalty); /* calculate the re-entrance time slot */
	if (triggerlist == NULL){
		triggerlist = malloc(sizeof(trigger));
		triggerlist -> entry_time = re_entry_time;
		triggerlist -> prey_num = 1;
		triggerlist -> next = NULL;
		return;
	}
	trigger * tmp = triggerlist;
	while (tmp -> next != NULL && tmp -> entry_time < re_entry_time)
		tmp = tmp -> next;
	if (tmp -> entry_time == re_entry_time) /* another trigger set at the same round */
		tmp -> prey_num++;
	else{ /* first trigger on that round */
		trigger * t = malloc(sizeof(trigger));
		t -> entry_time = re_entry_time;
		t -> prey_num = 1;
		t -> next = tmp -> next;
		tmp -> next = t;
	}
}

/* -------------- deplete & remove start -------------- */

void remove_prey(unsigned pid){

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
