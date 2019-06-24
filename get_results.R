#concatenate results from all simulations

#SET THE PATH TO MATCH THE PROPER DIRECTORY BEFORE RUNNING THE SCRIPT

path = "/syn_com/syn"
files = "/syn"

param = c(0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8)
mut = c("05",  "2", "3")

for (m in mut){
    for (i in param){
	cat <- paste("cat ", path, "/_mut", m, files, i, files, i,"_mut",m, "*/ms_mutation_table.txt > mut_table", i, ".txt",  sep="")
	system(cat)
	comus <- paste("CoMuStats -ms -sfs -input mut_table", i, ".txt > ic_ign",m, "_", i, ".stats", sep="")
	system(comus)


	strat <- paste("paste ", path, "/_mut", m, files, i, files, i, "_mut", m, "*/strat_percent.txt > strategies", i, ".txt",  sep="")
	system(strat)

    }
    system(paste("mkdir ms_table", m, sep=""))
    system(paste("mv mut_table*.txt ms_table", m," ;  mv ms_table", m, " ", path, "/_mut", m, "/",  sep=""))

    system(paste("mkdir sum_stats", m, sep=""))
    system(paste("mv ic_ign*.stats sum_stats", m, " ; mv sum_stats", m, " ", path, "/_mut", m, "/",  sep=""))

    system(paste("mkdir strat_perc", m, sep=""))
    system(paste("mv strategies*.txt strat_perc", m, " ; mv strat_perc", m, " ", path, "/_mut", m, "/",  sep=""))
}
