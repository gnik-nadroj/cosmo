import numpy as np
import matplotlib.pyplot as plt
import os

value_sizes = ['1KB', '4KB', '16KB', '64KB', '256KB']
file_types = ['seq', 'concurrent']
metrics = ['Fastest', 'Mean', 'Median', '99th', 'Tail']


def read_latencies(file_path):
    with open(file_path, 'r') as file:
        latencies = [float(line.strip()) for line in file]
    return np.array(latencies)

def plot_diagrams(value_sizes, file_types, statistics):
    evolution_fig, evolution_axs = plt.subplots(1, len(metrics), figsize=(20, 5))
    evolution_fig.suptitle('Evolution of Metrics by Value Size')

    for i, metric in enumerate(metrics):
        metric_values = {
            file_type: [statistics[size][file_type][i] for size in value_sizes]
            for file_type in file_types
        }
        for file_type, values in metric_values.items():
            evolution_axs[i].plot(value_sizes, values, label=f'{file_type.capitalize()} Write', marker='o')
            evolution_axs[i].set_title(f'{metric} Latency Evolution')
            evolution_axs[i].set_xlabel('Value Size')
            evolution_axs[i].set_ylabel('Latency (ms)')
            evolution_axs[i].legend()

    throughput_fig, throughput_ax = plt.subplots(figsize=(10, 5))
    throughput_fig.suptitle('Throughput Evolution by Value Size')

    for file_type in file_types:
        throughput_values = statistics['throughput'][file_type]
        print(throughput_values)
        throughput_ax.plot(value_sizes, throughput_values, label=f'{file_type.capitalize()} Write', marker='o')
        throughput_ax.set_title('Throughput (GB/s)')
        throughput_ax.set_xlabel('Value Size')
        throughput_ax.set_ylabel('Throughput (GB/s)')
        throughput_ax.legend()

    plt.tight_layout(rect=[0, 0.03, 1, 0.95])
    plt.show()

def calculate_statistics_and_throughput(latencies, value_size, num_requests):
    fastest = np.min(latencies)
    mean = np.mean(latencies)
    median = np.median(latencies)
    percentile_99 = np.percentile(latencies, 99)
    tail_latency = np.max(latencies)
    total_time = np.sum(latencies) / 1000
    value_size_bytes = int(value_size[:-2]) * (1024 if 'KB' in value_size else 1)
    total_data_transferred = value_size_bytes * num_requests
    throughput_gbps = (total_data_transferred / total_time) / (1024**3 / 8)
    print(value_size, median)
    return fastest, mean, median, percentile_99, tail_latency, throughput_gbps

def perform_analysis(value_sizes, file_types):
    statistics = {value_size: {file_type: [] for file_type in file_types} for value_size in value_sizes}
    statistics['throughput'] = {file_type: [] for file_type in file_types}  # Initialize throughput key

    for value_size in value_sizes:
        for file_type in file_types:
            file_name = f'{file_type}_write_latencies{value_size}.txt'
            if os.path.exists(file_name):
                latencies = read_latencies(file_name)
                num_requests = len(latencies)
                stats = calculate_statistics_and_throughput(latencies, value_size, num_requests)
                statistics[value_size][file_type] = stats[:-1]
                statistics['throughput'][file_type].append(stats[-1])
            else:
                print(f'File {file_name} not found.')

    plot_diagrams(value_sizes, file_types, statistics)

perform_analysis(value_sizes, file_types)