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

param = c (0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8) #different values for the Synergy, Competition thresholds.
len <- length(param) #number of different values

onoma = "recigncom"
mut = c(0.005, 0.01, 0.02, 0.03) #different values for the mutation rate

ff <- "05"
out_dir <- "this"

for (m in mut){ #for each mutation rate


  if (m == 0.01){
     ff <- "1"
  } else if  (m == 0.02){
     ff <- "2"
  } else if (m == 0.03){
     ff <- "3"
  }
  mut_mkdir <- paste("mkdir _mut",ff, sep="")


    for (i in ( 1 : (len-1) )){       #for each Synergy threshold
        for (j in ( (i+1) : len) ){   #for each Competition threshold


	        seed <- sample(1:100000000, n,  replace=FALSE)
	        both_param <- paste(param[i], "_", param[j], sep="")
	        #out_path <- paste(out_dir , sep="")

   	      cmds <- paste(curDir, "/feg -pred 500 -prey 1 -pinf 1 3 -burn 4000 -rnds 5000 -synr ",param[i], " -comr ", param[j], " -nsyn 500  -seed ", seed, " -mutr ", m, sep="")
    	    write.table(x=cmds, file="cmd.txt", append=F, quote=F, row.names=F, col.names=F)

    	    make_dir <- paste("mkdir ", out_dir, both_param, sep="")
    	    system(make_dir)

       	  foreach(p =  1:n)%dopar%{
        		setwd(file.path(curDir, name))
        		dirName <- paste(onoma, both_param, "_mut", ff, p, sep="")
        		working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
        		dir.create(working_dir, showWarnings=FALSE)
        		setwd(working_dir)
        		cmd <- cmds[p]
        		system(cmd)
        		getwd()
    	    }

  	       setwd(file.path(curDir, name))
  	       mv <- paste("mv ", onoma, both_param,"_mut",  ff, "* ", out_dir, both_param, sep="")
  	       system(mv)

  	       mv2 <- paste("mv ",out_dir,  both_param," _mut", ff, sep="")
  	       system(mv2)
      }
   }
}
