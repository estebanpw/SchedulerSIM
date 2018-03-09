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

# Function to scale jobs into time units
scaled_throughput <- function(job_finish_times, makespan, time_unit){
  
  m <- matrix(0, nrow = makespan/time_unit, ncol = 2)
  m[,1] <- c(1:(makespan/time_unit))
  m[,2] <- 0
  
  for(j in job_finish_times){
    c <- job_finish_times/time_unit
    m[c,2] <- m[c,2] + 1
  }
  
  return (m)
}

# Function to scale jobs into time units
scaled_stat_per_time <- function(stat_object, time_object, makespan, time_unit){
  
  m <- matrix(0, nrow = makespan/time_unit, ncol = 2)
  m[,1] <- c(1:(makespan/time_unit))
  m[,2] <- 0
  
  for(i in 1:length(stat_object)){
    c <- time_object[i]/time_unit
    m[c,2] <- m[c,2] + stat_object[i]
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
job_finish_times <- c()
queued_jobs <- c()
launched_jobs <- c()
finished_jobs <- c()
jobs_times <- c()
cost_per_second <- c()
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
      diff_submit_start_jobs <- c(diff_submit_start_jobs, (start - submit)/60 )
    }
    
    # Copy [SYS LOAD]
    if( grepl("SYS LOAD", line) == TRUE){
      jobs_times <- c(jobs_times, as.numeric((strsplit(line, "\\$"))[[1]][2]))
      queued_jobs <- c(queued_jobs, as.numeric((strsplit(line, "\\$"))[[1]][4]))
      launched_jobs <- c(launched_jobs, as.numeric((strsplit(line, "\\$"))[[1]][6]))
      finished_jobs <- c(finished_jobs, as.numeric((strsplit(line, "\\$"))[[1]][8]))
      cost_per_second <- c(cost_per_second, as.numeric(strsplit(line, "\\s|\\$")[[1]][27]))
    }
    
    # Copy [SYSTEM OFF] (t=1505485) (CL=7527426)
    if( grepl("SYSTEM OFF", line) == TRUE){
      makespan <- as.numeric(strsplit(line, "\\$")[[1]][2])
    }
    
  }
}
close(con)

# Output results

time_unit <- 60*60

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

print(paste("Total billig:", paste(sum(m_cost[,2]), "monetary units")))
print(paste("Makespan ( s ):", makespan))
print(paste("Makespan (", paste(time_unit_symbol, paste("):", makespan / time_unit))))
print(paste("Average throughput ( jobs /", paste(time_unit_symbol, paste("): ", length(job_finish_times)/(makespan/(time_unit))))))

