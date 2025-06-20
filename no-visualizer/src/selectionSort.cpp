#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <omp.h>

void selectionSortParallel(int* arr, int arrSize)
/*
In selection sort you take the smallest element of the remaining
array and put it up next to where you know it's sorted.

Because of this, the swap is still not parallelizable.

However, to choose the smallest element you need comparisons which
are parallelizable, this code aims to explore this.

*/
{
    for(int i=0;i<arrSize-1;i++)
    {
        int minIndex = i;

        #pragma omp parallel
        {
            int localMinIndex = minIndex;

            #pragma omp for nowait
            for(int j=i+1;j<arrSize;j++)
            {
                if(arr[j]<arr[localMinIndex])
                {
                    localMinIndex=j;
                }
            }
            #pragma omp critical
            if (arr[localMinIndex] < arr[minIndex]){
                minIndex = localMinIndex;
            }
            
        }
        
        int temp=arr[i];
        arr[i]=arr[minIndex];
        arr[minIndex]=temp;
    }
}

void selectionSort(int* arr, int arrSize)
{
    int minIndex;
    for(int i=0;i<arrSize-1;i++)
    {
        minIndex=i;
        for(int j=i+1;j<arrSize;j++)
        {
            if(arr[j]<arr[minIndex])
            {
                minIndex=j;
            }
        }
        int temp=arr[i];
        arr[i]=arr[minIndex];
        arr[minIndex]=temp;
    }
}

void fillRandom(int* arr, int size, std::mt19937& gen, std::uniform_int_distribution<>& dist) {
    for (int i = 0; i < size; ++i) {
        arr[i] = dist(gen);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <array_size>\n";
        return 1;
    }

    int n = std::atoi(argv[1]);
    

    int* arr = new int[n];
    int* arrCopy = new int[n];

    std::mt19937 gen(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_int_distribution<> dist(0, 1000000);

    // Fill array with random values
    fillRandom(arr, n, gen, dist);
    std::copy(arr, arr + n, arrCopy);

    auto startA = std::chrono::high_resolution_clock::now();
    selectionSort(arr, n);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    selectionSortParallel(arr, n);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Selection Sort time: " << durationA << " ms\n";
    std::cout << "Selection Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
