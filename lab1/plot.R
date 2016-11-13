s <- read.table("cs.dat")

png('plot_cores.png')
plot(s$V1, s$V2, "l")
dev.off()

png('plot_memory.png')
plot(s$V1, s$V3, "l")
dev.off()
