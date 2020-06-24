#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const unsigned int DEFAULT_SEED = 1234U;
const int DEFAULT_RANGE = 10000000;
const int DEFAULT_THRESHOLD = 10;

const int INT_RADIX_BASE = 10;

typedef int (*pivot_func)(int[], int);
typedef int (*partition_func)(int[], int, int);
typedef void (*finished_func)(int[], int);
typedef struct partition_ {
  partition_func func;
  char *name;
} Partition;

typedef struct pivot_ {
  pivot_func func;
  char *name;
} Pivot;

typedef struct callback_ {
  finished_func func;
  int threshold;
  char *name;
} Callback;

typedef struct sorter_ {
  Pivot pivot;
  Partition partition;
  Callback callback;
} QuickSorter;

/* === Utility Methods === */

void swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

unsigned int parse_int(char *arg) {
  char *end;
  int val = strtol(arg, &end, INT_RADIX_BASE);
  if (end - arg < strlen(arg)) {
    fprintf(stderr, "%s is not an int (base %d)\n", arg, INT_RADIX_BASE);
    exit(EXIT_FAILURE);
  }
  return val;
}

int *init_array(int size) {
  int *arr = (int *)malloc(sizeof(int) * size);
  for (int i = 0; i < size; i++) {
    arr[i] = rand() % size;
  }
  return arr;
}

/* === Callbacks === */

void Id(int arr[], int len) {}
void InsertionSort(int arr[], int len) {
  int i = 1;
  while (i < len) {
    int x = arr[i];
    int j = i - 1;
    while (j >= 0 && arr[j] > x) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = x;
    i++;
  }
}

/* === Partitions === */

int lomuto(int arr[], int len, int pivot) {
  int s = 0;
  for (int i = 1; i < len; i++) {
    if (arr[i] < pivot) {
      s++;
      swap(&arr[i], &arr[s]);
    }
  }
  swap(&arr[0], &arr[s]);
  return s;
}

int hoare(int arr[], int len, int pivot) {
  int i = 1;
  int j = len - 1;
  while (true) {
    while (arr[i] < pivot && i < len) {
      i++;
    }

    while (arr[j] >= pivot && j > 0) {
      j--;
    }

    if (i < j) {
      swap(&arr[i], &arr[j]);
    } else {
      break;
    }
  }

  swap(&arr[i], &arr[j]);
  swap(&arr[0], &arr[j]);

  return j;
}

/* === Pivots === */

int median(int arr[], int len) {
  int lo = 0;
  int hi = len - 1;
  int mid = (hi + lo) / 2;
  int pivot;
  if (arr[lo] > arr[mid]) {
    swap(&arr[lo], &arr[mid]);
  }
  if (arr[lo] > arr[hi]) {
    swap(&arr[lo], &arr[hi]);
  }
  if (arr[hi] <= arr[mid]) {
    swap(&arr[hi], &arr[mid]);
  }

  swap(&arr[hi - 1], &arr[mid]);
  return arr[lo];
}

int first(int arr[], int len) { return arr[0]; }

int rand_pivot(int arr[], int len) {
  swap(&arr[0], &arr[rand() % len]);
  return arr[0];
}

/* === QuickSort === */

Pivot pivots[3] = {{first, "First Value"},
                   {median, "Median of 3"},
                   {rand_pivot, "Random value"}};
Partition partitions[2] = {{hoare, "Hoare"}, {lomuto, "Lomuto"}};
Callback callbacks[2] = {{Id, 2, "No Callback"},
                         {InsertionSort, 10, "Insertion Sort"}};

void QuickSort(int arr[], int len, QuickSorter config, bool toplevel) {
  if (len >= config.callback.threshold) {
    int pivot = config.pivot.func(arr, len);
    int p = config.partition.func(arr, len, pivot);
    QuickSort(arr, p, config, false);
    QuickSort(arr + p + 1, len - p - 1, config, false);
  }
  if (toplevel) {
    config.callback.func(arr, len);
  }
}

/* === MAIN === */

int main(int argc, char *argv[]) {
  int range;
  int thresholdMax;
  clock_t start;
  clock_t finish;

  // srand(DEFAULT_SEED); // reproducible seed
  srand(time(NULL)); // closer to random.

  if (argc == 3) {
    thresholdMax = parse_int(argv[2]);
    range = parse_int(argv[1]);
  } else if (argc == 2) {
    fprintf(stderr, "Using default insertion sort threshhold of %d\n",
            DEFAULT_THRESHOLD);
    thresholdMax = DEFAULT_THRESHOLD;
    range = parse_int(argv[1]);
  } else {
    fprintf(stderr, "Using default insertion sort threshhold of %d\n",
            DEFAULT_THRESHOLD);
    fprintf(stderr, "Using default range of %d\n", DEFAULT_RANGE);
    range = DEFAULT_RANGE;
  }

  printf("pivot,partition,callback,size,time\n");

  int windows = 100;
  int repeats = 100;
  int pivot_count = sizeof(pivots) / sizeof(*pivots);
  int partition_count = sizeof(partitions) / sizeof(*partitions);
  int callback_count = sizeof(callbacks) / sizeof(*callbacks);

  int trial = 1;
  int trials =
      windows * repeats * pivot_count * partition_count * callback_count;

  for (int size = range; size <= windows * range; size += range) {
    int *arr = init_array(size);
    fprintf(stderr, "\n");
    for (int pivot = 0; pivot < pivot_count; pivot++) {
      for (int part = 0; part < partition_count; part++) {
        for (int call = 0; call < callback_count; call++) {
          QuickSorter config = {pivots[pivot], partitions[part],
                                callbacks[call]};
          double time = 0;
          for (int n = 1; n <= repeats; n++) {
            fprintf(stderr, "\rTrial # %d / %d done. (%0.2f%% of all values) ",
                    trial, trials, 100.0 * pow((double)trial / trials, 2.0));

            int *tmp = (int *)malloc(size * sizeof(int));
            memcpy(tmp, arr, size * sizeof(int));
            start = clock();
            QuickSort(tmp, size, config, true);
            finish = clock();

            for (int idx = 0; idx < size - 1; idx++) {
              if (tmp[idx] > tmp[idx + 1]) {
                fprintf(stderr,
                        "error after sorting: tmp[%d] > tmp[%d], (%d > %d)\n.",
                        idx, idx + 1, tmp[idx], tmp[idx + 1]);
                exit(EXIT_FAILURE);
              }
            }
            time += (double)(finish - start) / (repeats * CLOCKS_PER_SEC);
            trial++;
            free(tmp);
          }
          printf("%s,%s,%s,%d,%f\n", pivots[pivot].name, partitions[part].name,
                 callbacks[call].name, size, time);
        }
      }
    }
    free(arr);
  }
}