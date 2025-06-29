#include <SDL2/SDL.h>
#include <iostream>
#include <limits>
#include <time.h>
#include <string>
#include <omp.h>
#include <set>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>


using namespace std;

const int SCREEN_WIDTH=1024;
const int SCREEN_HEIGHT=700;

const int arrSize=128;
const int rectSize=1024/128;

int arr[arrSize];
int Barr[arrSize];

mutex visualize_mutex;
set<int> greenIndices;
set<int> pinkIndices;

SDL_Window* window=NULL;
SDL_Renderer* renderer=NULL;

bool complete=false;

inline void busywait_ms(int milliseconds) {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::high_resolution_clock::now() - start).count() < milliseconds){
        // spin to keep the thread busy (avoids task stealing)
        asm volatile("" ::: "memory");  // prevent loop from being optimized away
    }
}

bool init()
{
    bool success=true;
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        cout<<"Couldn't initialize SDL. SDL_Error: "<<SDL_GetError();
        success=false;
    }
    else
    {
        if(!(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")))
        {
            cout<<"Warning: Linear Texture Filtering not enabled.\n";
        }

        window=SDL_CreateWindow("Sorting Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(window==NULL)
        {
            cout<<"Couldn't create window. SDL_Error: "<<SDL_GetError();
            success=false;
        }
        else
        {
            renderer=SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if(renderer==NULL)
            {
                cout<<"Couldn't create renderer. SDL_Error: "<<SDL_GetError();
                success=false;
            }
        }
    }

    return success;
}

void close()
{
    SDL_DestroyRenderer(renderer);
    renderer=NULL;

    SDL_DestroyWindow(window);
    window=NULL;

    SDL_Quit();
}

void visualize_parallel()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    lock_guard<mutex> lock(visualize_mutex);

    int j=0;
    for(int i=0; i<=SCREEN_WIDTH-rectSize; i+=rectSize)
    {
        SDL_Rect rect={i, 0, rectSize, arr[j]};
        if(complete)
        {
            SDL_SetRenderDrawColor(renderer, 100, 180, 100, 0);
            SDL_RenderDrawRect(renderer, &rect);
        }
        else if (greenIndices.count(j))
        {
            SDL_SetRenderDrawColor(renderer, 100, 180, 100, 0);
            SDL_RenderFillRect(renderer, &rect);
        }
        else if (pinkIndices.count(j))
        {
            SDL_SetRenderDrawColor(renderer, 165, 105, 189, 0);
            SDL_RenderFillRect(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 170, 183, 184, 0);
            SDL_RenderDrawRect(renderer, &rect);
        }
        j++;
    }
    SDL_RenderPresent(renderer);
}


void visualize(int x=-1, int y=-1, int z=-1)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    int j=0;
    for(int i=0; i<=SCREEN_WIDTH-rectSize; i+=rectSize)
    {
        SDL_PumpEvents();

        SDL_Rect rect={i, 0, rectSize, arr[j]};
        if(complete)
        {
            SDL_SetRenderDrawColor(renderer, 100, 180, 100, 0);
            SDL_RenderDrawRect(renderer, &rect);
        }
        else if(j==x || j==z)
        {
            SDL_SetRenderDrawColor(renderer, 100, 180, 100, 0);
            SDL_RenderFillRect(renderer, &rect);
        }
        else if(j==y)
        {
            SDL_SetRenderDrawColor(renderer, 165, 105, 189, 0);
            SDL_RenderFillRect(renderer, &rect);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 170, 183, 184, 0);
            SDL_RenderDrawRect(renderer, &rect);
        }
        j++;
    }
    SDL_RenderPresent(renderer);
}

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

           visualize(parentIndex, childIndex);
           SDL_Delay(40);

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

            visualize(maxIndex, parentIndex, heapLast);
            SDL_Delay(40);

            parentIndex=maxIndex;
            leftChildIndex=2*parentIndex + 1;
            rightChildIndex=2*parentIndex + 2;
        }
    }
}


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
    visualize(c, si);

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

            visualize(i, j);
            SDL_Delay(70);

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

int partitionArrayParallel(int a[], int si, int ei) {
    int count_small = 0;

    for (int i = si + 1; i <= ei; i++) {
        if (a[i] <= a[si]) {
            count_small++;
        }
    }

    int c = si + count_small;
    swap(a[c], a[si]);

    #pragma omp critical
    {
        greenIndices.insert(si);
        pinkIndices.insert(c);
    }

    #pragma omp critical
    {
        visualize_parallel();
    }

    this_thread::sleep_for(chrono::milliseconds(30));

    #pragma omp critical
    {
        greenIndices.erase(si);
        pinkIndices.erase(c);
    }

    int i = si, j = ei;

    while (i < c && j > c) {
        if (a[i] <= a[c]) {
            i++;
        } else if (a[j] > a[c]) {
            j--;
        } else {
            swap(a[i], a[j]);

            #pragma omp critical
            {
                greenIndices.insert(i);
                pinkIndices.insert(j);
            }

            #pragma omp critical
            {
                visualize_parallel();
            }

            this_thread::sleep_for(chrono::milliseconds(30));

            #pragma omp critical
            {
                greenIndices.erase(i);
                pinkIndices.erase(j);
            }
            i++;
            j--;
        }
    }
    return c;
}

void quickSortParallel(int a[], int si, int ei) {
    if (si >= ei)
        return;

    int c = partitionArrayParallel(a, si, ei);

    int size = ei - si;
    if (size > 10000) {
        #pragma omp task shared(a)
        quickSortParallel(a, si, c - 1);

        #pragma omp task shared(a)
        quickSortParallel(a, c + 1, ei);

        #pragma omp taskwait
    } else {
        quickSortParallel(a, si, c - 1);
        quickSortParallel(a, c + 1, ei);
    }
}

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
            visualize(i, j);
            i++;
            k++;
        }
        else
        {
            output[k]=a[j];
            visualize(i, j);
            j++;
            k++;
        }

    }
    while(i<=mid)
    {
        output[k]=a[i];
        visualize(-1, i);
        i++;
        k++;
    }
    while(j<=ei)
    {
        output[k]=a[j];
        visualize(-1, j);
        j++;
        k++;
    }
    int x=0;
    for(int l=si; l<=ei; l++)
    {
        a[l]=output[x];
        visualize(l);
        SDL_Delay(15);
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
    int* support=new int[size_output];

    int mid=(si+ei)/2;
    int i=si, j=mid+1, k=0;
    while(i<=mid && j<=ei)
    {
        if(a[i]<=a[j])
        {
            output[k]=a[i];
            support[k] = i;
            i++;
            k++;
        }
        else
        {
            output[k]=a[j];
            support[k] = j;
            j++;
            k++;
        }

    }
    while(i<=mid)
    {
        output[k]=a[i];
        support[k] = i;
        i++;
        k++;
    }
    while(j<=ei)
    {
        output[k]=a[j];
        support[k] = j;
        j++;
        k++;
    }
    int x=0;
    for(int l=si; l<=ei; l++)
    {
        a[l]=output[x];

        #pragma omp critical
        {
            greenIndices.insert(l);
        }
        
        #pragma omp critical
        {
            visualize_parallel();  
            busywait_ms(15);
        }
        //busywait here
        busywait_ms(45);

        #pragma omp critical
        {
            greenIndices.erase(l);
        }
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

    #pragma omp task shared(a)
    mergeSortParallelHelper(a, si, mid);

    #pragma omp task shared(a)
    mergeSortParallelHelper(a, mid+1, ei);

    #pragma omp taskwait
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

void bubbleSortParallel()
/*
In general bubbleSort is sequential as comparisons move along
the array. To parallelize this doesn't work.

A possible solution is to first compare even-starting pairs
and odd-starting pairs. In this manner in the same iteration
one shift doesn't need to be sequential with the others, then
you may use openMP for parallel computing.

*/
{
    for (int i = 0; i < arrSize; i++)
    {
        int phase = i % 2;

        #pragma omp parallel for
        for (int j = phase; j < arrSize - 1; j += 2)
        {
            if (arr[j] > arr[j + 1])
            {
                // Swap
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }

            #pragma omp critical
            {
                greenIndices.insert(j);
                pinkIndices.insert(j + 1);
            }

            #pragma omp critical
            {
                visualize_parallel();
            }

            this_thread::sleep_for(chrono::milliseconds(30));

            #pragma omp critical
            {
                greenIndices.erase(j);
                pinkIndices.erase(j + 1);
            }
        }
    }
}

void bubbleSort()
{
    for(int i=0; i<arrSize-1; i++)
    {
        for(int j=0; j<arrSize-1-i; j++)
        {
            if(arr[j+1]<arr[j])
            {
                int temp=arr[j];
                arr[j]=arr[j+1];
                arr[j+1]=temp;

                visualize(j+1, j, arrSize-i);
            }
        }
    }
}

void insertionSort()
{
    for(int i=1; i<arrSize; i++)
    {
        int j=i-1;
        int temp=arr[i];
        while(j>=0 && arr[j]>temp)
        {
            arr[j+1]=arr[j];
            j--;

            visualize(i, j+1);
            SDL_Delay(5);
        }
        arr[j+1]=temp;
    }
}

void selectionSortParallel()
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


        #pragma omp critical
        {
            greenIndices.insert(i);
            pinkIndices.insert(minIndex);
        }

        #pragma omp critical
        {
            visualize_parallel();
        }

        this_thread::sleep_for(chrono::milliseconds(30));

        #pragma omp critical
        {
            greenIndices.erase(i);
            pinkIndices.erase(minIndex);
        }
    }
}

void selectionSort()
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
                visualize(i, minIndex);
            }
            SDL_Delay(1);
        }
        int temp=arr[i];
        arr[i]=arr[minIndex];
        arr[minIndex]=temp;
    }
}

void bitonicMerge(int low, int count, bool dir)
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
                visualize(i, i + k);
            }
            SDL_Delay(10);
        }
        bitonicMerge(low, k, dir);
        bitonicMerge(low + k, k, dir);
    }
}

void bitonicSortRec(int low, int count, bool dir)
{
    if (count > 1)
    {
        int k = count / 2;

        bitonicSortRec(low, k, true);
        bitonicSortRec(low + k, k, false);

        bitonicMerge(low, count, dir);
    }
}

void bitonicSort()
{
    bitonicSortRec(0, arrSize, true);
}

void bitonicMergeParallel(int a[], int low, int count, bool dir)
{
    if(count > 1)
    {
        int k = count / 2;
        for(int i = low; i < low + k; i++)
        {
            #pragma omp critical
            {
                greenIndices.insert(i);
                greenIndices.insert(i + k);
            }

            #pragma omp critical
            {
                visualize_parallel();
                busywait_ms(15);
            }

            if((dir && a[i] > a[i + k]) || (!dir && a[i] < a[i + k]))
            {
                int temp = a[i];
                a[i] = a[i + k];
                a[i + k] = temp;

                busywait_ms(45);
            }

            #pragma omp critical
            {
                greenIndices.erase(i);
                greenIndices.erase(i + k);
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

        #pragma omp task shared(a)
        bitonicSortParallelHelper(a, low, k, true);

        #pragma omp task shared(a)
        bitonicSortParallelHelper(a, low + k, k, false);

        #pragma omp taskwait
        bitonicMergeParallel(a, low, count, dir);
    }
}

void bitonicSortParallel(int a[], int size)
{
    #pragma omp critical
    {
        visualize_parallel();
        busywait_ms(300);
    }
    #pragma omp parallel
    {
        #pragma omp single
        {
            bitonicSortParallelHelper(a, 0, size, true);
        }
    }
}


void loadArr()
{
    memcpy(arr, Barr, sizeof(int)*arrSize);
}

void randomizeAndSaveArray()
{
    unsigned int seed=(unsigned)time(NULL);
    srand(seed);
    for(int i=0; i<arrSize; i++)
    {
        int random=rand()%(SCREEN_HEIGHT);
        Barr[i]=random;
    }
}

void execute()
{
    if(!init())
    {
        cout<<"SDL Initialization Failed.\n";
    }
    else
    {
        randomizeAndSaveArray();
        loadArr();

        SDL_Event e;
        bool quit=false;
        while(!quit)
        {
            while(SDL_PollEvent(&e)!=0)
            {
                if(e.type==SDL_QUIT)
                {
                    quit=true;
                    complete=false;
                }
                else if(e.type==SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {
                        case(SDLK_q):
                            quit=true;
                            complete=false;
                            cout<<"\nEXITING SORTING VISUALIZER.\n";
                            break;
                        case(SDLK_0):
                            randomizeAndSaveArray();
                            complete=false;
                            loadArr();
                            cout<<"\nNEW RANDOM LIST GENERATED.\n";
                            break;
                        case(SDLK_1):
                            loadArr();
                            cout<<"\nSELECTION SORT STARTED.\n";
                            complete=false;
                            selectionSort();
                            complete=true;
                            cout<<"\nSELECTION SORT COMPLETE.\n";
                            break;
                        case(SDLK_2):
                            loadArr();
                            cout<<"\nINSERTION SORT STARTED.\n";
                            complete=false;
                            insertionSort();
                            complete=true;
                            cout<<"\nINSERTION SORT COMPLETE.\n";
                            break;
                        case(SDLK_3):
                            loadArr();
                            cout<<"\nBUBBLE SORT STARTED.\n";
                            complete=false;
                            bubbleSort();
                            complete=true;
                            cout<<"\nBUBBLE SORT COMPLETE.\n";
                            break;
                        case(SDLK_4):
                            loadArr();
                            cout<<"\nMERGE SORT STARTED.\n";
                            complete=false;
                            mergeSort(arr, 0, arrSize-1);
                            complete=true;
                            cout<<"\nMERGE SORT COMPLETE.\n";
                            break;
                        case(SDLK_5):
                            loadArr();
                            cout<<"\nQUICK SORT STARTED.\n";
                            complete=false;
                            quickSort(arr, 0, arrSize-1);
                            complete=true;
                            cout<<"\nQUICK SORT COMPLETE.\n";
                            break;
                        case(SDLK_6):
                            loadArr();
                            cout<<"\nHEAP SORT STARTED.\n";
                            complete=false;
                            inplaceHeapSort(arr, arrSize);
                            complete=true;
                            cout<<"\nHEAP SORT COMPLETE.\n";
                            break;
                        case(SDLK_7):
                            loadArr();
                            cout<<"\nBITONIC SORT STARTED.\n";
                            complete=false;
                            bitonicSort();
                            complete=true;
                            cout<<"\nBITONIC SORT COMPLETE.\n";
                            break;
                        case(SDLK_a):
                            loadArr();
                            cout<<"\nPARALLEL SELECTION SORT STARTED.\n";
                            complete=false;
                            selectionSortParallel();
                            complete=true;
                            cout<<"\nPARALLEL SELECTION SORT COMPLETE.\n";
                            break;
                        case(SDLK_b):
                            loadArr();
                            cout<<"\nPARALLEL BUBBLE SORT STARTED.\n";
                            complete=false;
                            bubbleSortParallel();
                            complete=true;
                            cout<<"\nPARALLEL BUBBLE SORT COMPLETE.\n";
                            break;
                        case(SDLK_c):
                            loadArr();
                            cout<<"\nPARALLEL MERGE SORT STARTED.\n";
                            complete=false;
                            mergeSortParallel(arr, 0, arrSize - 1);
                            complete=true;
                            cout<<"\nPARALLEL MERGE SORT COMPLETE.\n";
                            break;
                        case(SDLK_d):
                            loadArr();
                            cout<<"\nPARALLEL QUICK SORT STARTED.\n";
                            complete=false;
                            quickSortParallel(arr, 0, arrSize-1);
                            complete=true;
                            cout<<"\nPARALLEL QUICK SORT COMPLETE.\n";
                            break;
                        case(SDLK_e):
                            loadArr();
                            cout<<"\nPARALLEL BITONIC SORT STARTED.\n";
                            complete=false;
                            bitonicSortParallel(arr, arrSize);
                            complete=true;
                            cout<<"\nPARALLEL BITONIC SORT COMPLETE.\n";
                            break;
                    }
                }
            }
            visualize();
        }
        close();
    }
}

bool controls()
{
    cout <<"Available Commands inside Sorting Visualizer:\n"
         <<"    Use 0 to Generate a different randomized list.\n"
         <<"    Use 1 to start Selection Sort Algorithm.\n"
         <<"    Use 2 to start Insertion Sort Algorithm.\n"
         <<"    Use 3 to start Bubble Sort Algorithm.\n"
         <<"    Use 4 to start Merge Sort Algorithm.\n"
         <<"    Use 5 to start Quick Sort Algorithm.\n"
         <<"    Use 6 to start Heap Sort Algorithm.\n"
         <<"    Use 7 to start Bitonic Sort Algorithm.\n"
         <<"    Use a to start Parallel Selection Sort Algorithm.\n"
         <<"    Use b to start Parallel Bubble Sort Algorithm.\n"
         <<"    Use c to start Parallel Merge Sort Algorithm.\n"
         <<"    Use d to start Parallel Quick Sort Algorithm.\n"
         <<"    Use e to start Parallel Bitonic Sort Algorithm.\n"
         <<"    Use q to exit out of Sorting Visualizer\n\n"

         <<"WARNING: Giving repetitive commands may cause latency and the visualizer may behave unexpectedly. Please give a new command only after the current command's execution is done.\n\n"
         <<"THIS IS HOW THE SORTING VISUALIZER WORKS:\n\n"
         <<"  - Once you press ENTER, the visualizer window will open. Click on the window and press the command you'd like to execute, to see the visualization of the algorithm you chose.\n"
         <<"  - To quit the program, you should exit the visualizer window pressing 'q' key, and then, type -1 in the terminal and press ENTER to quit the program.\n\n"
         <<"PRESS ENTER TO START SORTING VISUALIZER...\n\n"
         <<"Or type -1 on the terminal and press ENTER to quit the program.";

    string s;
    getline(cin, s);
    if(s=="-1")
    {
        return false;
    }
    //else if(s=="\n")
    //{
    //    return true;
    //}
    return true;
}

void intro()
{
    cout<<"==============================Sorting Visualizer==============================\n\n"
        <<"Visualization of different sorting algorithms in C++ with SDL2 Library. A sorting algorithm is an algorithm that puts the elements of a list in a certain order. While there are a large number of sorting algorithms, in practical implementations a few algorithms predominate.\n"
        <<"In this implementation of sorting visualizer, we'll be looking at some of these sorting algorithms and visually comprehend their working.\n"
        <<"The sorting algorithms covered here are Selection Sort, Insertion Sort, Bubble Sort, Merge Sort, Quick Sort and Heap Sort.\n"
        <<"The list size is fixed to 130 elements. You can randomize the list and select any type of sorting algorithm to call on the list from the given options. Here, all sorting algorithms will sort the elements in ascending order. The sorting time being visualized for an algorithm is not exactly same as their actual time complexities. The relatively faster algorithms like Merge Sort, etc. have been delayed so that they could be properly visualized.\n\n"
        <<"Press ENTER to show controls...";

    string s;
    getline(cin, s);
    if(s=="\n")
    {
        return;
    }
}

int main(int argc, char* args[])
{
    intro();
    omp_set_num_threads(4);
    while(1)
    {
        cout<<'\n';
        if(controls())
            execute();
        else
        {
            cout<<"\nEXITING PROGRAM.\n";
            break;
        }
    }

    return 0;
}
