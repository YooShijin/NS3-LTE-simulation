import numpy as np
import matplotlib.pyplot as plt


# MT



# Load data from text files
time_thp_mt, throughput_mt = np.loadtxt('Throughput_MT_S10.txt', unpack=True)
time_sinr_mt, sinr_mt = np.loadtxt('mt-SINR_S10.txt', unpack=True)

# Create the plot 1
plt.figure()
plt.plot(time_thp_mt, throughput_mt, linestyle='-', marker='', label='Throughput', color='blue')
plt.plot(time_sinr_mt, sinr_mt, linestyle='-', marker='', label='SINR', color='red')

# Set plot titles and labels
plt.title("SINR and Instantaneous Throughput Graph - MT 10m/s")
plt.xlabel("Time")
plt.ylabel("Value")
plt.legend()

# Save and show the plot
plt.savefig("ThroughPut_MT10.png")
plt.show()




# PSS 

# Load data from text files
time_thp_pss, throughput_pss = np.loadtxt('Throughput_PSS_S10.txt', unpack=True)
time_sinr_pss, sinr_pss = np.loadtxt('pssf-SINR_S10.txt', unpack=True)

# Create the plot 2 
plt.figure()
plt.plot(time_thp_pss, throughput_pss, linestyle='-', marker='', label='Throughput', color='blue')
plt.plot(time_sinr_pss, sinr_pss, linestyle='-', marker='', label='SINR', color='red')

# Set plot titles and labels
plt.title("SINR and Instantaneous Throughput Graph - PSS 10m/s")
plt.xlabel("Time")
plt.ylabel("Value")
plt.legend()

# Save and show the plot
plt.savefig("ThroughPut_PSS10.png")
plt.show()



# PF


# Load data from text files
time_thp_pf, throughput_pf = np.loadtxt('Throughput_PFF_S10.txt', unpack=True)
time_sinr_pf, sinr_pf = np.loadtxt('pf-SINR_S10.txt', unpack=True)

# Create the plot 2 
plt.figure()
plt.plot(time_thp_pf, throughput_pf, linestyle='-', marker='', label='Throughput', color='blue')
plt.plot(time_sinr_pf, sinr_pf, linestyle='-', marker='', label='SINR', color='red')

# Set plot titles and labels
plt.title("SINR and Instantaneous Throughput Graph - PF 10m/s")
plt.xlabel("Time")
plt.ylabel("Value")
plt.legend()

# Save and show the plot
plt.savefig("ThroughPut_PF10.png")
plt.show()




# RR


# Load data from text files
time_thp_rr, throughput_rr = np.loadtxt('Throughput_RR_S10.txt', unpack=True)
time_sinr_rr, sinr_rr = np.loadtxt('rr-SINR_S10.txt', unpack=True)

# Create the plot 2 
plt.figure()
plt.plot(time_thp_rr, throughput_rr, linestyle='-', marker='', label='Throughput', color='blue')
plt.plot(time_sinr_rr, sinr_rr, linestyle='-', marker='', label='SINR', color='red')

# Set plot titles and labels
plt.title("SINR and Instantaneous Throughput Graph - PSS 10m/s")
plt.xlabel("Time")
plt.ylabel("Value")
plt.legend()

# Save and show the plot
plt.savefig("ThroughPut_RR10.png")
plt.show()
