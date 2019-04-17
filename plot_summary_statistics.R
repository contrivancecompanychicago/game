#reading the data

stats <- read.table("gen_recomb_stats.txt", header = TRUE)
#clean the dataset ~ probably not required but just in case
k <- 1:length(names(stats))
stats.clean <- stats[complete.cases(stats.),]
names(stats.clean)

## Compare statistics between initial and end population
#plot(density(stats.left.clean[,2]), col='black')
#points(density(stats.left.clean[,43]), type='l', col='red')
#legend("topright", legend=c("end pop", "source pop"), col=c("black", "red"), lw=2)

nm <- names(stats.left.clean)
pdf("leftret.pdf")
for (i in 1:20){
        x1 <- density(stats.left.clean[,i], na.rm=TRUE)
        x2 <- density(stats.left.clean[,i+41], na.rm=TRUE)
        maxy <- max(x1$y, x2$y)
        maxx <- max(x1$x, x2$x)
        miny <- min(x1$y, x2$y)
        minx <- min(x1$x, x2$x)

        plot(x1, ylim=c(miny, 1.2*maxy), xlim=c(minx, 1.2*maxx), main=nm[i+82], col='black')
        points(x2, type='l', col='red')
        legend("topright", legend=c("end pop", "source pop"), col=c("black", "red"), lw=2)
}
dev.off()
