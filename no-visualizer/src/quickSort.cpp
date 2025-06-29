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

/* This parallel Quick Sort algorithm implements a "divide and conquer" strategy, using OpenMP tasks for parallel execution.

The main idea is:
1.  Partition Array: 

    In this subroutine, a pivot element is chosen (in this case, the first element of the sub-array),
    and the array is reordered so that all elements less than or equal to the pivot come before it,
    and all elements greater than the pivot come after it. This operation defines the pivot's
    final sorted position.

2.  Recursive Sorting with Parallel Tasks: 

    Quick Sort algorithm then recursively sorts the sub-arrays to the left and right of the pivot.
    For sub-arrays larger than a defined threshold (e.g., 10,000 elements), OpenMP tasks are
    used to execute these recursive calls in parallel. This allows different parts of the array
    to be sorted concurrently by available threads.
    Smaller sub-arrays are sorted sequentially to avoid overhead of task creation. */

int partitionArray(int a[], int si, int ei)
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

    int c=partitionArray(a, si, ei);
    quickSort(a, si, c-1);
    quickSort(a, c+1, ei);

}

int partitionArrayParallel(int a[], int si, int ei)
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


void quickSortParallel(int a[], int si, int ei)
{
    if (si >= ei)
        return;

    int c = partitionArrayParallel(a, si, ei);

    // Definir limite mínimo de tamanho para evitar overhead com tarefas pequenas
    int size = ei - si;
    if (size > 10000) {
        #pragma omp task shared(a)
        quickSortParallel(a, si, c - 1);

        #pragma omp task shared(a)
        quickSortParallel(a, c + 1, ei);

        #pragma omp taskwait  // Espera ambas as tarefas terminarem
    } else {
        quickSortParallel(a, si, c - 1);
        quickSortParallel(a, c + 1, ei);
    }
}

void quickSortParallelEntry(int a[], int si, int ei)
{
    #pragma omp parallel
    {
        #pragma omp single
        {
            quickSortParallel(a, si, ei);
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
    quickSort(arr, 0, n-1);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    quickSortParallelEntry(arr, 0, n-1);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Quick Sort time: " << durationA << " ms\n";
    std::cout << "Quick Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
