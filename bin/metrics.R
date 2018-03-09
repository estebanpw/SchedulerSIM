# Plots different metrics from a scheduling log file

#!/usr/bin/env Rscript
args = commandArgs(trailingOnly=TRUE)

if(length(args) < 1){
  stop("USE: Rscript --vanilla metrics.R <logfile>")
}

path_mat = args[1]

# Read log

makespan <- 0

con = file(path_mat, "r")
while ( TRUE ) {
  line = readLines(con, n = 1)
  if ( length(line) == 0 ) {
    break
  } else {
    # Store info here
    
    # Copy [JOB FINISHED]
    if( grepl("JOB FINISHED", line) == TRUE){
      print(strsplit(line, "\\$")[[1]][2])
    }
    
    # Copy [SYSTEM OFF] (t=1505485) (CL=7527426)
    if( grepl("SYSTEM OFF", line) == TRUE){
      makespan <- as.numeric(strsplit(line, "\\$")[[1]][2])
    }
    
    
  }
}
close(con)