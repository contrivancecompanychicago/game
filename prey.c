
/* In this file every function related to the preys of the game is implemented */
#include "prey.h"


unsigned prey_num = 10;
unsigned num_preys = 0;
unsigned playing_preys = 0;

unsigned max_penalty = 7;
unsigned min_penalty = 3;
double deplete = 0.8;	/* the max % of the prey that gets depleted from the predators */

float tot_fit = 0; 		/* total fitness/payoff among all players of a game */

float range = 45.0;

extern unsigned unable;	   /* determines the number of predators that did not play a game ~ starting value is the total number of predators for each generation */
extern double dimension;
extern unsigned curr_gen;
extern unsigned social_choices[3];

prey * preylist; 			/* list of every prey in the game */
trigger * triggerlist;	/* list of every addition of prey that should occur ~ obviously in ascending order */

void add_next(prey * p, predator * pred){
	next_gen * tmp = malloc(sizeof(next_gen));
	tmp -> pr = pred;
	pred -> fitness = 1;
	tmp -> fitness = 1;//payoff_matrix[p -> strategy -1] /  social_choices[p -> strategy - 1];
	tmp -> next = p -> ng;
	p -> ng = tmp;
	//printf("--> %d\n", p -> strategy);
	social_choices[pred -> strategy-1]++; /* we have the -1 since undifined = 0 in the enumeration */
	unable--;
}

void find_in_range(prey * p, unsigned latitude, unsigned longtitude, predator * root){
	if (root == NULL)	/* reached a leaf */
		return;
	/* we first check if the current node is in range */
	if (absolute(root -> xaxis - latitude) <= range && (absolute(root -> yaxis - longtitude) <= range))
		add_next(p, root);
	if (latitude - root -> xaxis <= range) /* if there is still reason to check left */
		find_in_range(p, latitude, longtitude, root -> lc);
	if (root -> xaxis - latitude <= range) /* if there is still reason to check right */
		find_in_range(p, latitude, longtitude, root -> rc);
}

/* ---------------------------------------------------- */

void add_prey(){
	prey * p = malloc(sizeof(prey));
	p -> id = ++num_preys;	/* we  increase the num of preys in the game by one */
	p -> value = (rand() % 50 ) + 50;
	p -> num = 0; /* number of predators playing the game ~ starts with zero */
	p -> xaxis = dimension * (rand() / (float)RAND_MAX); // isws na yparxei kapoios kalyteros tropos
	p -> yaxis = dimension * (rand() / (float)RAND_MAX);
	p -> ng = NULL;
	p -> next = preylist;		/* the list is in descending order regarding id */
	preylist = p;
	playing_preys++;
}

/* ---------------------------------------------------- */

void enable_trigger(){ /* the trigger that is enabled is always the first */
	if (preylist == NULL) /* safety clause - should never be triggered */
		assert(0);
	unsigned i;
	for ( i = 0; i < triggerlist -> prey_num; i++) /* we add as many preys as the trigger suggests */
		add_prey();
	if (triggerlist -> next == NULL) /* sole trigger */
		free(triggerlist);
	else{
		trigger * tmp = triggerlist;
		triggerlist = triggerlist -> next;
		free(tmp);
	}
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
	if (preylist == NULL) /* safety clause - should never be triggered */
		assert(0);
	prey * tmp = preylist;
	if (preylist -> id  == pid){
		preylist = preylist -> next;
		free(tmp);
		return;
	}
	prey * prev = tmp;
	while (tmp != NULL){
		if ( tmp -> id == pid){
			prev -> next = tmp -> next;
			if (tmp -> ng != NULL){
				next_gen * n = NULL;
				while (tmp -> ng != NULL){
					n = tmp -> ng;
					tmp = tmp -> next;
					free(n);
				}
			}
			free(tmp);
			playing_preys--;
			set_new_trigger();
			return;
		}
		else{
			prev = tmp;
			tmp = tmp -> next;
		}
	}
	printf("A prey with id %d was not found\n", pid);
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
	prey * tmp = preylist;
	FILE * f1 = fopen("preytogod.txt","a");
	while (tmp != NULL){
		fprintf(f1, "%u %u [%f %f]\n", tmp -> id, tmp -> value, tmp -> xaxis, tmp -> yaxis);
		tmp = tmp -> next;
	}
	fclose(f1);
}

void print_next(prey * p){
	next_gen * tmp = p -> ng;
	FILE * f1 = fopen("next_gen.txt", "a");
	fprintf(f1, "-------- Gen %d: for prey %d --------\n", curr_gen, p -> id);
	while(tmp != NULL){
		fprintf(f1, "id: %d par_id: %d fit: %f\n", tmp -> pr -> prid, tmp -> pr -> prid, tmp -> pr -> fitness);
		tmp = tmp -> next;
	}
	fprintf(f1, "------------------------------ \n");
	fclose(f1);
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
