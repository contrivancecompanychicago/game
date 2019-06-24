list=ls()
library(foreach)
library(doMC)
n <- 10000        #number of simulations to run for each scenario
registerDoMC(24)  #Number of parallel runs to be done simultaneously. TREAT WITH CARE. The number given will be the number
                  #of parallel runs regardless of the system. For example, in an environment with 10 threads if this is
                  #set to 100 it will try to perform 100 runs at the same time, possibly causing the system to fail.

system("make")    #compile before running

curDir <- getwd() #set working directory
name <- ""


#Since this script is for a two strategy simulation, one of the three possible strategies should be deemed unavailable This is achieved by:
# (a) to exclude the Ignore strategy, set an equal threshold for both Synergy and Competition. If ( aggression < threshold) Synergy else Competition.
# (b) to exclude the Synergy strategy, set the Synergy threshold to 0.
# (c) to exclude the Competition strategy, set the Competition threshold to a value greater that 1(1.1). This requires an aggression percentage of 110% which is impossible.

param = c (0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8) #different values for the Synergy, Competition thresholds.

onoma = "recigncom"
mut = c(0.005, 0.01, 0.02, 0.03) #different values for the mutation rate

ff <- "05" # to store runs with different mutation rates to different folders/files. Goes along with the if condition below.
out_dir <- " recigncom" #output directory folder.

for (m in mut){ #for each mutation rate

  if (m == 0.01){
     ff <- "1"
  } else if  (m == 0.02){
     ff <- "2"
  } else if (m == 0.03){
     ff <- "3"
  }

  for (j in param ) { #for each thresholds
    seed <- sample(1:100000000, n,  replace=FALSE) #random seeds

    #actual command line parameters
    cmds <- paste(curDir, "/game -pred 500 -prey 1 -pinf 1 3 -rnds 3000 -burn 500  -synr 0.0 -comr ", j, " -neut 500  -seed ", seed, " -mutr ", m, sep="")
    write.table(x=cmds, file="cmd.txt", append=F, quote=F, row.names=F, col.names=F)

    #make directory to store this set of simulations
    make_dir <- paste("mkdir", out_dir, j, sep="")
    system(make_dir)

    #actually run FEG and store each run in a separate folder.
    foreach(i =  1:n)%dopar%{
      setwd(file.path(curDir, name))
      dirName <- paste(onoma, j, "_mut", ff, i, sep="")
      working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
      dir.create(working_dir, showWarnings=FALSE)
      setwd(working_dir)
      cmd <- cmds[i]
      system(cmd)
      getwd()
    }

  setwd(file.path(curDir, name))
  mv <- paste("mv ", onoma, j,"_mut",  ff, "*", out_dir, j, sep="")
  system(mv)

  mv2 <- paste("mv",out_dir,  j," _mut", ff, sep="")
  system(mv2)

  }
}
