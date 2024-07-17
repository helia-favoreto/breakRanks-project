#!/bin/bash python3

import os
import numpy as np
from tabulate import tabulate

# Root directory of the data
data_directory = '/home/bertuzzi/src/plexe/examples/platooning/results'

# Function to process the data and generate the collision matrix table in LaTeX format
def process_data_and_generate_latex_table(data_directory, speeds, safety_distances):
    # Initialize a matrix to store collision counts
    collision_matrix = np.zeros((len(speeds), len(safety_distances)), dtype=int)

    # Create a mapping from speed and safety distance to matrix indices
    speed_to_index = {speed: idx for idx, speed in enumerate(speeds)}
    distance_to_index = {distance: idx for idx, distance in enumerate(safety_distances)}

    # Iterate over subdirectories within the root directory
    for subdir in os.listdir(data_directory):
        subdir_path = os.path.join(data_directory, subdir)
        if os.path.isdir(subdir_path):
            try:
                # Collision counter
                total_collisions = 0

                # Read .vec files to count collisions
                for i in range(10):  # Reads all result_*.vec files
                    file_path = os.path.join(subdir_path, f'result_{i}.vec')
                    if os.path.exists(file_path):
                        collisions = read_collisions_from_vec(file_path)
                        total_collisions += collisions

                # Extract parameters from directory name
                parts = subdir.split('_')
                if len(parts) < 6:
                    print(f"Skipping directory {subdir_path} due to incorrect format")
                    continue

                speed = int(parts[-3])  # Third last element is speed
                safety_distance = int(parts[-2])  # Second last element is safety distance

                # Update the collision matrix
                if speed in speed_to_index and safety_distance in distance_to_index:
                    row_idx = speed_to_index[speed]
                    col_idx = distance_to_index[safety_distance]
                    collision_matrix[row_idx, col_idx] += total_collisions

            except Exception as e:
                print(f"Error processing directory {subdir_path}: {str(e)}")

    # Generate LaTeX table using tabulate
    headers = [""] + [f"{dist}m" for dist in safety_distances]
    table_data = [[f"{speed}km/h"] + list(map(str, collision_matrix[i])) for i, speed in enumerate(speeds)]
    latex_table = tabulate(table_data, headers, tablefmt="latex")

    # Save the LaTeX table to a file
    output_file = 'collision_matrix_table.tex'
    with open(os.path.join(data_directory, output_file), 'w') as f:
        f.write(latex_table)
    print(f"Collision matrix table generated at '{output_file}'.")

# Function to read the number of collisions from a .vec file
def read_collisions_from_vec(file_path):
    collisions = 0
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('Collision'):
                try:
                    collisions += int(line.split()[-1])
                except ValueError:
                    continue  # Ignore lines that cannot be converted to int
    return collisions

# Define the speeds and safety distances to include in the matrix
speeds = [110, 130, 150]
safety_distances = [2, 3, 5, 10, 15, 20]

# Call the main function to process the data and generate the LaTeX table
process_data_and_generate_latex_table(data_directory, speeds, safety_distances)




