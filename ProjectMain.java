import java.util.*;

public class ProjectMain {

    // ---------------- CPU Simulation ----------------
    public static class CPUModel {
        public String name;
        public double clockMultiplier;
        public double cacheMissPenalty; // in microseconds
        public double cacheMissRate;

        public CPUModel(String name, double clockMultiplier, double cacheMissPenalty, double cacheMissRate) {
            this.name = name;
            this.clockMultiplier = clockMultiplier;
            this.cacheMissPenalty = cacheMissPenalty;
            this.cacheMissRate = cacheMissRate;
        }

        public double simulateTime(double measured, int memAccesses) {
            double base = measured / clockMultiplier;
            double penalty = (cacheMissPenalty / 1e6) * cacheMissRate * memAccesses;
            return base + penalty;
        }
    }

    // ---------------- Algorithm Base (OOP Style) ----------------
    public static abstract class Algorithm {
        public String name;
        public Algorithm(String name) { this.name = name; }
        public abstract Object run(int[] arr, int target, Counter memAccesses);
    }

    // Counter class for tracking memory accesses
    public static class Counter { public int value; }

    // ---------------- Bubble Sort ----------------
    public static class BubbleSort extends Algorithm {
        public BubbleSort() { super("Bubble Sort"); }

        @Override
        public Object run(int[] arr, int target, Counter memAccesses) {
            int[] sorted = Arrays.copyOf(arr, arr.length);
            memAccesses.value = 0;

            for (int i = 0; i < sorted.length; i++) {
                for (int j = 0; j < sorted.length - i - 1; j++) {
                    memAccesses.value += 2; // read two elements
                    if (sorted[j] > sorted[j + 1]) {
                        int temp = sorted[j];
                        sorted[j] = sorted[j + 1];
                        sorted[j + 1] = temp;
                        memAccesses.value += 4; // write four times (two swaps)
                    }
                }
            }
            return sorted; // return sorted array
        }
    }

    // ---------------- Linear Search ----------------
    public static class LinearSearch extends Algorithm {
        public LinearSearch() { super("Linear Search"); }

        @Override
        public Object run(int[] arr, int target, Counter memAccesses) {
            memAccesses.value = 0;
            for (int i = 0; i < arr.length; i++) {
                memAccesses.value++;
                if (arr[i] == target) {
                    return Integer.valueOf(i); // found index (boxed)
                }
            }
            return Integer.valueOf(-1); // not found
        }
    }

    // ---------------- Statistics ----------------
    public static class Statistics {
        public static double mean(double[] arr) {
            if (arr == null || arr.length == 0) return 0.0;
            double sum = 0;
            for (double v : arr) sum += v;
            return sum / arr.length;
        }

        public static double stddev(double[] arr, double mean) {
            if (arr == null || arr.length == 0) return 0.0;
            double sum = 0;
            for (double v : arr) sum += (v - mean) * (v - mean);
            return Math.sqrt(sum / arr.length);
        }
    }

    // ---------------- Experiment Runner ----------------
    public static class Experiment {
        public static void runExperiment(Algorithm alg, CPUModel cpu, int inputSize, int runs) {
            Random rand = new Random();
            double[] times = new double[runs];
            Counter memAccesses = new Counter();

            for (int r = 0; r < runs; r++) {
                int[] arr = new int[inputSize];
                // ensure bound > 0 for nextInt
                int bound = Math.max(1, inputSize * 10);
                for (int i = 0; i < inputSize; i++) arr[i] = rand.nextInt(bound);
                int target = arr[rand.nextInt(inputSize)];

                long start = System.nanoTime();
                Object result = alg.run(arr, target, memAccesses);
                long end = System.nanoTime();

                double measured = (end - start) / 1e9; // seconds
                times[r] = cpu.simulateTime(measured, memAccesses.value);
            }

            double mean = Statistics.mean(times);
            double sd = Statistics.stddev(times, mean);

            System.out.printf("| %-12s | %-6s | %6d | %10.6f | %10.6f |\n",
                    alg.name, cpu.name, inputSize, mean, sd);
        }
    }

    // ---------------- Main ----------------
    public static void main(String[] args) {
        // CPUs
        CPUModel basic = new CPUModel("Basic", 1.0, 50, 0.02);
        CPUModel mid   = new CPUModel("Mid",   2.0, 30, 0.015);
        CPUModel pro   = new CPUModel("Pro",   4.0, 10, 0.01);
        CPUModel[] cpus = { basic, mid, pro };

        // Algorithms
        Algorithm[] algs = { new BubbleSort(), new LinearSearch() };

        int[] sizes = { 100, 500, 1000 };

        System.out.println("| Algorithm    | CPU    | Input |   Mean(s)   |  StdDev(s)  |");
        System.out.println("|--------------|--------|-------|-------------|-------------|");

        for (Algorithm alg : algs) {
            for (CPUModel cpu : cpus) {
                for (int size : sizes) {
                    Experiment.runExperiment(alg, cpu, size, 5);
                }
            }
        }
    }
}
