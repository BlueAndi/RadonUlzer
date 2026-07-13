"""Compare reinforcement learning training metrics across runs."""

import argparse
import json
from datetime import datetime
from pathlib import Path

import matplotlib
import pandas as pd

matplotlib.use("Agg")
# pylint: disable=wrong-import-position,ungrouped-imports
import matplotlib.pyplot as plt

# Directories used to read training runs and write comparisons.
SCRIPT_DIR = Path(__file__).resolve().parent
LOGS_DIR = SCRIPT_DIR / "logs"
TRAINING_RUNS_DIR = SCRIPT_DIR / "training_runs"
COMPARISONS_DIR = LOGS_DIR / "comparisons"

# Minimum number of runs required for a meaningful comparison.
MINIMUM_RUN_COUNT = 2

# Columns required in every training log before metrics can be compared.
REQUIRED_COLUMNS = {
    "Training Update",
    "Mean Actor Loss",
    "Mean Critic Loss",
    "Mean Episode Reward",
    "Mean Episode Steps",
}

# Plot definitions keyed by comparison metric.
METRICS = {
    "reward": {
        "x_column": "Training Update",
        "y_column": "Mean Episode Reward",
        "filename": "reward_comparison.png",
        "title": "Reward Comparison",
        "xlabel": "Training Update",
        "ylabel": "Mean Episode Reward",
    },
    "mean_episode_steps": {
        "x_column": "Training Update",
        "y_column": "Mean Episode Steps",
        "filename": "mean_episode_steps_comparison.png",
        "title": "Mean Episode Steps Comparison",
        "xlabel": "Training Update",
        "ylabel": "Mean Episode Steps",
    },
    "actor_loss": {
        "x_column": "Training Update",
        "y_column": "Mean Actor Loss",
        "filename": "actor_loss_comparison.png",
        "title": "Actor Loss Comparison",
        "xlabel": "Training Update",
        "ylabel": "Actor Loss",
    },
    "critic_loss": {
        "x_column": "Training Update",
        "y_column": "Mean Critic Loss",
        "filename": "critic_loss_comparison.png",
        "title": "Critic Loss Comparison",
        "xlabel": "Training Update",
        "ylabel": "Mean Critic Loss",
    },
}


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line options.

    Returns
    ----------
        argparse.Namespace: Parsed command-line arguments.
    """

    parser = argparse.ArgumentParser(
        description=(
            "Compare reinforcement learning training metrics across two or more "
            "runs stored in the logs directory."
        ),
        epilog=(
            "Example: python compare_runs.py --runs "
            "2026-06-30_10-49-54 2026-06-30_10-53-34"
        ),
    )
    parser.add_argument(
        "--runs",
        nargs="+",
        required=True,
        help=(
            "Run directory names relative to logs/. Each run must contain "
            "training_logs.csv and run_configuration.json."
        ),
    )
    parser.add_argument(
        "--training-experiment",
        help="Experiment directory name below training_runs/ for parallel runs.",
    )
    args = parser.parse_args()

    if len(args.runs) < MINIMUM_RUN_COUNT:
        parser.error("At least two runs are required.")

    return args


def load_run(run_name, training_experiment=None) -> dict:
    """
    Load metrics and configuration for one run.

    Parameters
    ----------
        run_name: Name of the run directory.
        training_experiment: Optional parallel training experiment name.

    Returns
    ----------
        dict: Run name, training data, and run configuration.
    """

    if training_experiment:
        run_dir = TRAINING_RUNS_DIR / training_experiment / run_name / "logs"
    else:
        run_dir = LOGS_DIR / run_name
    if not run_dir.is_dir():
        raise FileNotFoundError(f"Run directory not found: {run_dir}")

    training_log = run_dir / "training_logs.csv"
    config_file = run_dir / "run_configuration.json"
    if not training_log.is_file():
        raise FileNotFoundError(f"Training log not found for {run_name}: {training_log}")
    if not config_file.is_file():
        raise FileNotFoundError(f"Run configuration not found for {run_name}: {config_file}")

    data = pd.read_csv(training_log)
    missing_columns = REQUIRED_COLUMNS.difference(data.columns)
    if missing_columns:
        raise ValueError(
            f"Missing columns in {training_log}: {', '.join(sorted(missing_columns))}"
        )
    if data.empty:
        raise ValueError(f"No training data found in {training_log}.")

    with open(config_file, mode="r", encoding="utf-8") as file:
        config = json.load(file)
    if not config:
        raise ValueError(f"No run configuration found in {config_file}.")

    return {"name": run_name, "training_data": data, "config": config}


def build_output_dir(training_experiment=None) -> Path:
    """
    Build the comparison output directory.

    Parameters
    ----------
        training_experiment: Optional parallel training experiment name.

    Returns
    ----------
        Path: Directory where comparison outputs should be written.
    """

    if training_experiment:
        return TRAINING_RUNS_DIR / training_experiment / "comparisons"

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    return COMPARISONS_DIR / timestamp


def flatten_config(config, prefix="") -> dict:
    """
    Flatten nested dictionaries into dotted parameter names.

    Parameters
    ----------
        config: Configuration dictionary to flatten.
        prefix: Prefix prepended to nested parameter names.

    Returns
    ----------
        dict: Flattened configuration values indexed by dotted parameter names.
    """

    flattened = {}
    for key, value in config.items():
        parameter_name = f"{prefix}.{key}" if prefix else key
        if isinstance(value, dict):
            flattened.update(flatten_config(value, parameter_name))
        else:
            flattened[parameter_name] = value
    return flattened


def find_parameter_differences(runs) -> dict:
    """
    Find configuration values that differ between runs.

    Parameters
    ----------
        runs: Loaded run data containing name, training data, and configuration.

    Returns
    ----------
        dict: Differing configuration values indexed by parameter name.
    """

    flattened_configs = {
        run["name"]: flatten_config(run["config"]) for run in runs
    }
    parameter_names = set().union(*flattened_configs.values())
    differences = {}

    for parameter_name in sorted(parameter_names):
        values = {
            run_name: config.get(parameter_name, "<missing>")
            for run_name, config in flattened_configs.items()
        }
        # Use repr() so list values can be compared in a set.
        if len({repr(value) for value in values.values()}) > 1:
            differences[parameter_name] = values

    return differences


def write_parameter_differences(runs, output_dir) -> None:
    """
    Write differing run configuration values to a text file.

    Parameters
    ----------
        runs: Loaded run data containing name, training data, and configuration.
        output_dir: Directory where the text file should be written.

    Returns
    ----------
        None
    """

    differences = find_parameter_differences(runs)
    output_file = output_dir / "parameter_differences.txt"

    with open(output_file, mode="w", encoding="utf-8") as file:
        file.write("Compared runs:\n")
        for run in runs:
            file.write(f"- {run['name']}\n")
        file.write("\nDifferent parameters:\n\n")

        if not differences:
            file.write("No parameter differences found.\n")
            return

        for parameter_name, values in differences.items():
            file.write(f"{parameter_name}:\n\n")
            for run_name, value in values.items():
                file.write(f"  {run_name}: {value}\n")
            file.write("\n")


def plot_metric_comparisons(runs, output_dir) -> None:
    """
    Create one comparison plot for each configured metric.

    Parameters
    ----------
        runs: Loaded run data containing name, training data, and configuration.
        output_dir: Directory where plot files should be written.

    Returns
    ----------
        None
    """

    for metric in METRICS.values():
        plt.figure(figsize=(10, 6))
        for run in runs:
            training_data = run["training_data"]
            plt.plot(
                training_data[metric["x_column"]],
                training_data[metric["y_column"]],
                label=run["name"],
            )

        plt.title(metric["title"])
        plt.xlabel(metric["xlabel"])
        plt.ylabel(metric["ylabel"])
        plt.grid(True)
        plt.legend()
        plt.tight_layout()
        plt.savefig(output_dir / metric["filename"])
        plt.close()


def main() -> None:
    """
    Run the comparison.

    Returns
    ----------
        None
    """

    args = parse_arguments()
    runs = [load_run(run_name, args.training_experiment) for run_name in args.runs]
    output_dir = build_output_dir(args.training_experiment)
    output_dir.mkdir(parents=True, exist_ok=True)
    write_parameter_differences(runs, output_dir)
    plot_metric_comparisons(runs, output_dir)
    print(f"Comparison written to: {output_dir}")


if __name__ == "__main__":
    main()
