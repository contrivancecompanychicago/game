#include "produce_output.h"

extern unsigned curr_flag;
extern unsigned curr_gen;
extern unsigned samples;
extern generation * gens;
extern unsigned genotype_size;
extern short * MutationEvents;

genome_sample * Sam_genome = NULL;
extern gsl_rng * r;
extern unsigned social_choices[3];
extern short byte2bit;

void print_strat_percentages(short gen){
  unsigned i;
  social_choices[0] = 0;
  social_choices[1] = 0;
  social_choices[2] = 0;
  unsigned tot = gens[gen].num;
  for (i = 0; i < tot; i++)
    social_choices[gens[gen].pred[i].strategy - 1]++;
  FILE * f1 = fopen("strat_percent.txt", "a");
  fprintf(f1, "%f %f %f\n", (float)social_choices[0] / tot,
  (float)social_choices[1] / tot, (float)social_choices[2] / tot);
  fclose(f1);
}

void print_strategies(unsigned gen){
  unsigned i;
  FILE * f1 = fopen("strategies.txt", "a");
  for (i = 0; i < gens[gen].num; i++)
    fprintf(f1, "%u ", gens[gen].pred[i].strategy);
  fprintf(f1, "\n");
  fclose(f1);
}

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
  print_strat_percentages(!curr_flag);
  return; /* will be removed later */
  gsl_permutation * perm = gsl_permutation_alloc((size_t)gens[curr_flag].num);
  gsl_permutation_init(perm);
  gsl_ran_shuffle(r, perm -> data, (size_t)gens[curr_flag].num, sizeof(size_t));

  if (Sam_genome == NULL){
    Sam_genome = malloc(num * sizeof(genome_sample));
    samples = 0;
  }
  else
    Sam_genome = realloc(Sam_genome, (num + samples) * sizeof(genome_sample));

  unsigned i;
  for (i = samples; i < (samples + num); i++){
    Sam_genome[i].geno = malloc(genotype_size * sizeof(num_type));
    memcpy(Sam_genome[i].geno, gens[curr_flag].pred[(unsigned)perm->data[i]].geno, genotype_size * sizeof(num_type));
    Sam_genome[i].strategy = gens[curr_flag].pred[(unsigned)perm->data[i]].strategy;
    Sam_genome[i].gen = curr_gen;
  }
  samples += num;
  fprintf(stderr, "sampl: %u\n", samples);

  FILE * f1 = fopen("sampled_strategies.txt","a");
  fprintf(f1, "Gen: %u w/ %u samples\n", curr_gen, samples);
  for (i = samples; i < (samples + num); i++)
    fprintf(f1, "%u ", gens[curr_flag].pred[(unsigned)perm->data[i]].strategy);
  fprintf(f1, "\n");
  fclose(f1);
  gsl_permutation_free(perm);
}

/* this is convert otherwise print */
void print_binary( num_type number, unsigned counter, unsigned max, FILE * out_file){
  if( counter < max )
    print_binary( number >> 1, ++counter, max, out_file);
  putc((number & 1) ? '1' : '0', out_file);
}

void ms_output(){
  /* we first need to print the general information */
  unsigned i;
  long long unsigned bits = sizeof(num_type) * byte2bit;
  FILE * f1 = fopen("ms_mutation_table.txt", "w");
  fprintf(f1, "//\nsegsites: %llu\npositions: ", genotype_size * bits);
  for (i = 0; i <  genotype_size * bits; i++)
    fprintf(f1, "%u ", i);
  fprintf(f1, "\n");
  unsigned s, j;
  for (s = 0; s < samples; s++){
    for (i = 0; i < genotype_size; i++){
      if (Sam_genome[s].geno[i])
        print_binary(Sam_genome[s].geno[i], 1, bits, f1);
      else{
        for (j = 0; j < bits; j++)
          fprintf(f1, "0");
      }
    }
    fprintf(f1, "\n");
  }
  fclose(f1);
}

void vcf_output(){
  FILE * f1 = fopen("vcf_mutation_table.txt", "w");
  fprintf(f1, "#fileformat=VCFv4.3\n");


  fclose(f1);
}

void free_samples(){
  if (Sam_genome = NULL)
    return;
  unsigned i;
  for (i = 0; i < samples; i++)
    free(Sam_genome[i].geno);
  free(Sam_genome);
}
