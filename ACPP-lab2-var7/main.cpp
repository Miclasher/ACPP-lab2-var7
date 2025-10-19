#include <iostream>
#include <vector>
#include <numeric>
#include <execution>
#include <random>
#include <chrono>
#include <thread>
#include <functional>
#include <cmath>
#include <iomanip>
#include <string>
#include <algorithm>

template<typename Func>
double measure_time_ms(Func f) {
    auto start = std::chrono::high_resolution_clock::now();
    f();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count();
}

std::vector<double> generate_data(size_t size) {
    std::vector<double> data(size);
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis(0.0, 100.0);
    std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
    return data;
}

template<typename It, typename T, typename Reduce, typename Transform>
T custom_parallel_transform_reduce(It first, It last, T init, Reduce reduce_op, Transform transform_op, unsigned int K) {
    const auto size = std::distance(first, last);
    if (size == 0 || K == 0) {
        return init;
    }

    if (K == 1) {
        return std::transform_reduce(std::execution::seq, first, last, init, reduce_op, transform_op);
    }

    std::vector<T> partial_results(K);
    std::vector<std::jthread> threads;

    const size_t chunk_size = (size + K - 1) / K;

    for (unsigned int i = 0; i < K; ++i) {
        auto chunk_start = first + i * chunk_size;
        auto chunk_end = std::min(first + (i + 1) * chunk_size, last);

        if (chunk_start >= chunk_end) continue;

        threads.emplace_back([&partial_results, i, chunk_start, chunk_end, reduce_op, transform_op]() {
            partial_results[i] = std::transform_reduce(
                std::execution::seq,
                chunk_start,
                chunk_end,
                T{},
                reduce_op,
                transform_op
            );
            });
    }

    return std::reduce(partial_results.begin(), partial_results.end(), init, reduce_op);
}

volatile double result_sink = 0.0;

void run_experiment(size_t data_size) {
    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "Starting experiment for data size: " << data_size << " elements\n";
    std::cout << std::string(80, '=') << "\n\n";

    auto data = generate_data(data_size);
    auto transform_op = [](double val) { return std::sin(val) * std::sqrt(val); };
    auto reduce_op = std::plus<double>{};
    double init_value = 0.0;

    std::cout << "--- Standard Library Algorithms ---\n";

    double time_no_policy = measure_time_ms([&]() {
        result_sink += std::transform_reduce(data.begin(), data.end(), init_value, reduce_op, transform_op);
        });
    std::cout << std::fixed << std::setprecision(4)
        << "1) No policy:          " << std::setw(10) << time_no_policy << " ms\n";

    double time_seq_policy = measure_time_ms([&]() {
        result_sink += std::transform_reduce(std::execution::seq, data.begin(), data.end(), init_value, reduce_op, transform_op);
        });
    std::cout << "2) execution::seq:     " << std::setw(10) << time_seq_policy << " ms\n";

    double time_par_policy = measure_time_ms([&]() {
        result_sink += std::transform_reduce(std::execution::par, data.begin(), data.end(), init_value, reduce_op, transform_op);
        });
    std::cout << "3) execution::par:     " << std::setw(10) << time_par_policy << " ms (Speedup vs seq: "
        << time_seq_policy / time_par_policy << "x)\n";

    double time_par_unseq_policy = measure_time_ms([&]() {
        result_sink += std::transform_reduce(std::execution::par_unseq, data.begin(), data.end(), init_value, reduce_op, transform_op);
        });
    std::cout << "4) execution::par_unseq: " << std::setw(10) << time_par_unseq_policy << " ms (Speedup vs seq: "
        << time_seq_policy / time_par_unseq_policy << "x)\n\n";

    std::cout << "--- Custom Parallel Algorithm ---\n";

    const unsigned int hardware_threads = std::thread::hardware_concurrency();
    std::cout << "Number of hardware threads on this system: " << hardware_threads << "\n\n";

    std::cout << "+-------+----------------+----------+\n";
    std::cout << "|   K   |  Time (ms)     | Speedup  |\n";
    std::cout << "+-------+----------------+----------+\n";

    double best_time = time_seq_policy;
    unsigned int best_k = 1;

    for (unsigned int k = 1; k <= hardware_threads * 2; ++k) {
        double current_time = measure_time_ms([&]() {
            result_sink += custom_parallel_transform_reduce(data.begin(), data.end(), init_value, reduce_op, transform_op, k);
            });

        if (current_time < best_time) {
            best_time = current_time;
            best_k = k;
        }

        double speedup = (k == 1) ? 1.0 : time_seq_policy / current_time;
        std::cout << "| " << std::setw(5) << k << " | "
            << std::setw(14) << std::fixed << std::setprecision(4) << current_time << " | "
            << std::setw(8) << std::fixed << std::setprecision(2) << speedup << "x |\n";
    }
    std::cout << "+-------+----------------+----------+\n\n";

    std::cout << "--- Findings for this data size ---\n";
    std::cout << "Best performance for custom algorithm was achieved at K = " << best_k << " threads.\n";
    std::cout << "This corresponds to " << static_cast<double>(best_k) / hardware_threads
        << " times the number of available hardware threads (" << hardware_threads << ").\n";
    std::cout << "The best speedup achieved was " << std::fixed << std::setprecision(2)
        << time_seq_policy / best_time << "x compared to the sequential version.\n";
}

int main() {
    size_t size = 100000000;

    run_experiment(size);

    std::cout << "\n" << std::string(80, '=') << "\n";
    std::cout << "All experiments completed.\n";
    std::cout << "Final result sink (to prevent optimization): " << result_sink << std::endl;
    std::cout << std::string(80, '=') << "\n";

    return 0;
}