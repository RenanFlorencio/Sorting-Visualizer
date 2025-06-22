#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <omp.h>
#include <limits>
#include <set>
#include <thread>
#include <mutex>
#include <vector>

int partition_array(int a[], int si, int ei)
{
    int count_small=0;

    for(int i=(si+1);i<=ei;i++)
    {
        if(a[i]<=a[si])
        {
            count_small++;
        }
    }
    int c=si+count_small;
    int temp=a[c];
    a[c]=a[si];
    a[si]=temp;

    int i=si, j=ei;

    while(i<c && j>c)
    {
        if(a[i]<= a[c])
        {
            i++;
        }
        else if(a[j]>a[c])
        {
            j--;
        }
        else
        {
            int temp_1=a[j];
            a[j]=a[i];
            a[i]=temp_1;

            i++;
            j--;
        }
    }
    return c;
}

void quickSort(int a[], int si, int ei)
{
    if(si>=ei)
    {
        return;
    }

    int c=partition_array(a, si, ei);
    quickSort(a, si, c-1);
    quickSort(a, c+1, ei);

}

int partition_array_parallel(int a[], int si, int ei)
{
    int pivot = a[si];
    int count_small = 0;

    // Flags para marcar quem é menor que o pivô
    std::vector<int> isSmaller(ei - si, 0);

    // Comparações paralelas
    #pragma omp parallel for reduction(+:count_small)
    for (int i = si + 1; i <= ei; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (a[i] <= pivot) {
            isSmaller[i - (si + 1)] = 1;
            count_small++;
        }
    }

    int c = si + count_small;

    // Troca do pivô para a posição correta
    std::swap(a[c], a[si]);
    std::this_thread::sleep_for(std::chrono::milliseconds(70));

    // Rearranjar os elementos à esquerda e direita do pivô (sequencial)
    int i = si, j = ei;

    while (i < c && j > c)
    {
        if (a[i] <= a[c]) {
            i++;
        }
        else if (a[j] > a[c]) {
            j--;
        }
        else {
            std::swap(a[i], a[j]);
            std::this_thread::sleep_for(std::chrono::milliseconds(70));
            i++;
            j--;
        }
    }

    return c;
}


void quickSortParallel(int a[], int si, int ei, int depth)
/*
Quick Sort selects a pivot and partitions the array such that all elements 
less than or equal to the pivot go to its left, and greater elements go to 
its right. It then recursively applies the same process to the two subarrays.

This parallel version optimizes the partitioning step: comparisons with 
the pivot are done in parallel using a helper array (`isSmaller`) and 
OpenMP reduction to count how many elements are smaller than the pivot. 
This part is highly parallelizable and benefits from multiple threads.

However, rearranging elements around the pivot (swapping from both ends) 
remains sequential due to data dependencies between indices.

Recursive calls to Quick Sort are parallelized using `#pragma omp task`, 
limited by a depth counter (`depth`) to avoid excessive thread spawning 
for small subarrays. While `depth > 0`, subproblems are run as tasks.

All visualizations are enclosed in critical sections to ensure consistent 
highlighting of indices and proper synchronization of animations.
*/

{
    if (si >= ei)
        return;

    int c = partition_array_parallel(a, si, ei);

    if (depth > 0) {
        #pragma omp task shared(a)
        quickSortParallel(a, si, c - 1, depth - 1);

        #pragma omp task shared(a)
        quickSortParallel(a, c + 1, ei, depth - 1);
    } else {
        quickSortParallel(a, si, c - 1, 0);
        quickSortParallel(a, c + 1, ei, 0);
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
    quickSort(arr, 0, n-1);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    quickSortParallel(arr, 0, n-1, 16);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Quick Sort time: " << durationA << " ms\n";
    std::cout << "Quick Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
