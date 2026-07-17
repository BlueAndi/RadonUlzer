"""Plot episode-level action quality from the diagnostics log."""

# MIT License
#
# Copyright (c) 2026 Andreas Merkle <web@blue-andi.de>
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

import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

################################################################################
# Variables
################################################################################

# pylint: disable=duplicate-code

SCRIPT_DIR = Path(__file__).resolve().parent
SENSOR_COLUMNS = [f"Sensor{index}" for index in range(5)]
LINE_CENTER = 2.0

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
        description="Plot mean steering quality per training episode."
    )
    parser.add_argument(
        "--run",
        type=Path,
        required = True,
        help="Name of the run directory below logs.",
    )

    return parser.parse_args()


def calculate_reference_correction(sensor_values) -> np.ndarray:
    """
    Calculate steering references from detected line positions.

    Parameters
    ----------
        sensor_values: Line sensor values for each diagnostic sample.

    Returns
    ----------
        np.ndarray: Normalized steering reference for each diagnostic sample.
    """

    sensor_sum = sensor_values.sum(axis=1)
    weighted_position = sensor_values @ np.arange(len(SENSOR_COLUMNS))
    line_position = np.divide(
        weighted_position,
        sensor_sum,
        out=np.full(sensor_sum.shape, np.nan, dtype=float),
        where=sensor_sum > 0,
    )
    return (LINE_CENTER - line_position) / LINE_CENTER


def calculate_action_score(actions, reference) -> np.ndarray:
    """
    Calculate agreement between sampled actions and steering references.

    Parameters
    ----------
        actions: Sampled steering actions for each diagnostic sample.
        reference: Steering references for each diagnostic sample.

    Returns
    ----------
        np.ndarray: Agreement score for each diagnostic sample.
    """

    return 1.0 - np.abs(actions - reference)


def main() -> None:
    """
    Aggregate diagnostic samples per episode and create the plot.

    Returns
    ----------
        None
    """

    args = parse_arguments()
    log_file = SCRIPT_DIR / "logs" / args.run / "action_diagnostics.csv"
    data = pd.read_csv(log_file)

    required_columns = {
        "Training Update",
        "Episode",
        "Actor Mean",
        "Sampled Action",
        *SENSOR_COLUMNS,
    }
    missing_columns = required_columns.difference(data.columns)
    if missing_columns:
        raise ValueError(
            f"Missing columns in {log_file}: {', '.join(sorted(missing_columns))}"
        )
    if data.empty:
        raise ValueError(f"No diagnostic data found in {log_file}.")

    sensor_values = data[SENSOR_COLUMNS].to_numpy(dtype=float)
    reference = calculate_reference_correction(sensor_values)
    line_visible = np.isfinite(reference)

    evaluated_data = data.loc[line_visible, ["Training Update", "Episode"]].copy()
    evaluated_data["Actor Mean Score"] = calculate_action_score(
        data.loc[line_visible, "Actor Mean"].to_numpy(dtype=float),
        reference[line_visible],
    )
    evaluated_data["Sampled Action Score"] = calculate_action_score(
        data.loc[line_visible, "Sampled Action"].to_numpy(dtype=float),
        reference[line_visible],
    )

    if evaluated_data.empty:
        raise ValueError("No samples with a visible line found in the diagnostics log.")

    episode_scores = evaluated_data.groupby(
        ["Training Update", "Episode"], as_index=False
    ).agg(
        {
            "Actor Mean Score": "mean",
            "Sampled Action Score": "mean",
        }
    )
    training_update_scores = (
        episode_scores.groupby("Training Update", as_index=False)
        .agg(
            {
                "Actor Mean Score": "mean",
                "Sampled Action Score": "mean",
            }
        )
        .sort_values("Training Update")
    )

    figure, axis = plt.subplots(figsize=(12, 6))
    axis.axhline(1.0, color="tab:green", linewidth=1, alpha=0.5)
    axis.axhline(0.0, color="black", linewidth=0.8)
    axis.axhline(-1.0, color="tab:red", linewidth=1, alpha=0.5)
    axis.plot(
        training_update_scores["Training Update"],
        training_update_scores["Actor Mean Score"],
        label="Actor mean score",
        color="tab:blue",
        linewidth=1.5,
    )
    axis.plot(
        training_update_scores["Training Update"],
        training_update_scores["Sampled Action Score"],
        label="Sampled action score",
        color="tab:orange",
        linewidth=1.2,
        alpha=0.85,
    )
    axis.set_ylim(-1.05, 1.05)
    axis.set_xlabel("Training Update")
    axis.set_ylabel("Mean action score")
    axis.set_title("Steering quality per training update")
    axis.grid(True, alpha=0.3)
    axis.legend(loc="best")
    axis.text(
        1.01,
        0.5,
        "+1 = exact reference\n"
        " 0 = large deviation\n"
        "-1 = fully opposite",
        transform=axis.transAxes,
        va="center",
    )
    figure.tight_layout()

    output_file = log_file.parent / "action_diagnostics_training_update_scores.png"
    figure.savefig(output_file, dpi=160)
    print(f"Plot saved to: {output_file}")
    plt.show()

################################################################################
# Main
################################################################################

if __name__ == "__main__":
    main()
