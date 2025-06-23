#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <omp.h>
#include <queue>    
#include <functional>

/* This parallel Heap Sort algorithm uses a "divide and conquer" strategy. It does not keep all the logic of the 
original Heap Sort algorithm, but we adapted it so it can be done in parallel, with a reasonable speedup.7

Parallelization is primarily applied in two phases:

1.  Parallel Heap Construction: The input array is divided into 'num_threads' sub-arrays (we chose 2).
    Each thread concurrently constructs an independent max-heap within its respective chunk.
    In this phase, the independence of the chunks allows for efficient parallelism.

2.  Merging: After the parallel heap construction, the elements are merged
    into a single, sorted final array. This merge is performed by a priority queue (min-heap)
    which sequentially extracts the smallest element from the "heads" of all sub-heaps.
    While the merging itself is sequential in this implementation, using a priority queue
    optimizes the combination process for the sub-heaps. */

void inplaceHeapSort(int* input, int n)
{
    for(int i=1; i<n; i++)
    {
       int childIndex=i;
       int parentIndex=(childIndex-1)/2;

       while(childIndex>0)
       {
           if(input[childIndex]>input[parentIndex])
           {
               int temp=input[parentIndex];
               input[parentIndex]=input[childIndex];
               input[childIndex]=temp;

           }
           else
           {
               break;
           }

           childIndex=parentIndex;
           parentIndex=(childIndex-1)/2;
       }
    }

    for(int heapLast=n-1; heapLast>=0 ; heapLast--)
    {
        int temp=input[0];
        input[0]=input[heapLast];
        input[heapLast]=temp;

        int parentIndex=0;
        int leftChildIndex=2*parentIndex + 1;
        int rightChildIndex=2*parentIndex + 2;

        while(leftChildIndex<heapLast)
        {
            int maxIndex=parentIndex;

            if(input[leftChildIndex]>input[maxIndex])
            {
                maxIndex=leftChildIndex;
            }
            if(rightChildIndex<heapLast && input[rightChildIndex]>input[maxIndex])
            {
                maxIndex=rightChildIndex;
            }
            if(maxIndex==parentIndex)
            {
                break;
            }

            int temp=input[parentIndex];
            input[parentIndex]=input[maxIndex];
            input[maxIndex]=temp;

            parentIndex=maxIndex;
            leftChildIndex=2*parentIndex + 1;
            rightChildIndex=2*parentIndex + 2;
        }
    }
}


void heapifyDown(int* arr, int n, int i, int heapLimit) {
    int parentIndex = i;
    int leftChildIndex = 2 * parentIndex + 1;
    int rightChildIndex = 2 * parentIndex + 2;

    while (leftChildIndex < heapLimit) {
        int maxIndex = parentIndex;

        if (arr[leftChildIndex] > arr[maxIndex]) {
            maxIndex = leftChildIndex;
        }
        if (rightChildIndex < heapLimit && arr[rightChildIndex] > arr[maxIndex]) {
            maxIndex = rightChildIndex;
        }

        if (maxIndex == parentIndex) {
            break; 
        }

        std::swap(arr[parentIndex], arr[maxIndex]);
        parentIndex = maxIndex;
        leftChildIndex = 2 * parentIndex + 1;
        rightChildIndex = 2 * parentIndex + 2;
    }
}

void buildHeap(int* arr, int n) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapifyDown(arr, n, i, n);
    }
}

void mergeKSortedArrays(std::vector<int*>& chunks, std::vector<int>& chunk_sizes, int* result_array, int total_size) {
    struct Element {
        int value;
        int chunk_idx;
        int element_idx_in_chunk;

        bool operator>(const Element& other) const {
            return value > other.value;
        }
    };

    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    for (int i = 0; i < chunks.size(); ++i) {
        if (chunk_sizes[i] > 0) {
            pq.push({chunks[i][0], i, 0});
        }
    }

    int current_result_idx = 0;
    while (!pq.empty()) {
        Element min_elem = pq.top();
        pq.pop();

        result_array[current_result_idx++] = min_elem.value;

        int next_element_idx = min_elem.element_idx_in_chunk + 1;
        if (next_element_idx < chunk_sizes[min_elem.chunk_idx]) {
            pq.push({chunks[min_elem.chunk_idx][next_element_idx], min_elem.chunk_idx, next_element_idx});
        }
    }
}


void parallelHeapSortDivideAndConquer(int* input, int n, int num_threads) {
    if (n <= 1) {
        return;
    }

    std::vector<int*> chunks;
    std::vector<int> chunk_sizes;
    int chunk_size = n / num_threads;
    int remaining = n % num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int current_chunk_size = chunk_size + (i < remaining ? 1 : 0);
        if (current_chunk_size > 0) {
            chunks.push_back(input + (i * chunk_size + std::min(i, remaining)));
            chunk_sizes.push_back(current_chunk_size);
        }
    }

    // Building a  heap for each chunk in parallel
    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < chunks.size(); ++i) {
        buildHeap(chunks[i], chunk_sizes[i]);
    }

    std::vector<int> temp_result(n);
    mergeKSortedArrays(chunks, chunk_sizes, temp_result.data(), n);

    for(int i = 0; i < n; ++i) {
        input[i] = temp_result[i];
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
    inplaceHeapSort(arr, n);
    auto endA = std::chrono::high_resolution_clock::now();
    auto durationA = std::chrono::duration_cast<std::chrono::milliseconds>(endA - startA).count();

    std::copy(arrCopy, arrCopy + n, arr);

    auto startB = std::chrono::high_resolution_clock::now();
    parallelHeapSortDivideAndConquer(arr, n, 2);
    auto endB = std::chrono::high_resolution_clock::now();
    auto durationB = std::chrono::duration_cast<std::chrono::milliseconds>(endB - startB).count();

    std::cout << "Heap Sort time: " << durationA << " ms\n";
    std::cout << "Heap Sort Parallel time: " << durationB << " ms\n";

    delete[] arr;
    delete[] arrCopy;

    return 0;
}
