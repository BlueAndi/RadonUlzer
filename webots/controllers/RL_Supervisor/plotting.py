""" Plotting script  with Matplotlib """

# Imports
import matplotlib.pyplot as plt
import pandas as pd

# Define the path to the CSV file
LOG_FILE = "logs/training_logs.csv"

# Read the CSV file
data = pd.read_csv(LOG_FILE)

# Define length
data["Mini Batch"] = range(1, len(data) + 1)

# Plotting Actor Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Mini Batch"], data["Actor Loss"], label="Actor Loss")
plt.xlabel("Mini Batch")
plt.ylabel("Loss")
plt.title("Actor Loss Over Mini Batches")
plt.legend()
plt.grid(True)
plt.savefig("logs/actor_loss_plot.png")
plt.show()

# Plotting Critic Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Mini Batch"], data["Critic Loss"], label="Critic Loss")
plt.xlabel("Mini Batch")
plt.ylabel("Loss")
plt.title("Critic Loss Over Mini Batches")
plt.legend()
plt.grid(True)
plt.savefig("logs/critic_loss_plot.png")
plt.show()

# Plotting Total Rewards
plt.figure(figsize=(10, 5))
plt.plot(data["Mini Batch"], data["Reward"], label="Reward")
plt.xlabel("Mini Batch")
plt.ylabel("Total Reward")
plt.title("Total Rewards Over Mini Batches")
plt.legend()
plt.grid(True)
plt.savefig("logs/total_rewards_plot.png")
plt.show()
