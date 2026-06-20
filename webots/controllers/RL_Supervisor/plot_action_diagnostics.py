"""Plot episode-level action quality from the diagnostics log."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_LOG_FILE = SCRIPT_DIR / "logs" / "action_diagnostics.csv"
SENSOR_COLUMNS = [f"Sensor{index}" for index in range(5)]
LINE_CENTER = 2.0


def parse_arguments():
    """Parse command-line options."""
    parser = argparse.ArgumentParser(
        description="Plot mean steering quality per training episode."
    )
    parser.add_argument(
        "--input",
        type=Path,
        default=DEFAULT_LOG_FILE,
        help="Path to action_diagnostics.csv.",
    )
    return parser.parse_args()


def calculate_reference_correction(sensor_values):
    """Calculate the steering reference from the detected line position.

    Sensor 0 is on the left and sensor 4 is on the right. Positive steering
    means left and negative steering means right.
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


def calculate_action_score(actions, reference):
    """Return action agreement from -1 (opposite) to 1 (exact match)."""
    return 1.0 - np.abs(actions - reference)


def main():
    """Aggregate diagnostic samples per episode and create the plot."""
    args = parse_arguments()
    data = pd.read_csv(args.input)

    required_columns = {
        "Episode",
        "Actor Mean",
        "Sampled Action",
        *SENSOR_COLUMNS,
    }
    missing_columns = required_columns.difference(data.columns)
    if missing_columns:
        raise ValueError(
            f"Missing columns in {args.input}: {', '.join(sorted(missing_columns))}"
        )
    if data.empty:
        raise ValueError(f"No diagnostic data found in {args.input}.")

    sensor_values = data[SENSOR_COLUMNS].to_numpy(dtype=float)
    reference = calculate_reference_correction(sensor_values)
    line_visible = np.isfinite(reference)

    evaluated_data = data.loc[line_visible, ["Episode"]].copy()
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

    episode_scores = (
        evaluated_data.groupby("Episode", as_index=False)
        .agg(
            {
                "Actor Mean Score": "mean",
                "Sampled Action Score": "mean",
            }
        )
        .sort_values("Episode")
    )

    figure, axis = plt.subplots(figsize=(12, 6))
    axis.axhline(1.0, color="tab:green", linewidth=1, alpha=0.5)
    axis.axhline(0.0, color="black", linewidth=0.8)
    axis.axhline(-1.0, color="tab:red", linewidth=1, alpha=0.5)
    axis.plot(
        episode_scores["Episode"],
        episode_scores["Actor Mean Score"],
        label="Actor mean score",
        color="tab:blue",
        linewidth=1.5,
    )
    axis.plot(
        episode_scores["Episode"],
        episode_scores["Sampled Action Score"],
        label="Sampled action score",
        color="tab:orange",
        linewidth=1.2,
        alpha=0.85,
    )
    axis.set_ylim(-1.05, 1.05)
    axis.set_xlabel("Episode")
    axis.set_ylabel("Mean action score")
    axis.set_title("Steering quality per episode")
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

    output_file = args.input.parent / "action_diagnostics_episode_scores.png"
    figure.savefig(output_file, dpi=160)
    print(f"Plot saved to: {output_file}")
    plt.show()


if __name__ == "__main__":
    main()
