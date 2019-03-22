#include "produce_output.h"

extern unsigned curr_flag;
extern unsigned curr_gen;
extern unsigned samples;
extern generation * gens;
extern unsigned genotype_size;
extern short * MutationEvents;

genome_sample * Sam_genome = NULL;

void print_samples(){
  FILE * f1 = fopen("samples.txt", "a");
  unsigned i,j;
  fprintf(f1, "SAMPLES: %u\n", samples);
  for (i = 0; i < samples; i++){
    fprintf(f1, "%u [%u %u]\n", i, Sam_genome[i].gen, Sam_genome[i].strategy);
    for (j = 0; j < genotype_size; j++)
      fprintf(f1, "%lu ",  Sam_genome[i].geno[j]);
    fprintf(f1, "\n");
  }
  fclose(f1);
}

void sampling(unsigned num){
  unsigned * samplesID = malloc(num * sizeof(unsigned));
  unsigned sampflag = 0;
  if (Sam_genome == NULL){
    Sam_genome = malloc(num * sizeof(genome_sample));
    samples = 0;
  }
  else
    Sam_genome = realloc(Sam_genome, (num + samples) * sizeof(genome_sample));

  unsigned i,j, random;
  for (i = 0; i < num; i++){
    sampflag = 0;
    random = rand() % gens[curr_flag].num;
    for (j = 0; j < i; j++){
      if (samplesID[j] == random){
        sampflag = 1;
        break;
      }
    }
    if (sampflag){
      i--;
      sampflag = 0;
      continue;
    }
    samplesID[i] = random;
  }

  for (i = samples; i < (samples + num); i++){
    Sam_genome[i].geno = malloc(genotype_size * sizeof(num_type));
    memcpy(Sam_genome[i].geno, gens[curr_flag].pred[samplesID[i-samples]].geno, genotype_size * sizeof(num_type));
    Sam_genome[i].strategy = gens[curr_flag].pred[samplesID[i-samples]].strategy;
    Sam_genome[i].gen = curr_gen;
  }
  samples += num;
  fprintf(stderr, "sampl: %u\n", samples);
  free(samplesID);
}

/* this is convert otherwise print */
void print_binary( num_type number, unsigned counter, unsigned max, FILE * out_file){
  if( counter < max )
    print_binary( number >> 1, ++counter, max, out_file);
  putc((number & 1) ? '1' : '0', out_file);
}

//prints the mutation table of the predators in the last generation
void ms_output(){
  /* we first need to print the general information */
  FILE * f1 = fopen("ms_mutation_table.txt", "w");
  fprintf(f1, "//\n");
  long long unsigned bits = sizeof(num_type) * 8;
  long long unsigned max_segment = genotype_size * 8 * sizeof(num_type) + 1;
  unsigned s, i, j;
  for (s = 0; s < samples; s++){
    for (i = 0; i < genotype_size; i++){
      if (Sam_genome[s].geno[i])
        print_binary(Sam_genome[s].geno[i], 1, bits, f1);
      else{
        for (j = 0; j < bits; j++)
          fprintf(f1, "0");
      }
      fprintf(f1, "\n");
    }
  }

  // fprintf(f1, "//\nsegsites: %d\npositions: ", (mut));   // <--- this is tricky AF
  // for (i = 0; i < mut; i++){ /* print the positions where the mutation occcured */
  //   if (poly[i])
  //     fprintf(f1, "%lf ", ( mut_pos[i] / (float)() ));
  // }
  //
  // unsigned bit_num = sizeof(num_type)*8;
  // unsigned s, i, j;
  // for (s = 0; s < samples; s++){ /* check every sample for mutations and print them */
  //   for (i = 0; i < genotype_size; i++)
  //     if (MutationEvents[i]) /* if there is at least a bit with a value of '1' */
  //       print_binary(gens[curr_flag], 0, bit_num, f1);                      // ????? is it curr_flag ?????
  //     else{ /* no need to convert to binary we already know it's 0s in every position */
  //       for (j = 0; j < bit_num; j++)
  //         fprintf(f1, "0");
  //     }
  //   fprintf(f1, "\n");
  // }
  fclose(f1);
}

void vcf_output(){
  FILE * f1 = fopen("vcf_mutation_table.txt", "w");
  fclose(f1);
}

void free_samples(){
  unsigned i;
  for (i = 0; i < samples; i++)
    free(Sam_genome[i].geno);
  free(Sam_genome);
}
/*
Αν έχω ancestral sampling δεν μπορώ να εκτυπώνω μόνο τις 64-αδες με το mutation διότι θα προστεθούν πιθανώς κι άλλες.
ΤΑ POSITIONS ΕΙΝΑΙ ΘΕΜΑ, όπως κι ο αριθμός γιατί δεν τα ξέρω πριν μπω στη διαδικασία να εκτυπώσω το mutation_table.
*/
