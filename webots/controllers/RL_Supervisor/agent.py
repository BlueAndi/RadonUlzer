"""Implementation of intelligent agent."""

# MIT License
#
# Copyright (c) 2023 - 2024 Andreas Merkle <web@blue-andi.de>
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

import csv
import json
import os
import struct
from datetime import datetime
import numpy as np
import tensorflow as tf
import tensorflow_probability as tfp
from trajectory_buffer import Memory
from networks import Models

################################################################################
# Variables
################################################################################
# pylint: disable=duplicate-code

# Constants

# APPRemoteControl SerialMuxChannels: "MOTOR_SET" carries MotorSpeed (2x int32, mm/s)
MOTOR_SPEED_CHANNEL_NAME = "MOTOR_SET"
MOTOR_SPEED_DLC = 8  # 2x int32

COMMAND_CHANNEL_NAME = "CMD"
# sizeof(Command) in APPRemoteControl: 1 byte CmdId + union { 3x int32 } = 13 bytes
CMD_DLC = 13

LINE_SENSOR_CHANNEL_NAME = "LINE_SENS"
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200

STATUS_CHANNEL_NAME = "STATUS"
STATUS_CHANNEL_ERROR_VAL = 1

# APPRemoteControl command IDs (SMPChannelPayload::CmdId)
CMD_ID_IDLE = 0
# stops motors and re-inits board; required after supervisor position reset
CMD_ID_REINIT_BOARD = 3

# Robot start poses used cyclically after each episode reset.
# Each entry consists of a Webots translation vector and rotation field value.
FORWARD_POSITION_DATA = [-0.24713614078815466, -0.04863962992854465, 0.013994298332013683]
FORWARD_ORIENTATION_DATA = [-1.0564747468923541e-06, 8.746699709178704e-07, 1.0, 1.5880805820884731]

REVERSE_POSITION_DATA = [-0.247145, 0.16, 0.0139943]
REVERSE_ORIENTATION_DATA = [-1.06e-06, 8.75e-07, 1.0, -1.55]

FORWARD_CURVE_POSITION_DATA = [-0.247145, 0.4, 0.0139943]
FORWARD_CURVE_ORIENTATION_DATA = [-1.06e-06, 8.75e-07, 1.0, 0.584]

REVERSE_CURVE_POSITION_DATA = [-0.247145, -0.36, 0.0139943]
REVERSE_CURVE_ORIENTATION_DATA = [-1.06e-06, 8.75e-07, 1.0, -0.584]
START_POSES = (
    (FORWARD_POSITION_DATA, FORWARD_ORIENTATION_DATA),
    (REVERSE_POSITION_DATA, REVERSE_ORIENTATION_DATA),
    (FORWARD_CURVE_POSITION_DATA, FORWARD_CURVE_ORIENTATION_DATA),
    (REVERSE_CURVE_POSITION_DATA, REVERSE_CURVE_ORIENTATION_DATA),
)

MAX_SENSOR_VALUE = 1000
MIN_STD_DEV = 0.03  # Minimum standard deviation
STD_DEV_FACTOR = 0.9975  # Reaches about 0.1 after 650 episodes

TRANSLATION_FIELD = "translation"
ROTATION_FIELD = "rotation"

IDLE = "IDLE_STATE"
READY = "READY_STATE"
TRAINING = "TRAINING_STATE"

DIRECTORY = "logs"
FILE_DIRECTORY = "training_logs.csv"
ACTION_DIAGNOSTICS_FILE = "action_diagnostics.csv"
RUN_CONFIG_FILE = "run_configuration.json"
ACTION_DIAGNOSTICS_STEPS = 20

################################################################################
# Classes
################################################################################


class Agent:  # pylint: disable=too-many-instance-attributes
    """
    The Agent class represents an intelligent agent that makes decisions to
    control motors based on the position of the robot.
    """

    # pylint: disable=too-many-arguments,too-many-locals
    def __init__(
        self,
        smp_server,
        gamma=0.99,
        actor_alpha=0.0001,
        critic_alpha=0.0003,
        gae_lambda=0.95,
        policy_clip=0.2,
        batch_size=64,
        n_epochs=3,
        std_dev=0.5,
        min_std_dev=MIN_STD_DEV,
        std_dev_factor=STD_DEV_FACTOR,
        chkpt_dir="models/",
        top_speed=250,  # mm/s (~2000 encoder steps/s on Zumo32U4)
        max_buffer_length=65536,
        max_training_updates=None,
        run_directory=None,
    ) -> None:
        """
        Initialize the reinforcement learning agent.

        Parameters
        ----------
            smp_server: Serial communication server used to control the robot.
            gamma: Discount factor for future rewards.
            actor_alpha: Learning rate of the actor network.
            critic_alpha: Learning rate of the critic network.
            gae_lambda: Decay factor for generalized advantage estimation.
            policy_clip: PPO policy clipping range.
            batch_size: Number of samples in each mini batch.
            n_epochs: Number of training epochs per trajectory batch.
            std_dev: Initial action sampling standard deviation.
            min_std_dev: Lower bound for the sampling standard deviation.
            std_dev_factor: Multiplicative decay factor for the standard deviation.
            chkpt_dir: Directory used for model checkpoints.
            top_speed: Maximum motor speed in millimeters per second.
            max_buffer_length: Maximum number of stored trajectory samples.
            max_training_updates: Optional limit for completed training updates.
            run_directory: Optional directory managed by a training wrapper.

        Returns
        ----------
            None
        """

        self.__serialmux = smp_server
        self.__gamma = gamma
        self.__actor_alpha = actor_alpha
        self.__critic_alpha = critic_alpha
        self.__gae_lambda = gae_lambda
        self.__policy_clip = policy_clip
        self.__batch_size = batch_size
        self.__min_buffer_length = max(batch_size * 8, 512)
        self.__max_buffer_length = max_buffer_length
        self.__max_training_updates = max_training_updates
        self.__run_directory = run_directory
        self.__chkpt_dir = chkpt_dir
        self.train_mode = False
        self.__top_speed = top_speed
        self.__std_dev = std_dev
        self.__min_std_dev = min_std_dev
        self.__std_dev_factor = std_dev_factor
        self.__memory = Memory(
            batch_size, max_buffer_length, gamma, gae_lambda,
            self.__min_buffer_length)
        self.__neural_network = Models(
            actor_alpha, critic_alpha, policy_clip)
        self.__training_index = 0  # Track batch index during training
        self.__current_batch = None  # Saving of the current batch which is in process
        self.n_epochs = n_epochs
        self.done = False
        self.action = None
        self.actor_mean = None
        self.value = None
        self.adjusted_log_prob = None
        self.num_training_updates = 0
        self.num_episodes = 1
        self.state = IDLE
        self.data_sent = True
        self.unsent_data = []
        self.reward_history = []
        self.reinitialized = False
        self.training_finished = False
        self.training_history = []
        self.__episode_steps = []
        self.__episode_rewards = []
        self.__actor_loss_start_index = 0
        self.__critic_loss_start_index = 0
        self.__diagnostic_step = 0
        # The initial world pose is START_POSES[0], so the first reset uses
        # the next pose in the cycle.
        self.__start_pose_index = 1
        self.__timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        self.__initialize_training_directory()
        self.__initialize_training_log()
        self.__initialize_action_diagnostics()
        self.__initialize_run_config()

    def set_train_mode(self) -> None:
        """
        Set the agent to training mode.

        Returns
        ----------
            None
        """

        self.train_mode = True
        self.state = READY

    def set_drive_mode(self) -> None:
        """
        Set the agent to driving mode.

        Returns
        ----------
            None
        """

        self.train_mode = False
        self.state = READY
        self.num_training_updates = 0
        self.num_episodes = 1

    def store_transition(
        self, state, action, probs, value, reward, done
    ) -> None:  # pylint: disable=too-many-arguments
        """Store transitions in the replay buffer.

        Parameters
        ----------
            state: The state observed.
            action: The action taken.
            probs: The probability of taking the action.
            value: The estimated value of the state.
            reward: The reward received.
            done: Indicating whether the target sequence has been reached.

        Returns
        ----------
            None
        """

        self.__memory.store_memory(state, action, probs, value, reward, done)

    def save_models(self) -> None:
        """
        Save the current model weights to the checkpoint directory.

        Returns
        ----------
            None
        """

        os.makedirs(self.__chkpt_dir, exist_ok=True)
        self.__neural_network.actor_network.save_weights(
            self.__chkpt_dir + "actor.weights.h5"
        )
        self.__neural_network.critic_network.save_weights(
            self.__chkpt_dir + "critic.weights.h5"
        )

    def load_models(self) -> None:
        """
        Load model weights from the checkpoint directory.

        Returns
        ----------
            None
        """

        self.__neural_network.actor_network.load_weights(
            self.__chkpt_dir + "actor.weights.h5"
        )
        self.__neural_network.critic_network.load_weights(
            self.__chkpt_dir + "critic.weights.h5"
        )

    def load_models_if_available(self) -> None:
        """
        Load model weights when a complete checkpoint exists.

        Returns
        ----------
            None
        """

        actor_path = self.__chkpt_dir + "actor.weights.h5"
        critic_path = self.__chkpt_dir + "critic.weights.h5"

        if os.path.exists(actor_path) and os.path.exists(critic_path):
            self.load_models()
        else:
            print("No complete model checkpoint available. Starting fresh training.")


    @tf.function(
            autograph=False,
            input_signature=[
                tf.TensorSpec(shape=(1, 5), dtype=tf.float32),
                tf.TensorSpec(shape=(), dtype=tf.float32)
            ]
    )
    def _predict_train_graph(
        self, state_tensor, std_dev
    ) -> tuple[tf.Tensor, tf.Tensor, tf.Tensor, tf.Tensor]:
        """
        Predict training outputs with the TensorFlow graph.

        Parameters
        ----------
            state_tensor: Normalized state tensor for one observation.
            std_dev: Standard deviation used for exploration sampling.

        Returns
        ----------
            tuple: Actor mean, transformed sampled action, critic value,
                   and adjusted log probability.
        """

        # Forward pass through the actor network to get the action mean
        action_mean = self.__neural_network.actor_network(state_tensor)

        # Create a normal distribution
        dist = tfp.distributions.Normal(action_mean, std_dev)

        # Sampling an action from the normal distribution
        sampled_action = dist.sample()

        # Apply the Tanh transformation to the sampled action
        transformed_action = tf.tanh(sampled_action)

        # Calculation of the logarithm of the probability density of the sampled action
        log_prob = dist.log_prob(sampled_action)

        # Calculation of the Jacobian determinant for the Tanh transformation
        jacobian_log_det = tf.math.log(
            1.0 - tf.square(transformed_action) + 1e-6)

        # Calculation of Adjusted probabilities by the neural network
        adjusted_log_prob = log_prob - jacobian_log_det

        # calculate the estimated value of a state, which is determined by the Critic network
        value = self.__neural_network.critic_network(state_tensor)

        return action_mean, transformed_action, value, adjusted_log_prob


    def predict_action(self, state) -> np.ndarray:
        """
        Predict an action based on the current state.

        Parameters
        ----------
            state: The state observed.

        Returns
        ----------
            np.ndarray: Predicted action.
        """

        # scales the sensor data to a range between 0 and 1
        m_state = self.normalize_sensor_data(state)

        # Conversion of the state into a tensor
        state_tensor = tf.convert_to_tensor([m_state], dtype=tf.float32)

        # Training mode is set.
        if self.train_mode is True:

            std_dev = tf.convert_to_tensor(self.__std_dev, dtype=tf.float32)
            (
                action_mean,
                transformed_action,
                value,
                adjusted_log_prob,
            ) = self._predict_train_graph(state_tensor, std_dev)

            self.actor_mean = action_mean.numpy()[0]
            self.action = transformed_action.numpy()[0]
            self.value = value.numpy()[0]
            self.adjusted_log_prob = adjusted_log_prob.numpy()[0]

        # Driving mode is set
        else:

            # Forward pass through the actor network to get the action mean
            action_mean = self.__neural_network.actor_network(state_tensor)

            self.actor_mean = action_mean.numpy()[0]
            self.action = action_mean.numpy()[0]

        return self.action

    def send_motor_speeds(self, state) -> None:
        """
        Send motor speeds to the robot.

        Parameters
        ----------
            state: The state observed.

        Returns
        ----------
            None
        """

        # pre_action contains the predicted action for the given state, calculated based
        # on the Actor model output.
        pre_action = self.predict_action(state)
        self.__log_action_diagnostics(state)

        # Get motor speed difference
        speed_difference = self.__top_speed * pre_action

        # Get individual motor speeds. The sign of speedDifference
        # determines if the robot turns left or right.
        left_motor_speed = int(self.__top_speed - speed_difference)
        right_motor_speed = int(self.__top_speed + speed_difference)

        # MotorSpeed payload: 2x int32 in mm/s (little-endian, packed)
        control_data = struct.pack("<2i", left_motor_speed, right_motor_speed)
        self.data_sent = self.__serialmux.send_data(
            MOTOR_SPEED_CHANNEL_NAME, control_data)

        # Failed to send data. Appends the data to unsent_data List.
        if self.data_sent is False:
            self.unsent_data.append((MOTOR_SPEED_CHANNEL_NAME, control_data))

    def __initialize_training_directory(self) -> None:
        """
        Create directories for logs and model checkpoints.

        Returns
        ----------
            None
        """

        if self.__run_directory is None:
            self.__training_directory = os.path.join(DIRECTORY, self.__timestamp)
        else:
            self.__training_directory = os.path.join(self.__run_directory, DIRECTORY)
            self.__chkpt_dir = os.path.join(self.__run_directory, "models", "")

        os.makedirs(self.__training_directory, exist_ok=True)
        os.makedirs(self.__chkpt_dir, exist_ok=True)


    def __initialize_action_diagnostics(self) -> None:
        """
        Create the action diagnostics log for this training run.

        Returns
        ----------
            None
        """

        log_file = os.path.join(self.__training_directory, ACTION_DIAGNOSTICS_FILE)

        with open(log_file, mode="w", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(
                [
                    "Training Update",
                    "Episode",
                    "Step",
                    "Sensor0",
                    "Sensor1",
                    "Sensor2",
                    "Sensor3",
                    "Sensor4",
                    "Actor Mean",
                    "Sampled Action",
                    "Std Dev",
                ]
            )

    def __initialize_training_log(self) -> None:
        """
        Create the training log for this training run.

        Returns
        ----------
            None
        """

        log_file = os.path.join(self.__training_directory, FILE_DIRECTORY)

        with open(log_file, mode="w", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(
                ["Training Update", "Mean Actor Loss", "Mean Critic Loss",
                 "Mean Episode Reward", "Mean Episode Steps"]
            )

    def complete_episode(self, steps) -> None:
        """
        Record a completed episode and reset its diagnostics counter.

        Parameters
        ----------
            steps: Number of simulation steps in the completed episode.

        Returns
        ----------
            None
        """

        self.__episode_steps.append(steps)
        self.__diagnostic_step = 0
        self.num_episodes += 1

    def __initialize_run_config(self) -> None:
        """
        Create the configuration file for this training run.

        Returns
        ----------
            None
        """

        log_file = os.path.join(self.__training_directory, RUN_CONFIG_FILE)

        # Capture parameters that affect run comparability.
        run_config = {
            "training": {"gamma" : self.__gamma,
                         "actor_alpha" : self.__actor_alpha,
                         "critic_alpha" : self.__critic_alpha,
                         "gae_lambda" : self.__gae_lambda,
                         "policy_clip" : self.__policy_clip,
                         "batch_size" : self.__batch_size,
                         "n_epochs" : self.n_epochs},
            "exploration": {"std_dev" : self.__std_dev,
                            "min_std_dev" : self.__min_std_dev,
                            "std_dev_factor" : self.__std_dev_factor},
            "diagnostics": {"action_diagnostics_steps" : ACTION_DIAGNOSTICS_STEPS},
            "buffer": {"min_buffer_length" : self.__min_buffer_length,
                       "max_buffer_length" : self.__max_buffer_length},
            "environment": {"top_speed" : self.__top_speed,
                           "max_sensor_value" : MAX_SENSOR_VALUE,
                           "forward_position_data" : FORWARD_POSITION_DATA,
                           "forward_orientation_data" : FORWARD_ORIENTATION_DATA,
                           "reverse_position_data" : REVERSE_POSITION_DATA,
                           "reverse_orientation_data" : REVERSE_ORIENTATION_DATA,
                           "forward_curve_position_data" : FORWARD_CURVE_POSITION_DATA,
                           "forward_curve_orientation_data" : FORWARD_CURVE_ORIENTATION_DATA,
                           "reverse_curve_position_data" : REVERSE_CURVE_POSITION_DATA,
                           "reverse_curve_orientation_data" : REVERSE_CURVE_ORIENTATION_DATA },
            "reward": {"function": "triangle",
                       "max" : 1,
                       "min" : 0,
                       "min_position" : 500,
                       "center_position" : 2000,
                       "max_position" : 3500,
                       "scale" : 1500}
        }

        with open(log_file, mode="w", encoding="utf-8") as file:
            json.dump(run_config, file, indent=4)

    def __log_action_diagnostics(self, sensor_data) -> None:
        """
        Log the first actions of each episode for policy analysis.

        Parameters
        ----------
            sensor_data: Raw line sensor values for the current action.

        Returns
        ----------
            None
        """

        self.__diagnostic_step += 1

        if self.__diagnostic_step > ACTION_DIAGNOSTICS_STEPS:
            return

        log_file = os.path.join(self.__training_directory, ACTION_DIAGNOSTICS_FILE)
        with open(log_file, mode="a", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(
                [
                    self.num_training_updates + 1,
                    self.num_episodes,
                    self.__diagnostic_step,
                    *sensor_data,
                    float(self.actor_mean[0]),
                    float(self.action[0]),
                    self.__std_dev,
                ]
            )

    def update(self, robot_node) -> None:
        """
        Check whether an episode ended and update the agent state.

        Parameters
        ----------
            robot_node: The Robot interface

        Returns
        ----------
            None
        """

        # Checks whether the sequence has ended if it is set to Training mode.
        if self.train_mode is True and (
                self.done is True or self.__memory.is_memory_full() is True):

            # REINIT_BOARD stops motors and re-initializes the board drivers.
            # This is necessary because reinitialize() teleports the robot in
            # Webots without restarting the controller executable.
            cmd_payload = struct.pack("<Biii", CMD_ID_REINIT_BOARD, 0, 0, 0)
            self.data_sent = self.__serialmux.send_data(
                COMMAND_CHANNEL_NAME, cmd_payload)

            # Failed to send data. Appends the data to unsent_data List.
            if self.data_sent is False:
                self.unsent_data.append((COMMAND_CHANNEL_NAME, cmd_payload))

            # Teleport the robot back to start for the next episode.
            self.reinitialize(robot_node)
            self.done = False
            self.reinitialized = True

            # Only start training once enough experience is accumulated across
            # episodes. Until then, keep collecting in READY state.
            if self.__memory.is_ready_for_training():
                self.state = TRAINING

        # Checks whether the sequence has ended if it is set to driving mode.
        if (self.train_mode is False) and (self.done is True):
            self.done = False

            # Stop motors immediately
            motorcontrol = struct.pack("<2i", 0, 0)
            self.data_sent = self.__serialmux.send_data(
                MOTOR_SPEED_CHANNEL_NAME, motorcontrol
            )

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append(
                    (MOTOR_SPEED_CHANNEL_NAME, motorcontrol))

            # Re-init board for next inference run
            cmd_payload = struct.pack("<Biii", CMD_ID_REINIT_BOARD, 0, 0, 0)
            self.reinitialize(robot_node)
            self.data_sent = self.__serialmux.send_data(
                COMMAND_CHANNEL_NAME, cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append((COMMAND_CHANNEL_NAME, cmd_payload))

            # Return directly to READY — APPRemoteControl stays in DrivingState
            self.set_drive_mode()

    def normalize_sensor_data(self, sensor_data) -> np.ndarray:
        """
        Scale sensor data to a range between 0 and 1.

        Parameters
        ----------
            sensor_data: The state observed.

        Returns
        ----------
            np.ndarray: Normalized sensor data.
        """

        normalized_sensor_data = np.array(sensor_data) / MAX_SENSOR_VALUE
        return normalized_sensor_data

    def determine_reward(self, sensor_data) -> float:
        """
        Calculate the reward for the current sensor data.

        Parameters
        ----------
            sensor_data: The state observed.

        Returns
        ----------
            float: Calculated reward.
        """

        reward = self.__memory.calculate_reward(sensor_data)
        return reward

    def learn(
        self,
        states,
        actions,
        old_probs,
        values,
        advantages,
        normalized_advantages,
    ) -> None:
        """
        Perform training to optimize model weights.

        Parameters
        ----------
            states:     The saved states observed during interactions with the environment.
            actions:    The saved actions taken in response to the observed states.
            old_probs:  The saved probabilities of the actions taken, based on the previous policy.
            values:     The saved estimated values of the observed states.
            advantages: The original advantage values used to train the critic.
            normalized_advantages: The normalized advantage values used to train the actor.

        Returns
        ----------
            None
        """

        # scales the sensor data to a range between 0 and 1
        m_states = self.normalize_sensor_data(states)

        for _ in range(self.n_epochs):

            states = tf.convert_to_tensor(m_states, dtype=tf.float32)
            actions = tf.convert_to_tensor(actions, dtype=tf.float32)
            old_probs = tf.convert_to_tensor(old_probs, dtype=tf.float32)

            # optimize Actor Network weights
            self.__neural_network.compute_actor_gradient(
                states,
                actions,
                old_probs,
                normalized_advantages,
                self.__std_dev
            )

            # optimize Critic Network weights
            self.__neural_network.compute_critic_gradient(
                states, values, advantages)

    def save_logs_to_csv(self) -> None:
        """
        Append the latest training metrics to the training log.

        Returns
        ----------
            None
        """

        log_file = os.path.join(self.__training_directory, FILE_DIRECTORY)

        if not self.training_history:
            return

        with open(log_file, mode="a", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(self.training_history[-1])

    def perform_training(self) -> None:
        """
        Run training for the current trajectory batch.

        Returns
        ----------
            None
        """

        if self.__current_batch is None:

            # Grab sample from memory
            self.__current_batch = self.__memory.generate_batches()

            self.__episode_rewards = self.__memory.get_episode_rewards()

            self.__actor_loss_start_index = len(
                self.__neural_network.actor_loss_history
            )
            self.__critic_loss_start_index = len(
                self.__neural_network.critic_loss_history
            )

        # Perform training with mini batches.
        if self.__training_index < len(self.__current_batch[-1]):
            (
                state_arr,
                action_arr,
                old_prob_arr,
                vals_arr,
                advatage_arr,
                normalized_advatage_arr,
                batches,
            ) = self.__current_batch
            batch = batches[self.__training_index]

            # pylint: disable=too-many-arguments
            self.learn(
                state_arr[batch],
                action_arr[batch],
                old_prob_arr[batch],
                vals_arr[batch],
                advatage_arr[batch],
                normalized_advatage_arr[batch]
            )
            self.__training_index += 1

        # Training completed.
        else:
            self.__training_index = 0
            self.__current_batch = None
            self.done = False

            actor_losses = self.__neural_network.actor_loss_history[
                self.__actor_loss_start_index:
            ]

            critic_losses = self.__neural_network.critic_loss_history[
                self.__critic_loss_start_index:
            ]

            mean_actor_loss = float(np.mean(actor_losses))
            mean_critic_loss = float(np.mean(critic_losses))
            mean_episode_steps = float(np.mean(self.__episode_steps))
            mean_episode_rewards = float(np.mean(self.__episode_rewards))

            self.training_history.append(
                (
                    self.num_training_updates + 1,
                    mean_actor_loss,
                    mean_critic_loss,
                    mean_episode_rewards,
                    mean_episode_steps,
                )
            )

            self.save_logs_to_csv()

            self.__memory.clear_memory()
            self.__episode_steps = []
            self.num_training_updates += 1
            if (
                self.__max_training_updates is not None
                and self.num_training_updates >= self.__max_training_updates
            ):
                self.training_finished = True

            # Minimize standard deviation until the minimum standard deviation is reached
            self.__std_dev = self.__std_dev * self.__std_dev_factor
            self.__std_dev = max(self.__std_dev, self.__min_std_dev)

            # APPRemoteControl stays in DrivingState — no command needed to restart.
            # Transition directly back to READY so motor speeds resume on the next
            # LINE_SENS callback.
            self.set_train_mode()

    def reinitialize(self, robot_node) -> None:
        """
        Reset the robot position and orientation to the next start pose.

        Parameters
        ----------
            robot_node: The Robot interface

        Returns
        ----------
            None
        """

        trans_field = robot_node.getField(TRANSLATION_FIELD)
        rot_field = robot_node.getField(ROTATION_FIELD)
        initial_position, initial_orientation = START_POSES[
            self.__start_pose_index
        ]

        trans_field.setSFVec3f(initial_position)
        rot_field.setSFRotation(initial_orientation)
        self.__start_pose_index = (
            self.__start_pose_index + 1
        ) % len(START_POSES)


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
