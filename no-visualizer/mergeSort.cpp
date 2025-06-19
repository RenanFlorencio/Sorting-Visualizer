#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <omp.h>

void merge2SortedArrays(int a[], int si, int ei)
{
    int size_output=(ei-si)+1;
    int* output=new int[size_output];

    int mid=(si+ei)/2;
    int i=si, j=mid+1, k=0;
    while(i<=mid && j<=ei)
    {
        if(a[i]<=a[j])
        {
            output[k]=a[i];
            i++;
            k++;
        }
        else
        {
            output[k]=a[j];
            j++;
            k++;
        }

    }
    while(i<=mid)
    {
        output[k]=a[i];
        i++;
        k++;
    }
    while(j<=ei)
    {
        output[k]=a[j];
        j++;
        k++;
    }
    int x=0;
    for(int l=si; l<=ei; l++)
    {
        a[l]=output[x];
        x++;
    }
    delete []output;
}

void mergeSort(int a[], int si, int ei)
{
    if(si>=ei)
    {
        return;
    }
    int mid=(si+ei)/2;

    mergeSort(a, si, mid);
    mergeSort(a, mid+1, ei);

    merge2SortedArrays(a, si, ei);
}

void merge2SortedArraysParallel(int a[], int si, int ei)
{
    int size_output=(ei-si)+1;
    int* output=new int[size_output];

    int mid=(si+ei)/2;
    int i=si, j=mid+1, k=0;
    while(i<=mid && j<=ei)
    {
        if(a[i]<=a[j])
        {
            output[k]=a[i];
            i++;
            k++;
        }
        else
        {
            output[k]=a[j];
            j++;
            k++;
        }

    }
    while(i<=mid)
    {
        output[k]=a[i];
        i++;
        k++;
    }
    while(j<=ei)
    {
        output[k]=a[j];
        j++;
        k++;
    }
    int x=0;
    for(int l=si; l<=ei; l++)
    {
        a[l]=output[x];
        x++;
    }
    delete []output;
}

void mergeSortParallelHelper(int a[], int si, int ei)
{
    if(si>=ei)
    {
        return;
    }
    int mid=(si+ei)/2;
    const int THRESHOLD = 5000; 
    //using a threshold to limit the creation of small tasks
    if ((ei - si) > THRESHOLD) {
        #pragma omp task shared(a)
        mergeSortParallelHelper(a, si, mid);

        #pragma omp task shared(a)
        mergeSortParallelHelper(a, mid+1, ei);

        #pragma omp taskwait
    } else {
        mergeSortParallelHelper(a, si, mid);
        mergeSortParallelHelper(a, mid + 1, ei);
    }
    merge2SortedArraysParallel(a, si, ei);

}

void mergeSortParallel(int a[], int si, int ei)
{
    #pragma omp parallel
    {
        #pragma omp single
        {
            mergeSortParallelHelper(a, si, ei);
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
    mergeSort(arr, 0, n);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    mergeSortParallel(arr, 0, n);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Merge Sort time: " << durationA << " ms\n";
    std::cout << "Merge Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
