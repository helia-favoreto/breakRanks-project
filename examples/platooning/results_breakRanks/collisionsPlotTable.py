#!/bin/bash python3

import os
import matplotlib.pyplot as plt
from matplotlib.table import Table

# Root directory of the data
data_directory = '/home/bertuzzi/src/plexe/examples/platooning/results_breakRanks'

# Function to process the data and generate the collision table as an image
def process_data_and_generate_table_image(data_directory):
    # List to store results from each directory
    results = []
    #
    # Iterate over subdirectories within the root directory
    for subdir in os.listdir(data_directory):
        subdir_path = os.path.join(data_directory, subdir)
        if os.path.isdir(subdir_path):
            try:
                # Collision counter
                total_collisions = 0

                # Read .vec files to count collisions
                for i in range(10):  # Reads all result_*.vec files
                    collisions = read_collisions_from_vec(os.path.join(subdir_path, f'result_{i}.vec'))
                    total_collisions += collisions

                # Extract speed and safetyDistance parameters from directory name
                parts = subdir.split('_')
                speed = int(parts[-2])  # Penultimate element is speed
                safety_distance = int(parts[-1])  # Last element is safetyDistance

                # Add results from this directory to the list
                results.append({
                    'LeaderSpeed': speed,
                    'SafetyDistance': safety_distance,
                    'TotalCollisions': total_collisions
                })

            except Exception as e:
                print(f"Error processing directory {subdir_path}: {str(e)}")

    # Sort results by safety distance in ascending order
    results_sorted = sorted(results, key=lambda x: x['SafetyDistance'])

    # Create a matplotlib Figure object
    fig, ax = plt.subplots()

    # Table configuration
    col_labels = ['leaderSpeed (km/h)', 'safetyDistance (m)', 'Total Collisions']
    table_data = [[f"{row['LeaderSpeed']}", f"{row['SafetyDistance']}", row['TotalCollisions']] for row in results_sorted]

    # Create the table and configure cell styles
    table = ax.table(cellText=table_data, colLabels=col_labels, loc='center', cellLoc='center', colColours=['#f2f2f2']*3)
    table.auto_set_font_size(False)
    table.set_fontsize(12)
    table.scale(1.2, 1.2)  # Adjust table size

    # Hide axes
    ax.axis('off')

    # Adjust figure layout
    fig.tight_layout()

    # Save the table image
    output_file = 'collision_detection_table.png'
    plt.savefig(os.path.join(data_directory, output_file))
    print(f"Collision detection table generated at '{output_file}'.")

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

# Call the main function to process the data and generate the table as an image
process_data_and_generate_table_image(data_directory)

