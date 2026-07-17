"""Plot episode-level reinforcement learning training metrics."""

# MIT License
#
# Copyright (c) 2024 - 2026 Andreas Merkle <web@blue-andi.de>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#


################################################################################
# Imports
################################################################################

from pathlib import Path
import argparse
import matplotlib.pyplot as plt
import pandas as pd

################################################################################
# Variables
################################################################################

# pylint: disable=duplicate-code

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

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

################################################################################
# Main
################################################################################

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
