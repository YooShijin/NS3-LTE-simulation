import numpy as np
import matplotlib.pyplot as plt

# Example throughput data for different schedulers at 0ms and 10ms
throughputs_mt_0ms = np.array([22.9485, 22.9477, 22.9467, 22.9522, 22.9512])
throughputs_mt_10ms = np.array([22.9504, 22.9511, 22.9494, 22.9519, 22.9466])
throughputs_pf_0ms = np.array([22.9481, 22.9473, 22.9464, 22.951, 22.9508])
throughputs_pf_10ms = np.array([22.9483, 22.9507, 22.9491, 22.9515, 22.9463])
throughputs_pss_0ms = np.array([17.0654, 16.5682, 16.5275, 17.0518, 14.9845])
throughputs_pss_10ms = np.array([17.084, 17.0157, 17.0353, 16.5275, 17.0517])
throughputs_rr_0ms = np.array([22.9724, 22.9746, 22.9677, 22.9698, 22.9733])
throughputs_rr_10ms = np.array([22.9724, 22.9634, 22.9719, 22.9688, 22.965])

# Function to calculate sorted throughputs and cumulative probabilities
def calculate_cdf(data):
    sorted_data = np.sort(data)
    cdf = np.arange(len(sorted_data)) / float(len(sorted_data))
    return sorted_data, cdf

# Calculate CDF for each scheduler and speed combination
sorted_mt_0ms, cdf_mt_0ms = calculate_cdf(throughputs_mt_0ms)
sorted_mt_10ms, cdf_mt_10ms = calculate_cdf(throughputs_mt_10ms)
sorted_pf_0ms, cdf_pf_0ms = calculate_cdf(throughputs_pf_0ms)
sorted_pf_10ms, cdf_pf_10ms = calculate_cdf(throughputs_pf_10ms)
sorted_pss_0ms, cdf_pss_0ms = calculate_cdf(throughputs_pss_0ms)
sorted_pss_10ms, cdf_pss_10ms = calculate_cdf(throughputs_pss_10ms)
sorted_rr_0ms, cdf_rr_0ms = calculate_cdf(throughputs_rr_0ms)
sorted_rr_10ms, cdf_rr_10ms = calculate_cdf(throughputs_rr_10ms)

# Plot the CDFs
plt.figure()
plt.plot(sorted_mt_0ms, cdf_mt_0ms, marker='o', linestyle='-', label='MT Throughput 0m/s')
plt.plot(sorted_mt_10ms, cdf_mt_10ms, marker='p', linestyle='-', label='MT Throughput 10m/s')
plt.plot(sorted_pf_0ms, cdf_pf_0ms, marker='o', linestyle='--', label='PF Throughput 0m/s')
plt.plot(sorted_pf_10ms, cdf_pf_10ms, marker='p', linestyle='--', label='PF Throughput 10m/s')
plt.plot(sorted_pss_0ms, cdf_pss_0ms, marker='o', linestyle='-.', label='PSS Throughput 0m/s')
plt.plot(sorted_pss_10ms, cdf_pss_10ms, marker='p', linestyle='-.', label='PSS Throughput 10m/s')
plt.plot(sorted_rr_0ms, cdf_rr_0ms, marker='o', linestyle=':', label='RR Throughput 0m/s')
plt.plot(sorted_rr_10ms, cdf_rr_10ms, marker='p', linestyle=':', label='RR Throughput 10m/s')

plt.xlabel('Throughput (Mbps)')
plt.ylabel('Cumulative Probability')
plt.title('Cumulative Distribution Function (CDF) of Throughput')
plt.legend()
plt.grid(True)
plt.show()

