# Plots different metrics from a scheduling log file

#!/usr/bin/env Rscript
args = commandArgs(trailingOnly=TRUE)

if(length(args) < 2){
  stop("USE: Rscript --vanilla metrics.R <logfile> <timeunit>")
}

path_mat = args[1]
time_unit_symbol = args[2]
time_unit <- 1
if(time_unit_symbol == "s") time_unit <- 1
if(time_unit_symbol == "m") time_unit <- 60
if(time_unit_symbol == "h") time_unit <- 60*60
if(time_unit_symbol == "d") time_unit <- 60*60*24
if(time_unit_symbol == "w") time_unit <- 60*60*24*7
if(time_unit_symbol == "M") time_unit <- 60*60*24*7*4

# Function to scale jobs into time units
scaled_throughput <- function(job_finish_times, makespan, time_unit){
  
  m <- matrix(0, nrow = makespan/time_unit, ncol = 2)
  m[,1] <- c(1:(makespan/time_unit))
  m[,2] <- 0
  
  for(j in job_finish_times){
    k <- j/time_unit
    m[k,2] <- m[k,2] + 1
  }
  
  return (m)
}

# Function to scale jobs into time units
scaled_stat_per_time <- function(stat_object, time_object, makespan, time_unit){
  
  m <- matrix(0, nrow = makespan/time_unit, ncol = 2)
  m[,1] <- c(1:(makespan/time_unit))
  m[,2] <- 0
  
  for(i in 1:length(stat_object)){
    k <- time_object[i]/time_unit
    m[k,2] <- m[k,2] + stat_object[i]
  }
  
  return (m)
}

fill_scaled_stat_per_time <- function(stat_object, time_object, makespan, time_unit){
  
  m <- matrix(0, nrow = makespan/time_unit, ncol = 2)
  m[,1] <- c(1:(makespan/time_unit))
  m[,2] <- 0
  
  prev <- 1
  for(i in 1:length(stat_object)){
    c <- time_object[i]/time_unit
    for(j in prev:c){
      m[j,2] <- stat_object[i]
    }
    prev <- c
  }
  return (m)
}

# Read log

makespan <- 0
maxnodes <- 0
job_finish_times <- c()
queued_jobs <- c()
launched_jobs <- c()
finished_jobs <- c()
jobs_times <- c()
cost_per_second <- c()
cpu_usage <- c()
mem_usage <- c()
nodes_usage <- c()
diff_submit_start_jobs <- c()

con = file(path_mat, "r")
while ( TRUE ) {
  line = readLines(con, n = 1)
  if ( length(line) == 0 ) {
    break
  } else {
    # Store info here
    
    # Copy [JOB FINISHED]
    if( grepl("JOB FINISHED", line) == TRUE){
      job_finish_times <- c(job_finish_times, as.numeric(strsplit(line, "\\$")[[1]][2]))
    }
    # Copy [JOB LAUNCHED]
    if( grepl("JOB LAUNCHED", line) == TRUE){
      submit <- as.numeric(strsplit(line, ":|\\s")[[1]][11])
      start <- as.numeric(strsplit(line, ":|\\s")[[1]][13])
      diff_submit_start_jobs <- c(diff_submit_start_jobs, (start - submit)/60 ) # This one is always a histogram in minutes
    }
    
    # Copy [SYS LOAD]
    if( grepl("SYS LOAD", line) == TRUE){
      ss_just_dollar <- strsplit(line, "\\$")
      ss__dollar_and_space <- strsplit(line, "\\s|\\$")
      jobs_times <- c(jobs_times, as.numeric(ss_just_dollar[[1]][2]))
      queued_jobs <- c(queued_jobs, as.numeric(ss_just_dollar[[1]][4]))
      launched_jobs <- c(launched_jobs, as.numeric(ss_just_dollar[[1]][6]))
      finished_jobs <- c(finished_jobs, as.numeric(ss_just_dollar[[1]][8]))
      cost_per_second <- c(cost_per_second, as.numeric(ss__dollar_and_space[[1]][27]))
      
      cpu_usage <- c(cpu_usage, as.numeric(strsplit(ss__dollar_and_space[[1]][21], "%")[[1]][1]))
      mem_usage <- c(mem_usage, as.numeric(strsplit(ss__dollar_and_space[[1]][24], "%")[[1]][1]))
      nodes_usage <- c(nodes_usage, as.numeric(strsplit(ss__dollar_and_space[[1]][19], "/")[[1]][1]))
      maxnodes <- as.numeric(strsplit(ss__dollar_and_space[[1]][19], "/")[[1]][2])
      
    }
    
    # Copy [SYSTEM OFF] (t=$1505485$) (CL=$7527426$)
    if( grepl("SYSTEM OFF", line) == TRUE){
      makespan <- as.numeric(strsplit(line, "\\$")[[1]][2])
    }
    
  }
}
close(con)

# Output results

pdf(paste(path_mat, ".pdf", sep="")) 
par(mfrow=c(3,2))

#png(paste(path_mat, ".png", sep=""), width = length(data[,1]), height = length(data[,1]))

# Plot throughput
m_throughput <- scaled_throughput(job_finish_times, makespan, time_unit)
plot(m_throughput[,1], m_throughput[,2], type = "l", main = paste("Throughput (jobs/", paste(time_unit_symbol, ")", sep = "")),
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Jobs finished")

# Plot queued jobs
m_queued <- scaled_stat_per_time(queued_jobs, jobs_times, makespan, time_unit)
m_launched <- scaled_stat_per_time(launched_jobs, jobs_times, makespan, time_unit)
m_finished <- scaled_stat_per_time(finished_jobs, jobs_times, makespan, time_unit)

plot(m_queued[,1], m_queued[,2], ylim = c(0, max(m_queued[,2], m_launched[,2], m_finished[,2])), type = "l", main = "Job status throughout time",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Number of jobs")
legend("topleft", legend=c("Jobs queued", "Jobs launched", "Jobs finished"),
       col=c("black", "red", "blue"), lty=1:1, cex=0.8)
lines(m_launched[,1], m_launched[,2], col = "red")
lines(m_finished[,1], m_finished[,2], col = "blue")

# Plot queue time histogram
hist(diff_submit_start_jobs, main = "Histogram of queueing time (m)", xlab = "Minutes spent in queue")

# Plot cost per second
m_cost <- fill_scaled_stat_per_time(cost_per_second, jobs_times, makespan, time_unit)
plot(m_cost[,1], m_cost[,2], type = "l", main = paste("Cost throughout time ( c /", paste(time_unit_symbol, ")")),
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Cost in monetary units")

# Plot CPU and MEM usage
m_cpu <- fill_scaled_stat_per_time(cpu_usage, jobs_times, makespan, time_unit)
m_mem <- fill_scaled_stat_per_time(mem_usage, jobs_times, makespan, time_unit)
plot(m_cpu[,1], m_cpu[,2], ylim = c(0, 100), type = "l", main = "CPU and MEM usage (%)",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Percentage of use", col = "red")
legend("topleft", legend=c("CPU usage", "Mem usage"),
       col=c("red", "blue"), lty=1:1, cex=0.8)
lines(m_mem[,1], m_mem[,2], col = "blue")

# Plot nodes use
m_nodes <- fill_scaled_stat_per_time(nodes_usage, jobs_times, makespan, time_unit)
plot(m_nodes[,1], m_nodes[,2], ylim = c(0, maxnodes), type = "l", main = "Nodes usage (absolute)",
     xlab = paste("Time unit (", paste(time_unit_symbol, ")", sep = ""), sep = ""),
     ylab = "Nodes online", col = "red")



# Plot general information
print(paste("Total billing:", paste(sum(m_cost[,2]), "monetary units")))
print(paste("Makespan ( s ):", makespan))
print(paste("Makespan (", paste(time_unit_symbol, paste("):", makespan / time_unit))))
print(paste("Average throughput ( jobs /", paste(time_unit_symbol, paste("): ", length(job_finish_times)/(makespan/(time_unit))))))

dev.off()

