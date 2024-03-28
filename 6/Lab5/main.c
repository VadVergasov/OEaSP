#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#define NUM_THREADS 4

typedef struct {
    int* array;
    int start;
    int end;
} ThreadArgs;

int compare(const void* a, const void* b) { return (*(int*)a - *(int*)b); }

void* sort(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    qsort(args->array + args->start, args->end - args->start + 1, sizeof(int),
          compare);
    pthread_exit(NULL);
}

void merge(int* array, int start, int mid, int end) {
    int leftSize = mid - start + 1;
    int rightSize = end - mid;

    int* leftArray = malloc(leftSize * sizeof(int));
    int* rightArray = malloc(rightSize * sizeof(int));

    for (int i = 0; i < leftSize; i++) {
        leftArray[i] = array[start + i];
    }
    for (int i = 0; i < rightSize; i++) {
        rightArray[i] = array[mid + 1 + i];
    }

    int i = 0, j = 0, k = start;
    while (i < leftSize && j < rightSize) {
        if (leftArray[i] <= rightArray[j]) {
            array[k++] = leftArray[i++];
        } else {
            array[k++] = rightArray[j++];
        }
    }

    while (i < leftSize) {
        array[k++] = leftArray[i++];
    }
    while (j < rightSize) {
        array[k++] = rightArray[j++];
    }

    free(leftArray);
    free(rightArray);
}

// Функция для создания и запуска потоков
void createThreads(pthread_t threads[], ThreadArgs threadArgs[], int* array,
                   int chunkSize) {
    for (int i = 0; i < NUM_THREADS; i++) {
        threadArgs[i].array = array;
        threadArgs[i].start = i * chunkSize;
        threadArgs[i].end = (i + 1) * chunkSize - 1;

        pthread_create(&threads[i], NULL, sort, (void*)&threadArgs[i]);
    }
}

int main() {
    srand(time(NULL));
    int size;

    scanf("%d", &size);

    int* array = (int*)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        array[i] = rand() % 10000;
    }

    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    int chunkSize = size / NUM_THREADS;

    pthread_t threads[NUM_THREADS];
    ThreadArgs threadArgs[NUM_THREADS];

    createThreads(threads, threadArgs, array, chunkSize);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 1; i < NUM_THREADS; i++) {
        merge(array, 0, i * chunkSize - 1, (i + 1) * chunkSize - 1);
    }

    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");

    return 0;
}
