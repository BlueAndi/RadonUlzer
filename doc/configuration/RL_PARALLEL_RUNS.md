# RL Parallel Runs

This document explains how to start multiple RL supervisor training runs in
parallel and how to configure run-specific hyperparameters.

## Purpose

`parallel_runs.py` starts multiple independent Webots RL training runs in
parallel. Each run can use its own hyperparameter configuration and writes logs,
models, process output, and comparison data into isolated run directories.

The main goal is to make hyperparameter experiments reproducible and comparable.
This is the foundation for a later automated hyperparameter search, where
candidate configurations can be generated, trained in parallel, evaluated, and
used to select the next search step.

## Prerequisites

Make sure the following setup is available:

- Webots is installed.
- `WEBOTS_HOME` points to the Webots installation directory.
- `platformio` is available on `PATH`.
- The RL supervisor Python environment is active.

See `RL_ENVIRONMENT.md` for the Python environment setup.

In an open terminal, change to the RL supervisor directory:

```powershell
cd webots/controllers/RL_Supervisor
```

## Basic Usage

Show the available command-line arguments:

```powershell
python .\parallel_runs.py --help
```

Start multiple runs without a config file:

```powershell
python .\parallel_runs.py --experiment-name test_parallel --max-training-updates 5 --num-runs 2
```

This creates two runs with generated names:

```text
run_001
run_002
```

The first Webots port defaults to `1234`. This is the recommended base port
because it is the Webots default. Additional runs use consecutive ports.

## Config-Based Usage

Training configs are stored in:

```text
webots/controllers/RL_Supervisor/training_configs/
```

Start a config-based experiment:

```powershell
python .\parallel_runs.py --config test_training_parameters.json
```

The `--config` value can be either a direct file path or a file name inside
`training_configs/`.

## Config File Structure

Example:

```json
{
    "experiment_name": "test_training_parameters",
    "max_training_updates": 5,
    "base_port": 1234,
    "runs": [
        {
            "name": "run_001_conservative",
            "parameters": {
                "actor_learning_rate": 0.00005,
                "critic_learning_rate": 0.0002,
                "gamma": 0.98,
                "gae_lambda": 0.9,
                "policy_clip": 0.15,
                "batch_size": 32,
                "n_epochs": 2,
                "std_dev": 0.6,
                "min_std_dev": 0.05,
                "std_dev_factor": 0.995
            }
        },
        {
            "name": "run_002_baseline",
            "parameters": {
                "actor_learning_rate": 0.0001,
                "critic_learning_rate": 0.0003,
                "gamma": 0.99,
                "gae_lambda": 0.95,
                "policy_clip": 0.2,
                "batch_size": 64,
                "n_epochs": 3,
                "std_dev": 0.5,
                "min_std_dev": 0.03,
                "std_dev_factor": 0.9975
            }
        }
    ]
}
```

Each run requires a `name`. The `parameters` object is optional. Missing
parameters keep the defaults from the `Agent` constructor.

## Supported Parameters

- `actor_learning_rate`
- `critic_learning_rate`
- `gamma`
- `gae_lambda`
- `policy_clip`
- `batch_size`
- `n_epochs`
- `std_dev`
- `min_std_dev`
- `std_dev_factor`

## Output Structure

Each experiment is written below:

```text
webots/controllers/RL_Supervisor/training_runs/<experiment_name>/
```

Each run contains:

```text
run_001/
  logs/
    training_logs.csv
    action_diagnostics.csv
    run_configuration.json
  models/
    actor.weights.h5
    critic.weights.h5
  process.log
```

The temporary `.pio_build/` directory is removed after the run finishes.

## Comparison Output

After all runs finish successfully, `parallel_runs.py` calls `compare_runs.py`.
Comparison files are written to:

```text
training_runs/<experiment_name>/comparisons/
```

This includes metric plots and `parameter_differences.txt`.

## Notes

- The experiment directory must not exist before starting the wrapper.
- Per-run process output is written to `process.log`.
- Webots may print clipboard warnings when several instances start. These are
  known and currently not treated as fatal.
- Config values are validated by the RL supervisor when they are read from
  environment variables.
