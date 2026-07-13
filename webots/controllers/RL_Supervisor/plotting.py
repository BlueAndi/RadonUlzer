"""Plot episode-level reinforcement learning training metrics."""

# pylint: disable=duplicate-code

from pathlib import Path
import argparse
import matplotlib.pyplot as plt
import pandas as pd

def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line options.

    Returns
    ----------
        argparse.Namespace: Parsed command-line options.
    """

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

required_columns = {"Training Update", "Mean Actor Loss", "Mean Critic Loss",
                    "Mean Episode Reward", "Mean Episode Steps"}
missing_columns = required_columns.difference(data.columns)
if missing_columns:
    raise ValueError(
        f"Missing columns in {LOG_FILE}: {', '.join(sorted(missing_columns))}"
    )

if data.empty:
    raise ValueError(f"No training data found in {LOG_FILE}.")

# Plotting Actor Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Training Update"], data["Mean Actor Loss"], label="Mean Actor Loss")
plt.xlabel("Training Update")
plt.ylabel("Loss")
plt.title("Mean Actor Loss per Training Update")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "actor_loss_plot.png")

# Plotting Critic Loss
plt.figure(figsize=(10, 5))
plt.plot(data["Training Update"], data["Mean Critic Loss"], label="Mean Critic Loss")
plt.xlabel("Training Update")
plt.ylabel("Loss")
plt.title("Mean Critic Loss per Training Update")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "critic_loss_plot.png")

# Plotting Total Rewards
plt.figure(figsize=(10, 5))
plt.plot(data["Training Update"], data["Mean Episode Reward"], label="Mean Episode Reward")
plt.xlabel("Training Update")
plt.ylabel("Mean Episode Reward")
plt.title("Mean Episode Reward per Training Update")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "total_rewards_plot.png")

# Plotting Mean Episode Steps
plt.figure(figsize=(10, 5))
plt.plot(data["Training Update"], data["Mean Episode Steps"], label="Mean Episode Steps")
plt.xlabel("Training Update")
plt.ylabel("Steps")
plt.title("Mean Episode Steps per Training Update")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(LOG_DIR / "mean_episode_steps_plot.png")

print(f"Plots saved to: {LOG_DIR}")
plt.show()
