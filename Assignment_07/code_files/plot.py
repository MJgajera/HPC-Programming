import pandas as pd
import matplotlib.pyplot as plt
import os

# 1. Load the Results Data
try:
    df = pd.read_csv('results.csv')
except FileNotFoundError:
    print("Error: results.csv not found. Please run the bash script first.")
    exit()

configs = df['Config'].unique()

# Create a directory to keep your folder clean
output_dir = "Assignment_Plots"
os.makedirs(output_dir, exist_ok=True)

print(f"Generating plots for {len(configs)} configurations...")

# ==========================================
# Initialize the 3 Combined Master Figures
# ==========================================
fig_exec, ax_exec = plt.subplots(figsize=(10, 6))
fig_speed, ax_speed = plt.subplots(figsize=(10, 6))
fig_phase, ax_phase = plt.subplots(figsize=(12, 7)) # Slightly larger to fit the legend

# Define distinct colors for the 5 configurations for the combined plots
color_map = {'A': 'blue', 'B': 'green', 'C': 'red', 'D': 'purple', 'E': 'darkorange'}

for config in configs:
    # Isolate data for the current configuration
    subset = df[df['Config'] == config]
    
    # Extract the true serial time (1 core) for speedup calculations
    serial_time = subset[subset['Cores'] == 1]['Total_Alg_Time'].values[0]
    
    # Use ALL data points including the 1-core serial benchmark
    cores = subset['Cores'].values
    total_times = subset['Total_Alg_Time'].values
    int_times = subset['Int_Time'].values
    mover_times = subset['Mover_Time'].values
    
    # Calculate Speedup (T_serial / T_parallel)
    speedup = serial_time / total_times
    
    cfg_color = color_map.get(config, 'black')

    # ==========================================
    # ADD TO COMBINED PLOTS
    # ==========================================
    ax_exec.plot(cores, total_times, marker='o', color=cfg_color, linewidth=2, markersize=7, label=f'Config {config}')
    ax_speed.plot(cores, speedup, marker='s', color=cfg_color, linewidth=2, markersize=7, label=f'Config {config}')
    
    # Solid line for Int, Dashed for Mover
    ax_phase.plot(cores, int_times, marker='o', linestyle='-', color=cfg_color, linewidth=2, markersize=6, label=f'Config {config} (Int)')
    ax_phase.plot(cores, mover_times, marker='^', linestyle='--', color=cfg_color, linewidth=2, markersize=6, label=f'Config {config} (Mover)')

    # ==========================================
    # PLOT 1: Individual Execution Time vs Cores
    # ==========================================
    plt.figure(figsize=(9, 6))
    plt.plot(cores, total_times, marker='o', color='dodgerblue', linewidth=2, markersize=8, label=f'Config {config} Total Time')
    plt.title(f'Execution Time vs Cores (Configuration {config})', fontsize=14, fontweight='bold')
    plt.xlabel('Number of Cores', fontsize=12)
    plt.ylabel('Execution Time (Seconds)', fontsize=12)
    plt.xticks(cores)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=11)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/Execution_Time_Config_{config}.png', dpi=300)
    plt.close()

    # ==========================================
    # PLOT 2: Individual Speedup vs Cores
    # ==========================================
    plt.figure(figsize=(9, 6))
    plt.plot(cores, speedup, marker='s', color='forestgreen', linewidth=2, markersize=8, label='Actual Speedup')
    plt.plot(cores, cores, linestyle='--', color='black', linewidth=2, label='Ideal Speedup (Linear)')
    plt.title(f'Speedup vs Cores (Configuration {config})', fontsize=14, fontweight='bold')
    plt.xlabel('Number of Cores', fontsize=12)
    plt.ylabel('Speedup (S = T1 / Tn)', fontsize=12)
    plt.xticks(cores)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=11)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/Speedup_Config_{config}.png', dpi=300)
    plt.close()

    # ==========================================
    # PLOT 3: Individual Phase Bottleneck Analysis
    # ==========================================
    plt.figure(figsize=(9, 6))
    plt.plot(cores, int_times, marker='o', color='crimson', linewidth=2, markersize=8, label='Interpolation Time (Scatter)')
    plt.plot(cores, mover_times, marker='^', color='indigo', linewidth=2, markersize=8, label='Mover Time (Gather)')
    plt.title(f'Phase Bottleneck Analysis (Configuration {config})', fontsize=14, fontweight='bold')
    plt.xlabel('Number of Cores', fontsize=12)
    plt.ylabel('Execution Time (Seconds)', fontsize=12)
    plt.xticks(cores)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(loc='center right', fontsize=11)
    
    avg_int_percentage = (int_times.sum() / (int_times.sum() + mover_times.sum())) * 100
    plt.text(0.95, 0.85, f'Interpolation accounts for ~{avg_int_percentage:.1f}% of core logic time', 
             transform=plt.gca().transAxes, fontsize=11, horizontalalignment='right',
             bbox=dict(facecolor='white', alpha=0.9, edgecolor='black'))
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/Phase_Analysis_Config_{config}.png', dpi=300)
    plt.close()

# ==========================================
# FINALIZE & SAVE COMBINED PLOTS
# ==========================================

# 16. Combined Execution Time
ax_exec.set_title('Combined Execution Time vs Cores', fontsize=14, fontweight='bold')
ax_exec.set_xlabel('Number of Cores', fontsize=12)
ax_exec.set_ylabel('Execution Time (Seconds)', fontsize=12)
ax_exec.set_xticks(cores)
ax_exec.grid(True, linestyle='--', alpha=0.7)
ax_exec.legend(fontsize=11)
fig_exec.tight_layout()
fig_exec.savefig(f'{output_dir}/Combined_Execution_Time.png', dpi=300)
plt.close(fig_exec)

# 17. Combined Speedup
ax_speed.plot(cores, cores, linestyle='--', color='black', linewidth=2, label='Ideal Speedup')
ax_speed.set_title('Combined Speedup vs Cores', fontsize=14, fontweight='bold')
ax_speed.set_xlabel('Number of Cores', fontsize=12)
ax_speed.set_ylabel('Speedup (S = T1 / Tn)', fontsize=12)
ax_speed.set_xticks(cores)
ax_speed.grid(True, linestyle='--', alpha=0.7)
ax_speed.legend(fontsize=11)
fig_speed.tight_layout()
fig_speed.savefig(f'{output_dir}/Combined_Speedup.png', dpi=300)
plt.close(fig_speed)

# 18. Combined Phase Analysis
ax_phase.set_title('Combined Phase Analysis (Interpolation vs Mover)', fontsize=14, fontweight='bold')
ax_phase.set_xlabel('Number of Cores', fontsize=12)
ax_phase.set_ylabel('Execution Time (Seconds)', fontsize=12)
ax_phase.set_xticks(cores)
ax_phase.grid(True, linestyle='--', alpha=0.7)
# Put legend outside the plot area so it doesn't cover the data lines
ax_phase.legend(bbox_to_anchor=(1.04, 1), loc="upper left", fontsize=10)
fig_phase.tight_layout()
fig_phase.savefig(f'{output_dir}/Combined_Phase_Analysis.png', dpi=300)
plt.close(fig_phase)

print(f"Success! All 18 high-resolution plots have been generated and updated.")
