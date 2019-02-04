#include "predator.h"

predator * root;


extern strategy_t choose_strategy(predator * p, strategy_t social);

extern predator ** gens;
extern unsigned curr_gen;
extern double dimension;
extern float range;
extern unsigned social_choices[3];
extern float payoff_matrix[3];
extern float tot_fit;
extern double recombination_rate = 0.01;

/* ------ phenotype expression ------ */
unsigned phenotype_size = 5; 	/* determines how many bases affect the strategy of each predator */
float com_rate = 0.65;	 	/* determines below which threshold does the strategy become competitive */
float syn_rate = 0.35;	 	/* determines below which threshold does the strategy become synergetic */
unsigned migration_size = 5;	/* determines how many bases affect the migration pattern of each predator */
/* ---------------------------------- */

char const_size = 0; 		/* determines whether the number of predators remains constant in each generation ~ how the fuck does this work? */
unsigned pred_num = 10;		/* initial number of predators ~ Default 10 unless user defined */
unsigned pred_pid = 0;

double not_play = 0;		/* default fitness for predators prior to their partipation to any game */
unsigned unable;	    	/* determines the number of predators that did not play a game ~ starting value is the total number of predators for each generation */

unsigned * People; 			/* number of predators in each generation */

next_gen * chosen = NULL; 	/* list of predators that are going to reproduce from each subgame */

double fit_counter = 0; 	/* counter used for the selection of the parents of the child */

/* ram reduction */
unsigned cram = -1; 		/* marks the generation for which the tree is pruned */
ram * ram_h = NULL;
/* ------------- */

predator * choose_parent(predator * p, predator * root){
	if (root -> fitness < tot_fit - fit_counter) /* found the parent */
		return p;
	fit_counter += p -> fitness;
	predator * tmp = NULL;
	if (root -> lc != NULL){
		tmp = choose_parent(p, root -> lc);
		//if (tmp != NULL) /* we traced the parent somewhere in the left branch */
		return tmp;
	}
	if (root -> rc != NULL){
		if (tmp != NULL) /* we traced the parent somewhere in the left branch */
		tmp = choose_parent(p, root -> rc);
		//if (tmp != NULL) /* we traced the parent somewhere in the right branch */
		return tmp;
	}
	if (root -> lc == NULL && root -> rc == NULL)
		return NULL; /* found in neither side */
	assert(0);
}

void recombine(predator * p){
	unsigned cut_point = rand() % phenotype_size;
	float random = (float)rand()/RAND_MAX;
	unsigned i;
	if (random < 0.5){	/* first part inhereted from parent1 */
		for (i = 0; i < cut_point; i++)
			p -> pheno[i] = p -> parent1 -> pheno[i];
		for (i = cut_point; i < phenotype_size; i++)
			p -> pheno[i] = p -> parent2 -> pheno[i];
	}
	else{ /* first part inherited from parent2 */
		for (i = 0; i < cut_point; i++)
			p -> pheno[i] = p -> parent1 -> pheno[i];
		for (i = cut_point; i < phenotype_size; i++)
			p -> pheno[i] = p -> parent2 -> pheno[i];
	}
	p -> rec = cut_point;
}

/* creates a predator for the next generation and places it on the map */
void next_generation(unsigned num, unsigned latitude, unsigned longtitude){
	unsigned i = 0;
	predator * p = malloc(sizeof(predator));
	p -> prid = ++pred_pid;
	//p -> ratio = 0.0; // needs fixing
	p -> height = 0;
	p -> flag = 0;
	p -> rec = -1;

	/* position of new predator depends on the position of the prey */
	p -> xaxis = ((float)rand()/RAND_MAX) * 2 * range  + latitude;
	p -> yaxis = ((float)rand()/RAND_MAX) * 2 * range  + longtitude;

	/* ---------------- select parents ---------------- */
	fit_counter = 0;
	p -> parent1 = choose_parent(p, gens[curr_gen]);
	p -> parent2 = NULL;
	while (p -> parent2 != p -> parent1)
		p -> parent2 = choose_parent(p, gens[curr_gen]);
	p -> strategy = choose_strategy(p, synergy);
	assert(p -> parent1 != NULL);

	/* ------------- determine phenotype ------------- */
	float random = (float)rand()/RAND_MAX;
	p -> pheno = malloc(phenotype_size * sizeof(char));
	if (random < recombination_rate){
 		recombine(p);
	}
	else if (random < 0.5){
		for (i = 0; i < phenotype_size; i++)
			p -> pheno[i] = p -> parent1 -> pheno[i];
	}
	else{
		for (i = 0; i < phenotype_size; i++)
			p -> pheno[i] = p -> parent2 -> pheno[i];
	}
	/* ----------------------------------------------- */

	p -> pnode = NULL;
	p -> lc = NULL;
	p -> rc = NULL;

	p -> fitness = not_play;

	if (gens[curr_gen + 1] == NULL){ /* first predator in the new generation */
		gens[curr_gen + 1] = p;
		return;
	}
	add_predator(p, gens[curr_gen + 1]);
}

/* -------- next generation end -------- */

/* print the predator tree of the current generation */
void print_predators(predator * pr){//}, char * filename){
	//printf("Printing...\n");
	if (pr == NULL)
		return;
	predator * tmp = pr;
	//char * fname = malloc(sizeof(filename));
	//fname = strdup(filename);
	FILE * f1 = fopen("wirsinddiejaager.txt", "a");
	fprintf(f1, "%d %d | %d\n", tmp -> prid, tmp -> height, curr_gen);
	//fprintf(f1, "%d %d [%f %f] \n", tmp -> prid, tmp -> strategy, tmp -> xaxis, tmp -> yaxis);
	fclose(f1);
	if (pr -> lc != NULL)
		print_predators(pr -> lc);
	if (pr -> rc != NULL)
		print_predators(pr -> rc);
}

/* ------------------------ search start ------------------------ */

predator * find_predator(unsigned prid, unsigned latitude, unsigned longtitude, predator * tmp){  /* we are using in-order traversal */
	//printf("Find predator\n");
	if (tmp -> prid == prid)
		return tmp;
	else if ( (tmp -> xaxis > latitude)  || (tmp -> xaxis == latitude && tmp -> yaxis >= longtitude ) ){ /* need to go left */
		if (tmp -> lc != NULL)
			return find_predator(prid, latitude, longtitude, tmp -> lc);
		else
			return NULL;		/* reached the end going in the correct direction and still found nothing */
	}
	else{
		if (tmp -> rc != NULL)
			return find_predator(prid, latitude, longtitude, tmp -> rc);
		else
			return NULL;		/* reached the end going in the correct direction and still found nothing */
	}
}

/* ------- height and balance start ------- */

int get_height(predator * pr){
	if ( (pr -> lc == NULL) && (pr -> rc == NULL) ) /* leaf */
		return 0;
	if (pr -> lc == NULL)	/* only has right child */
		return get_height(pr -> rc) + 1;
	if (pr -> rc == NULL)	/* only has left child */
		return get_height(pr -> lc) + 1;
	return (max(get_height(pr -> lc), get_height(pr -> rc)) + 1);
}


void rotate_left(predator * pr){
	//printf("LEFT rotation\n");
	assert(pr != NULL);
	assert(pr -> rc != NULL);

	/* the parental node (if exists) needs to point to the LC */
	predator * x = pr -> pnode;
	if (x != NULL){ /* non-root rotation */
		if (x -> lc != NULL && x -> lc -> prid == pr -> prid) /* left exists and is the node */
			x -> lc = pr -> rc;
		else
			x -> rc = pr -> rc;
	}
	else /* root rotation */
		gens[curr_gen] = pr -> rc;

	predator * tmp = pr -> rc;
	tmp -> pnode = x;

	pr -> rc = tmp -> lc;
	tmp -> lc = pr;
	pr -> pnode = tmp;
	pr -> height = get_height(pr);
	tmp -> height = get_height(tmp);
}

void rotate_right(predator * pr){
	//printf("RIGHT rotation\n");
	assert(pr != NULL);
	assert(pr -> lc != NULL);


	/* the parental node (if exists) needs to point to the LC */
	predator * x = pr -> pnode;
	if (x != NULL){ /* non-root rotation */
		if (x -> lc != NULL && x -> lc -> prid == pr -> prid)
			x -> lc = pr -> lc;
		else
			x -> rc = pr -> lc;
	}
	else /* root rotation */
		gens[curr_gen] = pr -> lc;

	predator * tmp = pr -> lc;
	tmp -> pnode = x;
	/* the RC of the LC become the LC of the target node */
	pr -> lc = pr -> lc -> rc;
	tmp -> rc = pr;
	pr -> pnode = tmp;
	pr -> height = get_height(pr);
	tmp -> height = get_height(tmp);
}

/*
	returns the balance of  the tree, + means left side is greater, - means right side. |2| means impabalance.
*/
int check_balance(predator * pr){
	if ( pr -> rc == NULL && pr -> lc == NULL) /* leaf case */
		return 0;
	else if (pr -> rc == NULL){ /* left child only */
		if (pr -> lc -> height != 0) /* if that left child is not a leaf */
			return 2;
		return 1;
	}
	else if (pr -> lc == NULL){ /* right child only */
		if (pr -> rc -> height != 0) /* if that right child is not a leaf */
			return -2;
		return -1;
	}
	else{ /* has both child nodes */
		int x = absolute(pr -> lc -> height - pr -> rc -> height);
		if ( x < 2 ){
			if (pr -> rc -> height > pr -> lc -> height)
				return -x; /* could be return -1; */
			return x; /* either 1 or 0 */

		}
		else if ( pr -> rc -> height > pr -> lc -> height)
			return -2;
		else
			return 2; /* balanced so we don't really care */
	}
}

void balance(predator * pr){
	int bal = check_balance(pr);
	if (bal == 2){ /* imbalanced left */
		if (check_balance(pr -> lc) > 0){	/* left - left */
			//printf("LL\n");
			rotate_right(pr);
		}
		else{ 								/* left - right */
			//printf("LR\n");
			rotate_left(pr -> lc);
			rotate_right(pr);
		}
	}
	else if (bal == -2){ /* imbalanced right */
		if (check_balance(pr -> rc) < 0){	/* right - right */
			//printf("RR\n");
			rotate_left(pr);
		}
		else{								/* right - left */
			//printf("RL\n");
			rotate_right(pr -> rc);
			rotate_left(pr);
		}
	}
	if ( pr -> pnode != NULL) /* going up till we reach the root */
		balance(pr -> pnode);
}



/* -------- insertion start -------- */



void add_predator(predator * root, predator * tmp){
	//printf("add_predator\n");
	if ( tmp == NULL ){ /* safety clause should never be triggered */
		printf("NULL predator \n");
		assert(0);
	}

	FILE * f1 = fopen("added.txt", "a");
	fprintf(f1, "id:%d, xaxis:%f\n", tmp -> prid, tmp -> xaxis);
	fclose(f1);

	/* left side */
	if ( (tmp -> xaxis < root -> xaxis)  || (tmp -> xaxis == root -> xaxis && root -> yaxis <= tmp -> yaxis) ){
		if (root -> lc != NULL) /* non-leaf case */
			add_predator(root -> lc, tmp);
		else{ /* reached leaf so we add and chech the balance of the tree */
			root -> lc = tmp;
			//root -> lheight = 1
			tmp -> pnode = root;
			tmp -> height = 0;
			root -> height = get_height(root);
			balance(root);	/* no need to check balance for tmp leafs are always balanced */
		}
	}
	/* right side */
	else {
		if (root -> rc != NULL) /* non-leaf case */
			add_predator(root -> rc, tmp);
		else{
			root -> rc = tmp;
			//root -> rheight = 1;
			tmp -> pnode = root;
			tmp -> height = 0;
			root -> height = get_height(root);
			balance(root);	/* no need to check balance for tmp leafs are always balanced */
		}
	}
}

/* initialization of a new predator */
void init_pred(){
	//printf("init pred\n");
	predator * tmp = malloc(sizeof(predator));
	tmp -> prid = pred_pid++;

	tmp -> parent1 = NULL; /* first generation has no parents */
	tmp -> parent2 = NULL; /* first generation has no parents */
	tmp -> xaxis = ((float)rand()/RAND_MAX) * dimension;
	tmp -> yaxis = ((float)rand()/RAND_MAX) * dimension;
	tmp -> height = 0;
	tmp -> flag = 0;
	tmp -> rec = -1;

	tmp -> pnode = NULL;
	tmp -> lc = NULL;
	tmp -> rc = NULL;

	tmp -> pheno = malloc(phenotype_size * sizeof(char));
	unsigned i;
	for (i = 0; i< phenotype_size; i++){
		if (rand() % 2)
			tmp -> pheno[i] = 1;
		else
			tmp -> pheno[i] = 0;
	}
	//tmp -> migr = calloc(migration_size, sizeof(char));

	tmp -> fitness = not_play;
	tmp -> strategy = choose_strategy(tmp, synergy); // NEEDS FIXING

	if (gens[curr_gen] == NULL){
		gens[curr_gen] = tmp;
		return;
	}
	add_predator(gens[curr_gen], tmp);
}

/* ------------------ removal start ------------------ */
predator * next_inOrder(predator * p){
	predator * tmp = p;
	while ( tmp -> lc != NULL)
		tmp = tmp -> lc;
	return tmp;
}

void remove_predator(unsigned prid, unsigned latitude, unsigned longtitude, unsigned gener){
	predator * p = find_predator(prid, latitude, longtitude, gens[gener]);
	//printf("Found it\n");
	if (p == NULL){ /* non-existing predator */
		printf("can't remove something non-existant\n");
		assert(0); /* might need to change this into a simple return */
	}
	free(p -> pheno);
	//free(p -> migr);
	if (p -> lc != NULL && p -> rc != NULL){ /* has both children */
		predator * tmp = next_inOrder(p -> rc); 	/* we look for left-most child of p -> rc */

		/*********************************************************************/
		if (tmp -> prid != p -> rc -> prid) 	/* if p -> rc isn't the next inOrder node */
			tmp -> pnode -> lc = tmp -> rc;	/* temporary removal of tmp from the tree structure */

		if (p -> pnode -> lc -> prid == p -> prid) /* swapping the to-be-deleted node with the next inOrder */
			p -> pnode -> lc = tmp;
		else
			p -> pnode -> rc = tmp;
		/*********************************************************************/

	}
	else if (p -> lc != NULL){ /* has only left child */
		if (p -> pnode -> lc -> prid == p -> prid)
			p -> pnode -> lc = p -> lc;
		else
			p -> pnode -> rc = p -> lc;
		p -> lc -> pnode = p -> pnode;
	}
	else if (p -> rc != NULL){ /* has only right child */
		if (p -> pnode -> lc -> prid == p -> prid)
			p -> pnode -> lc = p -> rc;
		else
			p -> pnode -> rc = p -> rc;
		p -> rc -> pnode = p -> pnode;
	}
	else{ /* is leaf */
		if (p -> pnode -> lc -> prid == p -> prid){
			p -> pnode -> lc = NULL;
		}
		else{
			p -> pnode -> rc = NULL;
		}

	}
	p -> pnode -> height = get_height(p -> pnode);
	balance(p -> pnode);
	free(p);
}

/* ------------- memory reduction start ------------- */
void erase(predator * p, unsigned gener){
	if (p == NULL) /* reached a leaf */
		return;
	erase(p -> lc, gener);
	erase(p -> rc, gener);

	if (p -> flag == 0){ /* if he did not reproduce */
		p -> parent1 -> flag--;
		p -> parent2 -> flag--;
		remove_predator(p -> prid, p -> xaxis, p -> yaxis, gener);
	}
}


void prune(){
	unsigned pruned = 0;
	printf("Pruning the Genealogy Tree %d \n", cram);
	unsigned i;
	for (i = curr_gen - 1; i >= 0; i++){
		if ( gens[i] == NULL)
			break;
		erase(gens[i], i);
	}
	/* prepare for the next prune - if there is one */
	if ( ram_h -> next != NULL){
		ram * tmp = ram_h;
		ram_h = ram_h ->next;
		cram = ram_h -> cram;
		free(tmp);
	}
	else{
		free(ram_h);
		cram = -1;
	}

}

/* -------------- memory reduction end -------------- */

void free_tree(predator * root){
	if (root == NULL)
		return;
	free_tree(root -> lc);
	free_tree(root -> rc);
	free(root);
}

/* -----------------------------------------------------------------------------------------------  NOTES ----------------------------------------------------------------------------------------------- */
/*
	We are using InOrder parsing for searching. Since the data structure in use is an AVL tree, complexity of all these methods is O(log(n)), besides rotations which have a complexity of O(1).
*/
