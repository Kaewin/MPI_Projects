# import pandas as pd
# import matplotlib.pyplot as plt
# import numpy as np

# # Read the CSV file
# df = pd.read_csv('results.csv')

# # Set up the figure
# fig, ax = plt.subplots(figsize=(12, 6))

# # Prepare data for grouped bar chart
# x = np.arange(len(df))  # Label locations
# width = 0.35  # Width of bars

# # Create bars
# bars1 = ax.bar(x - width/2, df['Blocking_Time'], width, label='Blocking', color='steelblue')
# bars2 = ax.bar(x + width/2, df['NonBlocking_Time'], width, label='Non-blocking', color='coral')

# # Customize the chart
# ax.set_xlabel('Number of Points', fontsize=12)
# ax.set_ylabel('Completion Time (seconds)', fontsize=12)
# ax.set_title('MPI Monte Carlo Performance: Blocking vs Non-blocking', fontsize=14)

# # Set x-axis labels to show powers of 2
# points_labels = [f'2^{i}' for i in range(10, 10 + len(df))]
# ax.set_xticks(x)
# ax.set_xticklabels(points_labels, rotation=45, ha='right')

# # Add legend
# ax.legend()

# # Add grid for better readability
# ax.yaxis.grid(True, linestyle='--', alpha=0.7)
# ax.set_axisbelow(True)

# # Add value labels on bars if you want
# for bar in bars1:
#     height = bar.get_height()
#     ax.text(bar.get_x() + bar.get_width()/2., height,
#             f'{height:.3f}', ha='center', va='bottom', fontsize=9)

# for bar in bars2:
#     height = bar.get_height()
#     ax.text(bar.get_x() + bar.get_width()/2., height,
#             f'{height:.3f}', ha='center', va='bottom', fontsize=9)

# # Adjust layout to prevent label cutoff
# plt.tight_layout()

# # Save and show
# plt.savefig('monte_carlo_performance.png', dpi=150)
# plt.show()

# # Print summary statistics
# print("\nSummary Statistics:")
# print(f"Points tested: {len(df)}")
# print(f"Max blocking time: {df['Blocking_Time'].max():.3f} seconds")
# print(f"Max non-blocking time: {df['NonBlocking_Time'].max():.3f} seconds")
# print(f"Average difference: {(df['Blocking_Time'] - df['NonBlocking_Time']).mean():.6f} seconds")

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the CSV file
df = pd.read_csv('results.csv')

# Set up the figure - make it wider
fig, ax = plt.subplots(figsize=(16, 8))

# Prepare data for grouped bar chart
x = np.arange(len(df))
width = 0.35

# Create bars
bars1 = ax.bar(x - width/2, df['Blocking_Time'], width, label='Blocking', color='steelblue')
bars2 = ax.bar(x + width/2, df['NonBlocking_Time'], width, label='Non-blocking', color='coral')

# Customize the chart
ax.set_xlabel('Number of Points', fontsize=14)
ax.set_ylabel('Completion Time (seconds)', fontsize=14)
ax.set_title('MPI Monte Carlo Performance: Blocking vs Non-blocking', fontsize=16)

# Set x-axis labels to show actual point counts (more readable)
points_values = [2**i for i in range(10, 10 + len(df))]
ax.set_xticks(x)
ax.set_xticklabels([f'{val:,}' for val in points_values], rotation=45, ha='right', fontsize=10)

# Add legend
ax.legend(fontsize=12)

# Add grid
ax.yaxis.grid(True, linestyle='--', alpha=0.7)
ax.set_axisbelow(True)

# Only add value labels for bars > 1 second (to reduce clutter)
for bar in bars1:
    height = bar.get_height()
    if height > 1:  # Only label significant bars
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{height:.1f}', ha='center', va='bottom', fontsize=9)

for bar in bars2:
    height = bar.get_height()
    if height > 1:  # Only label significant bars
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{height:.1f}', ha='center', va='bottom', fontsize=9)

# Use log scale for y-axis if values span multiple orders of magnitude
if df['Blocking_Time'].max() / df['Blocking_Time'].min() > 100:
    ax.set_yscale('log')
    ax.set_ylabel('Completion Time (seconds) - Log Scale', fontsize=14)

# Adjust layout
plt.tight_layout()

# Save with high DPI
plt.savefig('monte_carlo_performance.png', dpi=150, bbox_inches='tight')
plt.show()

# Print summary
print("\nSummary Statistics:")
print(f"Points tested: {len(df)} configurations")
print(f"Max blocking time: {df['Blocking_Time'].max():.2f} seconds")
print(f"Max non-blocking time: {df['NonBlocking_Time'].max():.2f} seconds")