# Load necessary libraries
library(tidyverse)

# Function to process each directory
process_directory <- function(directory) {
  cat("Processing directory:", directory, "\n")
  
  # Read all .vec files in the directory
  vec_files <- list.files(path = directory, pattern = "\\.vec$", full.names = TRUE)
  
  # Initialize an empty list to store data frames
  data_list <- list()
  
  # Read each .vec file into a data frame
  for (file in vec_files) {
    cat("Reading file:", file, "\n")
    tryCatch({
      # Open the file connection
      con <- file(file, "r")
      
      # Initialize empty vectors to store data
      timestamps <- numeric()
      values <- numeric()
      
      # Read each line of the file
      while (TRUE) {
        line <- readLines(con, n = 1)
        if (length(line) == 0) {
          break
        }
        
        # Split line by tab delimiter (assuming it's tab-separated)
        parts <- strsplit(line, "\t")[[1]]
        
        # Check if the line has at least 3 parts (timestamp, value1, value2)
        if (length(parts) >= 3) {
          # Extract and convert data
          timestamp <- as.numeric(parts[1])
          value1 <- as.numeric(parts[2])
          value2 <- as.numeric(parts[3])
          
          # Store in vectors
          timestamps <- c(timestamps, timestamp)
          values <- c(values, value1)
          
          # Optionally, store value2 as well if needed
          # values2 <- c(values2, value2)
        }
      }
      
      # Close the file connection
      close(con)
      
      # Combine into a data frame
      data <- tibble(timestamp = timestamps, value1 = values)
      
      # Add a column for simulation index if needed
      data$simulation <- basename(file)
      
      # Store data frame in the list
      data_list[[basename(file)]] <- data
    }, error = function(e) {
      cat("Error reading file:", file, "- Skipping...\n")
    })
  }
  
  # Check if data_list is not empty
  if (length(data_list) == 0) {
    cat("No valid .vec files found in directory:", directory, "\n")
    return(NULL)
  }
  
  # Combine all data frames into one
  combined_data <- bind_rows(data_list, .id = "simulation")
  
  # Convert timestamp to numeric (assuming it's in seconds or milliseconds)
  combined_data$timestamp <- as.numeric(combined_data$timestamp)
  
  # Plotting using ggplot
  cat("Generating plots for directory:", directory, "\n")
  
  # Example plot: change according to your variables and plot requirements
  plot <- ggplot(combined_data, aes(x = timestamp, y = value1, color = simulation)) +
            geom_line() +
            labs(title = "Plot Title", x = "Time (seconds)", y = "Value 1") +
            theme_minimal()  # Adjust theme as needed
  
  # Save the plot to a file
  output_dir <- "./plots"  # Specify your output directory
  if (!file.exists(output_dir)) dir.create(output_dir)
  
  plot_filename <- file.path(output_dir, paste0("plot_", basename(directory), ".png"))
  ggsave(filename = plot_filename, plot = plot, width = 7, height = 7)
  
  cat("Plot saved:", plot_filename, "\n")
}

# Main script
# Specify the base directory where subdirectories (BreakRanks_1_0.1_*_*_*) are located
base_directory <- "/home/bertuzzi/src/plexe/examples/platooning/results"

# List all subdirectories matching the pattern
subdirectories <- list.dirs(base_directory, recursive = FALSE, full.names = TRUE)

# Process each subdirectory
for (dir in subdirectories) {
  process_directory(dir)
}
