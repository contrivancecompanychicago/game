rm(list=ls())
library(foreach)
library(doMC)
n <- 1
registerDoMC(1)

#system("make")

curDir <- getwd()
name <- ""

seed <- sample(1:100000000, n,  replace=FALSE)
write.table(seed, file="seeds.txt", quote=F, row.names=F, col.names=F)

cmds <- paste(curDir, "/game -pred 1000 -prey 4 -pinf 3 0 2 3 -rnds 10000 -burn 1000 -smpl 10000 10 -synr 0.33 -comr 0.66 -seed ", seed, sep="")
write.table(x=cmds, file="cmd.txt", append=F, quote=F, row.names=F, col.names=F)


foreach(i =  1:n)%dopar%{
    setwd(file.path(curDir, name))
    dirName <- paste("rungen", i, sep="")
    working_dir<-file.path(paste(curDir, "/", name, sep=""), dirName)
    dir.create(working_dir, showWarnings=FALSE)
    setwd(working_dir)
    cmd <- cmds[i]
    system(cmd)
    getwd()
}
