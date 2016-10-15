expData <- scan("hist.dat")
binCount <- 40
minValue <- min(expData)
maxValue <- max(expData)
stdDev <- sd(expData)

                                        # Для экспоненциального распределения DX = lamda^-2.
lambda <- 1 / stdDev
print(paste("Estimation of lambda is:", lambda))

expectedData <- rexp(length(expData), lambda)
minValue2 <- min(expectedData)
maxValue2 <- max(expectedData)

if (minValue2 < minValue) {
    minValue <- minValue2
}

if (maxValue2 > maxValue) {
    maxValue <- maxValue2
}

binsize <- (maxValue - minValue) / binCount
bins <- seq(minValue, maxValue, binsize)

png('outhist.png')
hist(expData, col = "red", freq = FALSE, breaks = bins)
expHist <- hist(expectedData, plot = FALSE, breaks = bins)
points(expHist$mids, expHist$density, type = "l", col = "blue", lwd = 2)
dev.off()
