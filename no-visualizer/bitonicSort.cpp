#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <omp.h>

void bitonicMerge(int arr[], int low, int count, bool dir)
{
    if (count > 1)
    {
        int k = count / 2;
        for (int i = low; i < low + k; i++)
        {
            if ((dir && arr[i] > arr[i + k]) || (!dir && arr[i] < arr[i + k]))
            {
                int temp = arr[i];
                arr[i] = arr[i + k];
                arr[i + k] = temp;
            }
        }
        bitonicMerge(arr, low, k, dir);
        bitonicMerge(arr, low + k, k, dir);
    }
}

void bitonicSortRec(int a[], int low, int count, bool dir)
{
    if (count > 1)
    {
        int k = count / 2;

        bitonicSortRec(a, low, k, true);
        bitonicSortRec(a, low + k, k, false);

        bitonicMerge(a, low, count, dir);
    }
}

void bitonicSort(int a[], int count, bool dir)
{
    bitonicSortRec(a, 0, count, true);
}

void bitonicMergeParallel(int a[], int low, int count, bool dir)
{
    if(count > 1)
    {
        int k = count / 2;
        for(int i = low; i < low + k; i++)
        {
            if((dir && a[i] > a[i + k]) || (!dir && a[i] < a[i + k]))
            {
                int temp = a[i];
                a[i] = a[i + k];
                a[i + k] = temp;
            }
        }
        bitonicMergeParallel(a, low, k, dir);
        bitonicMergeParallel(a, low + k, k, dir);
    }
}

void bitonicSortParallelHelper(int a[], int low, int count, bool dir)
{
    if(count > 1)
    {
        int k = count / 2;
        const int THRESHOLD = 5000; 
        //using a threshold to limit the creation of small tasks
        if((count - low) > THRESHOLD){
           #pragma omp task shared(a)
            bitonicSortParallelHelper(a, low, k, true);

            #pragma omp task shared(a)
            bitonicSortParallelHelper(a, low + k, k, false);

            #pragma omp taskwait 
        }else{

            bitonicSortParallelHelper(a, low, k, true);
            bitonicSortParallelHelper(a, low + k, k, false);
        }
        bitonicMergeParallel(a, low, count, dir);
    }
}

void bitonicSortParallel(int a[], int size)
{
    #pragma omp parallel
    {
        #pragma omp single
        {
            bitonicSortParallelHelper(a, 0, size, true);
        }
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
    bitonicSort(arr, n, true);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    bitonicSortParallel(arr, n);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Bitonic Sort time: " << durationA << " ms\n";
    std::cout << "Bitonic Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
