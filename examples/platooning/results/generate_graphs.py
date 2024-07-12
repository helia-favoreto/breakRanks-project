#!/bin/bash python3

import os
import matplotlib.pyplot as plt
import numpy as np

# Root directory of the data
data_directory = '/home/bertuzzi/src/plexe/examples/platooning/results'

# Function to process the data and generate plots
def process_data_and_generate_plots(data_directory):
    # Iterate over subdirectories within the root directory
    for subdir in os.listdir(data_directory):
        subdir_path = os.path.join(data_directory, subdir)
        if os.path.isdir(subdir_path) and subdir.startswith("BreakRanks_1_0.1_"):
            try:
                # Initialize lists to store data
                speeds = []
                accelerations = []
                distances = []

                # Read .vec files
                for i in range(10):  # Assuming there are 10 result_*.vec files
                    file_path = os.path.join(subdir_path, f'result_{i}.vec')
                    if os.path.exists(file_path):
                        speed, acceleration, distance = read_data_from_vec(file_path)
                        speeds.extend(speed)
                        accelerations.extend(acceleration)
                        distances.extend(distance)

                # Generate plots
                generate_plots(subdir, speeds, accelerations, distances)

            except Exception as e:
                print(f"Error processing directory {subdir_path}: {str(e)}")

def read_data_from_vec(file_path):
    speeds = []
    accelerations = []
    distances = []

    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('vector'):
                parts = line.split()
                if len(parts) >= 4:
                    if parts[3] == 'speed':
                        speeds.append(float(parts[4]))
                    elif parts[3] == 'acceleration':
                        accelerations.append(float(parts[4]))
                    elif parts[3] == 'distance':
                        distances.append(float(parts[4]))

    return speeds, accelerations, distances

def generate_plots(subdir, speeds, accelerations, distances):
    # Generate plots for speed, acceleration, and distance
    fig, axs = plt.subplots(3, 1, figsize=(10, 12))

    axs[0].plot(speeds, label='Speed')
    axs[0].set_title('Speed Plot')
    axs[0].set_xlabel('Time')
    axs[0].set_ylabel('Speed (m/s)')
    axs[0].legend()

    axs[1].plot(accelerations, label='Acceleration')
    axs[1].set_title('Acceleration Plot')
    axs[1].set_xlabel('Time')
    axs[1].set_ylabel('Acceleration (m/s^2)')
    axs[1].legend()

    axs[2].plot(distances, label='Distance')
    axs[2].set_title('Distance Plot')
    axs[2].set_xlabel('Time')
    axs[2].set_ylabel('Distance (m)')
    axs[2].legend()

    plt.tight_layout()
    plt.savefig(os.path.join(data_directory, f"{subdir}_plots.png"))
    plt.close()
    print(f"Plots generated for {subdir}")

# Call the function to process data and generate plots
process_data_and_generate_plots(data_directory)
