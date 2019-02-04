#include "cmd_params.h"

extern unsigned rounds;
extern unsigned seed;
/* ----- from prey.c ----- */
extern unsigned prey_num;


/* --- from predator.c --- */
extern char const_size;
extern unsigned pred_num;
extern unsigned cram;
extern ram * ram_h;


ram * tail = NULL;

void add_ram(unsigned cr){
	ram * tmp = malloc(sizeof(ram));
	tmp -> cram = cr;
	tmp -> next = NULL;
	if (ram_h == NULL){
		cram = cr;
		ram_h = tmp;
		tail = ram_h;
		return;
	}
	tail -> next = tmp;
	tail = tail -> next;
}

void print_help(){
	printf("\n\nhelp is for the weak. Go read the fucking code\n\n");
}

void cmd_params(int argc, char** argv){
	int i;
	for (i = 1; i < argv[i]; i++){

		/* --------- general use ---------- */

		/* seed for random */
		if ( (!strcmp(argv[i], "-seed" ) ) ){
			seed = atoi(argv[++i]);
			continue;
		}
		/* gaussian parameter */
		/*if ( (!strcmp(argv[i], "-gsnd" ) ) ){
			sigma = atof(argv[++i]);
			continue;
		}*/

		/* ----- help ----- */
		if ( (!strcmp(argv[i], "-help" ) ) ){
			print_help();
			exit(0);
		}


		if ( (!strcmp(argv[i], "-rnds" ) ) ){
			rounds = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-cnst" ) ) ){
			const_size = 1;
			continue;
		}

		if ( (!strcmp(argv[i], "-pred" ) ) ){
			pred_num = atoi(argv[++i]);
			continue;
		}

		if ( (!strcmp(argv[i], "-cram" ) ) ){
			add_ram(atoi(argv[++i]));
			continue;
		}

		if ( (!strcmp(argv[i], "-prey" ) ) ){
			prey_num = atoi(argv[++i]);
			continue;
		}


		fprintf(stderr, "Argument %s is invalid\n\n", argv[i]);
	}
}
