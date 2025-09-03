#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

// ---------------- CPU Simulation ----------------
typedef struct {
    char name[20];
    double clock_multiplier;     // relative speed
    double cache_miss_penalty;   // penalty in microseconds
    double cache_miss_rate;      // fraction of misses
} CPUModel;

double simulate_time(double measured, CPUModel cpu, int mem_accesses) {
    double base = measured / cpu.clock_multiplier;
    double penalty = (cpu.cache_miss_penalty / 1e6) * cpu.cache_miss_rate * mem_accesses;
    return base + penalty;
}

// ---------------- Algorithm Base (OOP style in C) ----------------
typedef struct Algorithm {
    char name[30];
    void* (run)(struct Algorithm self, int* arr, int n, int target, int* mem_accesses);
} Algorithm;

// ---------------- Bubble Sort ----------------
void* bubble_sort_run(Algorithm* self, int* arr, int n, int target, int* mem_accesses) {
    int* sorted = (int*)malloc(n * sizeof(int));
    memcpy(sorted, arr, n * sizeof(int));

    *mem_accesses = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            (*mem_accesses) += 2;
            if (sorted[j] > sorted[j+1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j+1];
                sorted[j+1] = temp;
                (*mem_accesses) += 4;
            }
        }
    }
    return sorted;
}

Algorithm* create_bubble_sort() {
    Algorithm* alg = (Algorithm*)malloc(sizeof(Algorithm));
    strcpy(alg->name, "Bubble Sort");
    alg->run = bubble_sort_run;
    return alg;
}

// ---------------- Linear Search ----------------
void* linear_search_run(Algorithm* self, int* arr, int n, int target, int* mem_accesses) {
    *mem_accesses = 0;
    for (int i = 0; i < n; i++) {
        (*mem_accesses)++;
        if (arr[i] == target) {
            int* result = (int*)malloc(sizeof(int));
            *result = i;
            return result;
        }
    }
    int* result = (int*)malloc(sizeof(int));
    *result = -1;
    return result;
}

Algorithm* create_linear_search() {
    Algorithm* alg = (Algorithm*)malloc(sizeof(Algorithm));
    strcpy(alg->name, "Linear Search");
    alg->run = linear_search_run;
    return alg;
}

// ---------------- Statistics ----------------
double mean(double arr[], int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += arr[i];
    return sum / n;
}

double stddev(double arr[], int n, double m) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += (arr[i] - m) * (arr[i] - m);
    return sqrt(sum / n);
}

// ---------------- Experiment Runner ----------------
void run_experiment(Algorithm* alg, CPUModel cpu, int input_size, int runs) {
    int* arr = (int*)malloc(input_size * sizeof(int));
    double times[50];
    int mem_accesses;

    for (int r = 0; r < runs; r++) {
        for (int i = 0; i < input_size; i++) {
            arr[i] = rand() % (input_size * 10);
        }
        int target = arr[rand() % input_size];

        clock_t start = clock();
        void* result = alg->run(alg, arr, input_size, target, &mem_accesses);
        clock_t end = clock();

        double measured = (double)(end - start) / CLOCKS_PER_SEC;
        times[r] = simulate_time(measured, cpu, mem_accesses);

        free(result); // free allocated result
    }

    double m = mean(times, runs);
    double sd = stddev(times, runs, m);

    printf("| %-12s | %-6s | %6d | %8.6f | %8.6f |\n",
           alg->name, cpu.name, input_size, m, sd);

    free(arr);
}

// ---------------- Main ----------------
int main() {
    srand(time(NULL));

    // CPUs
    CPUModel basic = {"Basic", 1.0, 50, 0.02};
    CPUModel mid   = {"Mid",   2.0, 30, 0.015};
    CPUModel pro   = {"Pro",   4.0, 10, 0.01};

    CPUModel cpus[] = {basic, mid, pro};
    int num_cpus = 3;

    // Algorithms
    Algorithm* bubble = create_bubble_sort();
    Algorithm* linear = create_linear_search();
    Algorithm* algs[] = {bubble, linear};
    int num_algs = 2;

    int sizes[] = {100, 500, 1000};
    int num_sizes = 3;

    printf("| Algorithm    | CPU    | Input |   Mean(s) |  StdDev(s) |\n");
    printf("|--------------|--------|-------|-----------|------------|\n");

    for (int a = 0; a < num_algs; a++) {
        for (int c = 0; c < num_cpus; c++) {
            for (int s = 0; s < num_sizes; s++) {
                run_experiment(algs[a], cpus[c], sizes[s], 5);
            }
        }
    }

    free(bubble);
    free(linear);
    return 0;
}
