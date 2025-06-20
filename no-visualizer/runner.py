import subprocess
import re
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def main():

    # Since bubble and selection sort are too slow, use the smaller number of elements
    # The values are the exponents of 2 for the number of elements

    # n_elements_array = [12, 14, 15, 16, 17, 18, 19, 20, 21] # Merge, Bitonic, and Heap sort
    # n_elements_array = [10, 12, 14, 15, 16] # Bubble, Selection, and Quick sort
    n_elements_array = [5, 6, 7, 8, 9, ] # Quick sort
    
    # sort_functions = ['mergeSort', 'bitonicSort', 'heapSort']
    # sort_functions = ['bubbleSort', 'selectionSort', 'quickSort']
    sort_functions = ['quickSort']

    n_iterations = 10 # Number of iterations for each number of elements for each sort
    serial_time = {}
    parallel_time = {}

    for sort in sort_functions:
        serial_time[sort] = {}
        parallel_time[sort] = {}

        for n in n_elements_array:

            serial_time[sort][n] = []
            parallel_time[sort][n] = []

            for i in range(n_iterations):

                print(f"Iteration {i + 1} with 2**{n} elements of {sort}...")
                output = subprocess.run([f"./{sort}", f'{2**n}'], capture_output=True, text=True).stdout
                values = re.findall(r'\d+', output)
                s_time = values[0]
                p_time = values[1]
                print(output)

                serial_time[sort][n].append(float(s_time)) # This holds a list of times for each n  
                parallel_time[sort][n].append(float(p_time))

    print("Serial time DataFrame")
    df_serial = pd.DataFrame(serial_time)
    print(df_serial)

    # Create a figure from the DataFrame and save as an image
    print("Parallel time DataFrame")
    df_parallel = pd.DataFrame(parallel_time)
    print(df_parallel)

    # Plot both DataFrames as lines on the same figure
    speedup = {}
    for sort in sort_functions:

        s_values = np.stack(df_serial[sort].to_numpy()) # Stacking the values to create a 2D array
        p_values = np.stack(df_parallel[sort].to_numpy())
        s_mean = np.mean(s_values, axis=1)
        p_mean = np.mean(p_values, axis=1)
        s_std = np.std(s_values, axis=1)
        p_std = np.std(p_values, axis=1)
        s_ci = 1.96 * s_std / np.sqrt(n_iterations) # 95% confidence interval
        p_ci = 1.96 * p_std / np.sqrt(n_iterations)

        speedup[sort] = s_mean / p_mean # Calculate speedup for each sort

        print(f"Serial mean for {sort}: {s_mean}")
        print(f"Parallel mean for {sort}: {p_mean}")

        plt.figure()
        plt.plot(n_elements_array, s_mean, label='Serial Time', color='r')
        plt.plot(n_elements_array, p_mean, label='Parallel Time', color='b')
        plt.fill_between(n_elements_array, p_mean - p_ci, p_mean + p_ci, color='b', alpha=0.2)
        plt.fill_between(n_elements_array, s_mean - s_ci, s_mean + s_ci, color='r', alpha=0.2)
        plt.xscale('log')
        plt.title(f"{sort.capitalize()} with 95% CI")
        plt.xlabel("Number of Elements (log scale)")
        plt.ylabel("Time (ms)")
        plt.legend(["Serial Time", "Parallel Time"])
        plt.tight_layout()
        plt.savefig(f"plots/{sort}_time.png")
        plt.close()
    
    # Save the speedup data to a CSV file
    sort_str = "_".join(sort_functions)
    pd.DataFrame(speedup, index=n_elements_array).to_csv(f"speedup_{sort_str}.csv")

if __name__ == "__main__":
    main()