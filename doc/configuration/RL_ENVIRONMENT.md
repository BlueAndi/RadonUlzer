# Python Virtual Environment for RL Supervisor

This document explains why the RL supervisor uses a Python virtual environment
and how to create and use it with `uv`.

## Why use a virtual environment?

The RL supervisor depends on Python packages such as TensorFlow, TensorFlow
Probability, Pandas and Matplotlib. Different Python projects may require
different versions of the same packages. A virtual environment keeps these
project-specific dependencies separate, so installing or updating packages for
one project does not break another project.

## Why use uv?

`uv` is a fast Python package and environment manager. In this project it is used
to create the virtual environment and install the dependencies from
`requirements.txt`.

Compared to manually using `venv` and `pip`, `uv` usually makes creating the
environment and installing dependencies faster and more convenient.

## Prerequisites

Make sure the following tools are installed:

- `uv` ([installation instructions](https://docs.astral.sh/uv/getting-started/installation/))

Open a terminal in the RL supervisor directory:

```bash
cd webots/controllers/RL_Supervisor
```

## Python version

This environment should use Python 3.10 because some Python packages can have
version-specific compatibility requirements.

## Create the virtual environment

Create a local virtual environment with Python 3.10:

```bash
uv venv --python 3.10
```

This creates a `.venv` directory inside the current folder.

## Activate the virtual environment

On Windows PowerShell:

```powershell
.venv\Scripts\Activate.ps1
```

On Linux or macOS:

```bash
source .venv/bin/activate
```

After activation, the terminal prompt shows the active environment name,
for example `(.venv)`. This shows that the virtual environment is active.

After activation, Python commands use the packages installed in `.venv`.

## Install dependencies

Install the required Python packages:

```bash
uv pip install -r requirements.txt
```

## Use the environment

After activating the environment, scripts can be run with the local Python
dependencies.

Examples:

```bash
python plotting.py
```

## Use the environment with Webots

Start Webots from a terminal where the virtual environment is activated. This
allows Webots to use the Python packages installed in `.venv`.

If the `webots` command is not available, start Webots with the full path to the
Webots executable.

On Windows PowerShell, for example:

```powershell
& "C:\Users\<user>\AppData\Local\Programs\Webots\msys64\mingw64\bin\webotsw.exe"
```

## Deactivate the environment

Leave the virtual environment with:

```bash
deactivate
```
