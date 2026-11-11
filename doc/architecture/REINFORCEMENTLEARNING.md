# Radon Ulzer - Reinforcement Learning Supervisor <!-- omit in toc -->

- [General](#general)
- [SW Architecture](#sw-architecture)
  - [Use Case View](#use-case-view)
  - [Logical View](#logical-view)
    - [Compact Class Diagram](#compact-class-diagram)
    - [Module Class Diagrams](#module-class-diagrams)
      - [Networks](#networks)
      - [Agent](#agent)
      - [Training Orchestrator](#training-orchestrator)
      - [Evaluation Orchestrator](#evaluation-orchestrator)
      - [Trajectory Buffer](#trajectory-buffer)
      - [Environment](#environment)
      - [Webots Environment](#webots-environment)
      - [Reward](#reward)
      - [Transport](#transport)
      - [Training Logger](#training-logger)
      - [Evaluation Logger](#evaluation-logger)
  - [Process View](#process-view)
    - [State Machine](#state-machine)
- [Abbreviations](#abbreviations)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## General

The reinforcement learning supervisor trains a PPO agent to control simulated
Zumo robots in Webots and evaluates saved checkpoints with deterministic
actions. Its environment-independent orchestration separates reinforcement
learning from Webots-specific interaction and robot communication.

This document presents the supervisor's use cases, logical structure, and
runtime lifecycle.

## SW Architecture

The following part contains the specific architectural views of the
reinforcement learning supervisor.

### Use Case View

The use case view presents the environment-independent RL Supervisor as the
system of interest. A training operator can start a training session or
evaluate a selected checkpoint. The environment participates in interaction
and episode management; the selected run mode determines whether the supervisor
trains models or records evaluation results.

![supervisorUseCases](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/UseCaseView/ReinforcementLearning/SupervisorUseCaseDiagram.plantuml)

### Logical View

The logical view presents the supervisor's static structure at two levels: a
compact overview of the main components and focused views of the individual
Python modules.

#### Compact Class Diagram

The compact class diagram provides a high-level overview of the supervisor's
core components and their principal relationships. It intentionally omits
supporting types, attributes, methods, and implementation-specific details and
therefore does not represent the complete class model.

![supervisorCompactClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/SupervisorCompactClassDiagram.plantuml)

#### Module Class Diagrams

The following focused class diagrams show the classifiers contained in each
Python module and its direct structural relationships to neighboring modules.
Attributes and operations are intentionally omitted to keep each view concise.

##### Networks

`Models` encapsulates the Actor and Critic networks and performs their
calculations and optimization. `TrainableStdDev` maintains the trainable
standard deviation used for exploration. `ActionResult`,
`PolicyOptimizationMetrics`, and `CriticOptimizationMetrics` provide the
results of action selection, policy optimization, and Critic optimization.

![networksClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/NetworksClassDiagram.plantuml)

##### Agent

`PPOAgent` provides the central interface for action selection, experience
storage, and complete PPO training updates. It coordinates `Memory` and
`Models`, including bootstrap estimation, KL-reactive policy optimization,
learning-rate adaptation, and checkpoint handling. `AgentUpdateResult`
summarizes the diagnostics produced by one completed training update.

![agentClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/AgentClassDiagram.plantuml)

##### Training Orchestrator

`TrainingOrchestrator` runs the environment-independent training loop. It
processes results for each interaction stream, completes transitions, dispatches
and logs actions, triggers and logs PPO updates, resets streams, and coordinates
checkpoint persistence. `PendingAction` links a submitted action and its PPO
information to the next environment result from the same stream.

![trainingOrchestratorClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/TrainingOrchestratorClassDiagram.plantuml)

##### Evaluation Orchestrator

`EvaluationOrchestrator` runs deterministic inference across asynchronous
environment streams. It collects the reward returned for each accepted action,
records completed-episode statistics, and optionally stops after the configured
completed-episode threshold has been reached. All results from the current
environment cycle are processed before the limit is checked. It does not store
PPO transitions or update the models.

![evaluationOrchestratorClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/EvaluationOrchestratorClassDiagram.plantuml)

##### Trajectory Buffer

`Memory` stores and validates ordered transitions separately for each
interaction stream. It derives completed-episode statistics and converts the
collected rollout into a shuffled TensorFlow dataset by calculating bootstrapped
returns and normalized generalized advantages. `Transition`,
`EpisodeStatistics`, and `TrainingBatch` represent one environment transition,
aggregate results of completed episodes, and the named tensors used for PPO
training.

![trajectoryBufferClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/TrajectoryBufferClassDiagram.plantuml)

##### Environment

`IEnvironment` defines the simulation-independent, asynchronous contract for
resetting interaction streams, submitting actions, advancing environment
cycles, and stopping execution. `EnvironmentStep` carries each fresh
observation together with its associated reward, terminal status, episode
identifier, step identifier, and optional opaque terminal-reason identifier.
The contract does not define the reason vocabulary, so training and evaluation
remain independent of task-specific outcomes. `WebotsEnv` realizes the contract
for the simulation.

![environmentClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/EnvironmentClassDiagram.plantuml)

##### Webots Environment

`WebotsEnv` realizes the simulation-independent environment contract by
advancing Webots and collecting fresh results from independently managed robot
streams. Each `ZumoRobot` coordinates its transport callbacks, normalizes
line-sensor data, translates normalized actions into motor speeds, delegates
reward and termination evaluation, and manages resets and episode state.
Each published Webots observation contains the previous and current normalized
five-sensor readings; the first valid reading is duplicated and each robot
clears its history after a successful reset.
`StartPose` defines a reset pose with optional lateral and rotational variation,
while `StartPoseSelector` chooses reset poses according to the configured
selection strategy.

![webotsEnvironmentClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/WebotsEnvironmentClassDiagram.plantuml)

##### Reward

`RewardCalculator` configures a reward function selected by identifier and uses
it to calculate rewards, while independently evaluating fixed lost-line and
finish-line termination conditions. Its `EpisodeTerminationReason` enum names
these line-follower-specific outcomes; the Webots adapter publishes their
string values through the generic environment contract. `RewardContext` groups
the raw sensor readings and episode information required for both evaluations.
Keeping this logic independent of Webots and robot communication allows
additional reward functions to be added within the module without changing
environment interaction.

![rewardClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/RewardClassDiagram.plantuml)

##### Transport

`ZumoSerial` provides the robot transport boundary around a SerialMuxProt server
dedicated to one robot stream. It creates the required transmit channels,
processes transport activity, encodes motor-speed and firmware-command payloads,
and decodes incoming line-sensor and status payloads before forwarding their
typed values through registered callbacks. `CommandId` and `RobotStatus`
represent the protocol-defined command and status values.

![transportClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/TransportClassDiagram.plantuml)

##### Training Logger

`TrainingLogger` creates the output files for one training run and persists
per-step action diagnostics, per-update aggregate training metrics, and the
run's configuration snapshot. It also records checkpoint metadata and preserves
the complete lineage when training resumes from an earlier checkpoint.
`ActionDiagnostics`, `TrainingMetrics`, and `CheckpointReference` structure
action records, update records, and checkpoint identities.

![trainingLoggerClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/TrainingLoggerClassDiagram.plantuml)

##### Evaluation Logger

`EvaluationLogger` records each completed `EpisodeEvaluation` in `episodes.csv`
and writes overall and per-stream statistics to `evaluation_summary.json` when
the run closes. Its run configuration identifies the evaluated checkpoint;
evaluation files are stored separately from training diagnostics and
checkpoints. Termination counts use the opaque reason identifiers supplied by
the environment; the logger does not prescribe task-specific categories.

![evaluationLoggerClasses](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/LogicalView/ReinforcementLearning/module_views/EvaluationLoggerClassDiagram.plantuml)

### Process View

The process view describes the normal runtime lifecycle of one reinforcement
learning run. It focuses on the high-level state changes; detailed
component interactions and exceptional termination paths are intentionally
omitted.

#### State Machine

After initialization, the supervisor interacts with the environment in the
`Running` state. In training mode, it collects experience and enters `Training`
when sufficient trajectory data is available. In evaluation mode, it selects
deterministic actions and records completed episodes without entering
`Training`. Training returns to `Running` for another rollout or enters
`Stopping` when the configured update limit is reached. Evaluation enters
`Stopping` when its optional completed-episode limit is reached. Environment
termination leads from `Running` to `Stopping` in either mode.

![supervisorStateMachine](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/RadonUlzer/refs/heads/base/RL/doc/architecture/uml/ProcessView/ReinforcementLearning/SupervisorStateDiagram.plantuml)

## Abbreviations

| Abbreviation | Description |
| - | - |
| KL | Kullback-Leibler Divergence |
| PPO | Proximal Policy Optimization |
| RL | Reinforcement Learning |
| UML | Unified Modeling Language |

## Issues, Ideas And Bugs

If you have further ideas or you found some bugs, great! Create an
[issue](https://github.com/BlueAndi/RadonUlzer/issues) or, if you are able and
willing to fix it by yourself, clone the repository and create a pull request.

## License

The whole source code is published under the
[MIT license](http://choosealicense.com/licenses/mit/). Consider the different
licenses of the used third-party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally
submitted for inclusion in the work by you shall be licensed as above, without
any additional terms or conditions.
