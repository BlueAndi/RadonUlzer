"""Plot episode-level reinforcement learning training metrics."""

from pathlib import Path
import matplotlib.pyplot as plt
import pandas as pd
import argparse

def parse_arguments():
    """Parse command-line options."""
    parser = argparse.ArgumentParser(
        description="Plot episode-level reinforcement learning training metrics."
    )
    parser.add_argument(
        "--run",
        type=Path,
        required = True,
        help="Name of the run directory below logs.",
    )

    return parser.parse_args()

args = parse_arguments()

SCRIPT_DIR = Path(__file__).resolve().parent
LOG_DIR = SCRIPT_DIR / "logs" / args.run
LOG_FILE = LOG_DIR / "training_logs.csv"

data = pd.read_csv(LOG_FILE)

required_columns = {"Episode", "Actor Loss", "Critic Loss", "Reward"}
missing_columns = required_columns.difference(data.columns)
if missing_columns:
    raise ValueError(
        f"Missing columns in {LOG_FILE}: {', '.join(sorted(missing_columns))}"
    )

if data.empty:
    raise ValueError(f"No training data found in {LOG_FILE}.")

# Plotting Actor Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Episode"], data["Actor Loss"], label="Mean Actor Loss")
plt.xlabel("Episode")
plt.ylabel("Loss")
plt.title("Mean Actor Loss per Episode")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "actor_loss_plot.png")

# Plotting Critic Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Episode"], data["Critic Loss"], label="Mean Critic Loss")
plt.xlabel("Episode")
plt.ylabel("Loss")
plt.title("Mean Critic Loss per Episode")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "critic_loss_plot.png")

# Plotting Total Rewards
plt.figure(figsize=(10, 5))
plt.plot(data["Episode"], data["Reward"], label="Total Reward")
plt.xlabel("Episode")
plt.ylabel("Total Reward")
plt.title("Total Reward per Episode")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "total_rewards_plot.png")

print(f"Plots saved to: {LOG_DIR}")
plt.show()
