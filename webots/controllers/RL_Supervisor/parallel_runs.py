"""Start parallel RL training runs through the PlatformIO Webots launcher."""

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
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

################################################################################
# Variables
################################################################################

# Environment variables passed to Webots and the RL supervisor.
MAX_TRAINING_UPDATES_ENV = "RL_MAX_TRAINING_UPDATES"
RUN_DIRECTORY_ENV = "RL_RUN_DIRECTORY"
ACTOR_LEARNING_RATE_ENV = "RL_ACTOR_LEARNING_RATE"
CRITIC_LEARNING_RATE_ENV = "RL_CRITIC_LEARNING_RATE"
GAMMA_ENV = "RL_GAMMA"
GAE_LAMBDA_ENV = "RL_GAE_LAMBDA"
POLICY_CLIP_ENV = "RL_POLICY_CLIP"
BATCH_SIZE_ENV = "RL_BATCH_SIZE"
N_EPOCHS_ENV = "RL_N_EPOCHS"
STD_DEV_ENV = "RL_STD_DEV"
MIN_STD_DEV_ENV = "RL_MIN_STD_DEV"
STD_DEV_FACTOR_ENV = "RL_STD_DEV_FACTOR"
WEBOTS_PORT_ENV = "WEBOTS_PORT"
PLATFORMIO_BUILD_DIR_ENV = "PLATFORMIO_BUILD_DIR"

# Defaults and validation limits for parallel run planning.
DEFAULT_BASE_PORT = 1234
MINIMUM_RUN_COUNT = 2

# Maps JSON run parameters to the environment variables read by rl_supervisor.py.
PARAMETER_TO_ENV = {
    "actor_learning_rate": ACTOR_LEARNING_RATE_ENV,
    "critic_learning_rate": CRITIC_LEARNING_RATE_ENV,
    "gamma": GAMMA_ENV,
    "gae_lambda": GAE_LAMBDA_ENV,
    "policy_clip": POLICY_CLIP_ENV,
    "batch_size": BATCH_SIZE_ENV,
    "n_epochs": N_EPOCHS_ENV,
    "std_dev": STD_DEV_ENV,
    "min_std_dev": MIN_STD_DEV_ENV,
    "std_dev_factor": STD_DEV_FACTOR_ENV,
}

# Project paths used to launch Webots and resolve training configuration files.
PROJECT_ROOT = Path(__file__).resolve().parents[3]
RL_SUPERVISOR_DIR = PROJECT_ROOT / "webots" / "controllers" / "RL_Supervisor"
TRAINING_CONFIGS_DIR = RL_SUPERVISOR_DIR / "training_configs"
WORLD_FILE = PROJECT_ROOT / "webots" / "worlds" / "RL_LineFollower.wbt"

# PlatformIO target that starts the external Webots controller.
PLATFORMIO_COMMAND = [
    "platformio",
    "run",
    "--target",
    "webots_launcher_zumo_com_system",
    "--environment",
    "RemoteControlSim",
]

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def get_webots_command(webots_port) -> list[str]:
    """
    Build the Webots command for one isolated simulator instance.

    Parameters
    ----------
        webots_port: Webots port for this simulator instance.

    Returns
    ----------
        list[str]: Webots command and arguments.
    """

    webots_home = Path(os.environ["WEBOTS_HOME"])
    webots_executable = webots_home / "msys64" / "mingw64" / "bin" / "webots.exe"
    return [
        str(webots_executable),
        "--port=" + str(webots_port),
        "--no-rendering",
        "--minimize",
        "--batch",
        "--clear-cache",
        str(WORLD_FILE),
    ]


def positive_int(value) -> int:
    """
    Parse a positive integer command-line value.

    Parameters
    ----------
        value: Command-line value to parse.

    Returns
    ----------
        int: Parsed positive integer value.
    """

    parsed_value = int(value)
    if parsed_value <= 0:
        raise argparse.ArgumentTypeError("value must be a positive integer")
    return parsed_value


def parse_arguments() -> argparse.Namespace:
    """
    Parse command-line options.

    Returns
    ----------
        argparse.Namespace: Parsed command-line arguments.
    """

    parser = argparse.ArgumentParser(description="Start parallel RL training runs.")
    parser.add_argument(
        "--max-training-updates",
        type=positive_int,
        help="Stop the run after this number of completed training updates.",
    )
    parser.add_argument(
        "--experiment-name",
        help="Name of the experiment folder below training_runs/.",
    )
    parser.add_argument(
        "--num-runs",
        type=positive_int,
        help="Number of parallel runs to plan.",
    )
    parser.add_argument(
        "--base-port",
        type=positive_int,
        default=DEFAULT_BASE_PORT,
        help="First Webots port used for parallel run planning.",
    )
    parser.add_argument(
        "--config",
        type=Path,
        help="JSON configuration file with explicit training runs.",
    )

    args = parser.parse_args()
    if not args.config and (
        not args.experiment_name or not args.max_training_updates or not args.num_runs
    ):
        parser.error(
            "--experiment-name, --max-training-updates, and --num-runs are required "
            "when --config is not used."
        )
    return args


def load_training_config(config_path) -> dict:
    """
    Load a config file from a path or from training_configs/.

    Parameters
    ----------
        config_path: Absolute config path or file name below training_configs/.

    Returns
    ----------
        dict: Loaded training configuration.
    """

    resolved_config_path = (
        config_path
        if config_path.is_file()
        else TRAINING_CONFIGS_DIR / config_path
    )

    with open(resolved_config_path, mode="r", encoding="utf-8") as config_file:
        return json.load(config_file)


def build_training_config(args) -> dict:
    """
    Load a JSON config or build a simple config from CLI arguments.

    Parameters
    ----------
        args: Parsed command-line arguments.

    Returns
    ----------
        dict: Training configuration used to start the runs.
    """

    if args.config:
        training_config = load_training_config(args.config)
        if len(training_config["runs"]) < MINIMUM_RUN_COUNT:
            raise ValueError("At least two runs are required.")
        return training_config

    if args.num_runs < MINIMUM_RUN_COUNT:
        raise ValueError("At least two runs are required.")

    return {
        "experiment_name": args.experiment_name,
        "max_training_updates": args.max_training_updates,
        "base_port": args.base_port,
        "runs": [
            {"name": f"run_{run_index + 1:03d}"}
            for run_index in range(args.num_runs)
        ],
    }


def build_experiment_directory(experiment_name) -> Path:
    """
    Build the wrapper-controlled experiment directory.

    Parameters
    ----------
        experiment_name: Name of the experiment directory.

    Returns
    ----------
        Path: Experiment directory below training_runs/.
    """

    return RL_SUPERVISOR_DIR / "training_runs" / experiment_name


def build_run_directory(experiment_directory, run_name) -> Path:
    """
    Build the wrapper-controlled run directory.

    Parameters
    ----------
        experiment_directory: Directory of the parent experiment.
        run_name: Name of the run directory.

    Returns
    ----------
        Path: Run directory below the experiment directory.
    """

    return experiment_directory / run_name


def cleanup_build_directory(run_directory) -> None:
    """
    Remove the temporary PlatformIO build directory of a run.

    Parameters
    ----------
        run_directory: Directory of the finished run.

    Returns
    ----------
        None
    """

    build_directory = run_directory / ".pio_build"
    try:
        shutil.rmtree(build_directory)
    except FileNotFoundError:
        pass
    except OSError as error:
        print(f"Warning: failed to remove {build_directory}: {error}")


# pylint: disable=consider-using-with
def run_training(max_training_updates, experiment_directory, run_config, webots_port) -> tuple:
    """
    Start one run with isolated paths, port, and runtime parameters.

    Parameters
    ----------
        max_training_updates: Number of completed training updates before stopping.
        experiment_directory: Directory of the parent experiment.
        run_config: Configuration of the run to start.
        webots_port: Webots port for this simulator instance.

    Returns
    ----------
        tuple: Started processes, process log, run directory, run name, and port.
    """

    env = os.environ.copy()
    run_name = run_config["name"]
    run_directory = build_run_directory(experiment_directory, run_name)

    # Child processes inherit these values; rl_supervisor.py reads RL_* at startup.
    env[MAX_TRAINING_UPDATES_ENV] = str(max_training_updates)
    env[RUN_DIRECTORY_ENV] = str(run_directory)
    env[WEBOTS_PORT_ENV] = str(webots_port)

    # Keep PlatformIO build output per run to avoid parallel Controller.dll conflicts.
    env[PLATFORMIO_BUILD_DIR_ENV] = str(run_directory / ".pio_build")

    parameters = run_config.get("parameters", {})
    for parameter_name, env_name in PARAMETER_TO_ENV.items():
        if parameter_name in parameters:
            env[env_name] = str(parameters[parameter_name])

    run_directory.mkdir(parents=True, exist_ok=True)

    # These resources remain open until main() waits for both child processes.
    process_log = open(run_directory / "process.log", "w", encoding="utf-8")

    webots_process = subprocess.Popen(
        get_webots_command(webots_port),
        env=env,
        cwd=PROJECT_ROOT,
        stdout=process_log,
        stderr=subprocess.STDOUT,
    )
    platformio_process = subprocess.Popen(
        PLATFORMIO_COMMAND,
        env=env,
        cwd=PROJECT_ROOT,
        stdout=process_log,
        stderr=subprocess.STDOUT,
    )
    return (
        webots_process,
        platformio_process,
        process_log,
        run_directory,
        run_name,
        webots_port,
    )

# pylint: disable=too-many-locals
def main() -> int:
    """
    Start all configured runs and compare successful experiments.

    Returns
    ----------
        int: Highest exit code returned by the started processes.
    """

    args = parse_arguments()
    training_config = build_training_config(args)
    experiment_name = training_config["experiment_name"]
    max_training_updates = training_config["max_training_updates"]
    base_port = training_config["base_port"]
    runs = training_config["runs"]
    experiment_directory = build_experiment_directory(experiment_name)
    if experiment_directory.exists():
        print(f"Experiment directory already exists: {experiment_directory}")
        return 1

    processes = []
    run_names = []

    for run_index, run_config in enumerate(runs):
        run_name = run_config["name"]
        webots_port = base_port + run_index
        print(f"{run_name} -> port {webots_port}")
        run_names.append(run_name)
        processes.append(
            run_training(
                max_training_updates,
                experiment_directory,
                run_config,
                webots_port,
            )
        )

    return_codes = []
    print("\nRun summary:")

    for (
        webots_process,
        platformio_process,
        process_log,
        run_directory,
        run_name,
        webots_port,
    ) in processes:
        platformio_return_code = platformio_process.wait()
        webots_return_code = webots_process.wait()
        return_codes.extend([platformio_return_code, webots_return_code])
        process_log.close()
        cleanup_build_directory(run_directory)

        status = (
            "OK"
            if platformio_return_code == 0 and webots_return_code == 0
            else "FAILED"
        )

        print(
            f"{run_name} port={webots_port} status={status} "
            f"platformio={platformio_return_code} webots={webots_return_code}"
        )
        if status != "OK":
            print(f"  log: {run_directory / 'process.log'}")

    # Compare only complete experiments so missing logs do not break comparison.
    if max(return_codes, default=1) == 0:
        completed_process = subprocess.run(
            [
                sys.executable,
                str(RL_SUPERVISOR_DIR / "compare_runs.py"),
                "--training-experiment",
                experiment_name,
                "--runs",
                *run_names,
            ],
            check=False,
        )
        return_codes.append(completed_process.returncode)

    return max(return_codes, default=1)

################################################################################
# Main
################################################################################

if __name__ == "__main__":
    raise SystemExit(main())
