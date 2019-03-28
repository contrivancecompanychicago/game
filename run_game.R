m(list=ls())
library(foreach)
library(doMC)
n <- 1
registerDoMC(1)

system("make")


curDir <- getwd()

name <- ""


seed <- sample(1:100000000, n,  replace=FALSE)
write.table(seed, file="seeds.txt", quote=F, row.names=F, col.names=F)

cmds <- paste(curDir, "game -pred 10000 -prey 4 -pinf 4 0 2 3 -rnds 100000 -burn 10000 -smpl 10000 50 -smpl 18000 500 -bttl 1000 1000 -bttl 2000 10000 -bttl 30000 12000"
write.table(x=cmds, file="cmd.txt", append=F, quote=F, row.names=F, col.names=F)


foreach(i =  1:n)%dopar%{
    #system(paste("rm -r run.", i, sep=""))
    setwd(file.path(curDir, name))
    dirName <- paste("run_left", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)
    cmd <- cmds[i]
    system(cmd)
    getwd()
}
