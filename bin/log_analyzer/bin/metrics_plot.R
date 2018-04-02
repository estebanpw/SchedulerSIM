# Plots different metrics from a scheduling log file
library(gridExtra)

#!/usr/bin/env Rscript
args = commandArgs(trailingOnly=TRUE)

if(length(args) < 9){
  stop("USE: Rscript --vanilla metrics.R <m_cost> <m_queued> <m_launched> <m_finished> <m_aborted> <m_throughput> <m_cpu> <m_mem> <m_nodes>")
}

path_m_cost = args[1]
path_m_queued = args[2]
path_m_launched = args[3]
path_m_finished = args[4]
path_m_aborted = args[5]
path_m_throughput = args[6]
path_m_cpu = args[7]
path_m_mem = args[8]
path_m_nodes = args[9]
path_general_info = args[10]
path_queued_jobs = args[11]
path_diff_submit_start_jobs = args[12]

# Disable scientific notation and limit decimal places
options(scipen=999)
ndigits <- 3

# Read log

v_general_info <- read.csv(file=path_general_info, header=FALSE, sep=",")
queued_jobs <- as.numeric(scan(file=path_queued_jobs, sep=","))
diff_submit_start_jobs <- as.numeric(scan(file=path_diff_submit_start_jobs, sep=","))
m_cost <- read.csv(file=path_m_cost, header=TRUE, sep=",")
m_queued <- read.csv(file=path_m_queued, header=TRUE, sep=",")
m_launched <- read.csv(file=path_m_launched, header=TRUE, sep=",")
m_finished <- read.csv(file=path_m_finished, header=TRUE, sep=",")
m_aborted <- read.csv(file=path_m_aborted, header=TRUE, sep=",")
m_throughput <- read.csv(file=path_m_throughput, header=TRUE, sep=",")
m_cpu <- read.csv(file=path_m_cpu, header=TRUE, sep=",")
m_mem <- read.csv(file=path_m_mem, header=TRUE, sep=",")
m_nodes <- read.csv(file=path_m_nodes, header=TRUE, sep=",")

makespan <- as.numeric(v_general_info[1])
time_unit <- as.numeric(v_general_info[2])
time_unit_symbol <- as.character(v_general_info[[3]])
job_finish_times_length <- as.numeric(v_general_info[4])
maxnodes <- as.numeric(v_general_info[5])

# Output results

pdf(paste(args[13], sep="")) 

# Plot general information

info_table <- matrix(c(sum(m_cost[,2]), makespan, round(makespan / time_unit, digits=ndigits), round(job_finish_times_length/(makespan/(time_unit)), digits=ndigits), round(sd(queued_jobs), digits=ndigits), round(mean(queued_jobs), digits=ndigits), round(median(queued_jobs), digits=ndigits)), ncol=1, byrow=TRUE)

colnames(info_table) <- c("Value")
rownames(info_table) <- c("Total billing", "Makespan (s)", paste("Makespan (", paste(time_unit_symbol, "):")), paste("Average throughput ( jobs /", paste(time_unit_symbol, "): ")), "Standard deviation of queue time (s):", "Average queue time (s):", "Median queue time (s)")

grid.table(as.table(info_table))

print(paste("Total billing:", paste(sum(m_cost[,2]), "monetary units")))
print(paste("Makespan ( s ):", makespan))
print(paste("Makespan (", paste(time_unit_symbol, paste("):", makespan / time_unit))))
print(paste("Average throughput ( jobs /", paste(time_unit_symbol, paste("): ", job_finish_times_length/(makespan/(time_unit))))))
print(paste("Standard deviation of queue time (s):", sd(queued_jobs)))
print(paste("Average queue time (s):", mean(queued_jobs)))
print(paste("Median queue time (s):", median(queued_jobs)))


par(mfrow=c(3,2))

# Plot throughput
plot(m_throughput[,1], m_throughput[,2], type = "l", main = paste("Throughput (jobs/", paste(time_unit_symbol, ")", sep = "")),
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Jobs finished")

# Plot queued jobs
plot(m_queued[,1], m_queued[,2], ylim = c(0, max(m_queued[,2], m_launched[,2], m_finished[,2])), type = "l", main = "Job status throughout time",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Number of jobs")
legend("topleft", legend=c("Jobs queued", "Jobs launched", "Jobs finished", "Jobs aborted"),
       col=c("black", "red", "blue", "green"), lty=1:1, cex=0.8)
lines(m_launched[,1], m_launched[,2], col = "red")
lines(m_finished[,1], m_finished[,2], col = "blue")
lines(m_aborted[,1], m_aborted[,2], col = "green")

# Plot queue time histogram
hist(diff_submit_start_jobs, main = "Histogram of queueing time (m)", xlab = "Minutes spent in queue")

# Plot cost per second
plot(m_cost[,1], m_cost[,2], type = "l", main = paste("Cost throughout time ( c /", paste(time_unit_symbol, ")")),
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Cost in monetary units")

# Plot CPU and MEM usage
plot(m_cpu[,1], m_cpu[,2], ylim = c(0, 100), type = "l", main = "CPU and MEM usage (%)",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Percentage of use", col = "red")
legend("topleft", legend=c("CPU usage", "Mem usage"),
       col=c("red", "blue"), lty=1:1, cex=0.8)
lines(m_mem[,1], m_mem[,2], col = "blue")

# Plot nodes use
plot(m_nodes[,1], m_nodes[,2], ylim = c(0, maxnodes), type = "l", main = "Nodes usage (absolute)",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Nodes online", col = "red")

dev.off()
