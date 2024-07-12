#!/bin/bash python3

import os
import matplotlib.pyplot as plt
import numpy as np

# Root directory of the data
data_directory = '/home/bertuzzi/src/plexe/examples/platooning/results'

# Function to process the data and generate the collision matrix table as an image
def process_data_and_generate_matrix_table_image(data_directory, speeds, safety_distances):
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

    # Create a matplotlib Figure object
    fig, ax = plt.subplots(figsize=(12, 8))  # Increase the figure width

    # Create the table
    ax.axis('tight')
    ax.axis('off')
    col_labels = [f"{dist} m" for dist in safety_distances]
    row_labels = [f"{speed} km/h" for speed in speeds]
    table = ax.table(cellText=collision_matrix, rowLabels=row_labels, colLabels=col_labels, loc='center', cellLoc='center', colColours=['#f2f2f2'] * len(safety_distances))

    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1.2, 1.2)  # Increase table scaling

    # Adjust column widths
    col_width = 1.2 / (len(safety_distances) + 1)  # Include an extra column for the row labels

    for (i, j), cell in table.get_celld().items():
        if j == -1:  # This is the row label (speed column)
            cell.set_width(0.1)  # Adjust width of the speed column
        else:
            cell.set_width(col_width)  # Adjust width of other columns

    # Save the table image
    output_file = 'collision_matrix_table.png'
    plt.savefig(os.path.join(data_directory, output_file), bbox_inches='tight', pad_inches=0.1)
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

# Call the main function to process the data and generate the matrix table as an image
process_data_and_generate_matrix_table_image(data_directory, speeds, safety_distances)
