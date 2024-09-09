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
import os
import struct
import numpy as np
import tensorflow as tf
import tensorflow_probability as tfp
from   trajectory_buffer import Memory
from   networks import Models

################################################################################
# Variables
################################################################################
# pylint: disable=duplicate-code

# Constants
COMMAND_CHANNEL_NAME = "CMD"
CMD_DLC = 1

SPEED_SET_CHANNEL_NAME = "SPEED_SET"
SPEED_SET_DLC = 4

LINE_SENSOR_CHANNEL_NAME = "LINE_SENS"
LINE_SENSOR_ON_TRACK_MIN_VALUE = 200

STATUS_CHANNEL_NAME = "STATUS"
STATUS_CHANNEL_ERROR_VAL = 1

MODE_CHANNEL_NAME = "MODE"
CMD_ID_SET_READY_STATE = 1
CMD_ID_SET_TRAINING_STATE = 2
POSITION_DATA = [-0.24713614078815466, -0.04863962992854465, 0.013994298332013683]
ORIENTATION_DATA = [
    -1.0564747468923541e-06,
    8.746699709178704e-07,
    0.9999999999990595,
    1.5880805820884731
]
MAX_SENSOR_VALUE = 1000
MIN_STD_DEV = 0.01  # Minimum standard deviation
STD_DEV_FACTOR = 0.995  # Discounter standard deviation factor

TRANSLATION_FIELD = "translation"
ROTATION_FIELD = "rotation"

IDLE = "IDLE_STATE"
READY = "READY_STATE"
TRAINING = "TRAINING_STATE"

DIRECTORY = "logs"
FILE_DIRECTORY = "training_logs.csv"

################################################################################
# Classes
################################################################################


class Agent:  # pylint: disable=too-many-instance-attributes
    """
    The Agent class represents an intelligent agent that makes decisions to
    control motors based on the position of the robot.
    """

    # pylint: disable=too-many-arguments
    def __init__(
        self,
        smp_server,
        gamma=0.99,
        actor_alpha=0.0001,
        critic_alpha=0.0003,
        gae_lambda=0.95,
        policy_clip=0.2,
        batch_size=64,
        chkpt_dir="models/",
        top_speed=2000,
        max_buffer_length=65536,
    ):
        self.__serialmux = smp_server
        self.__chkpt_dir = chkpt_dir
        self.train_mode = False
        self.__top_speed = top_speed
        self.__std_dev = 0.05
        self.__memory = Memory(batch_size, max_buffer_length, gamma, gae_lambda)
        self.__neural_network = Models(actor_alpha, critic_alpha, self.__std_dev, policy_clip)
        self.__training_index = 0  # Track batch index during training
        self.__current_batch = None  # Saving of the current batch which is in process
        self.n_epochs = 3
        self.done = False
        self.action = None
        self.value = None
        self.adjusted_log_prob = None
        self.num_episodes = 0
        self.state = IDLE
        self.data_sent = True
        self.unsent_data = []
        self.reward_history = []

    def set_train_mode(self):
        """Set the Agent mode to train mode."""
        self.train_mode = True
        self.state = READY

    def set_drive_mode(self):
        """Set the Agent mode to drive mode."""
        self.train_mode = False
        self.state = READY
        self.num_episodes = 0

    def store_transition(
        self, state, action, probs, value, reward, done
    ):  # pylint: disable=too-many-arguments
        """Store transitions in the replay buffer.

        Parameters
        ----------
            state: The state observed.
            action: The action taken.
            prob: The probability of taking the action.
            value: The estimated value of the state.
            reward: The reward received.
            done: Indicating whether the target sequence has been reached.
        """
        self.__memory.store_memory(state, action, probs, value, reward, done)

    def save_models(self):
        """Saves the models in the specified file."""

        self.__neural_network.actor_network.save(self.__chkpt_dir + "actor.keras")
        self.__neural_network.critic_network.save(self.__chkpt_dir + "critic.keras")

    def load_models(self):
        """Loads the models in the specified file."""

        self.__neural_network.actor_network = tf.keras.models.load_model(
            self.__chkpt_dir + "actor.keras", compile=False
        )
        self.__neural_network.critic_network = tf.keras.models.load_model(
            self.__chkpt_dir + "critic.keras", compile=False
        )

    def predict_action(self, state):
        """
        Predicts an action based on the current state.

        Parameters
        ----------
            state: The state observed.
        
        Returns
        ----------
            float32: The action taken.
        """
        # scales the sensor data to a range between 0 and 1
        m_state = self.normalize_sensor_data(state)

        # Conversion of the state into a tensor
        state = tf.convert_to_tensor([m_state], dtype=tf.float32)

        # Forward pass through the actor network to get the action mean
        action_mean = self.__neural_network.actor_network(state)

        # Training mode is set.
        if self.train_mode is True:

            # Create a normal distribution
            dist = tfp.distributions.Normal(action_mean, self.__std_dev)

            # Sampling an action from the normal distribution
            sampled_action = dist.sample()

            # Apply the Tanh transformation to the sampled action
            transformed_action = tf.tanh(sampled_action)

            # Calculation of the logarithm of the probability density of the sampled action
            log_prob = dist.log_prob(sampled_action)

            # Calculation of the Jacobian determinant for the Tanh transformation
            jacobian_log_det = tf.math.log(1 - tf.square(transformed_action) + 1e-6)

            # Calculation of Adjusted probabilities by the neural network
            adjusted_log_prob = log_prob - jacobian_log_det

            # calculate the estimated value of a state, which is determined by the Critic network
            value = self.__neural_network.critic_network(state)

            self.action = transformed_action.numpy()[0]
            self.value = value.numpy()[0]
            self.adjusted_log_prob = adjusted_log_prob.numpy()[0]

        # Driving mode is set
        else:
            self.action = action_mean.numpy()[0]

        return self.action

    def send_motor_speeds(self, state):
        """
        Sends the motor speeds to the robot.

        Parameters
        ----------
            state: The state observed.
        """
        # pre_action contains the predicted action for the given state, calculated based
        # on the Actor model output.
        pre_action = self.predict_action(state)

        # Get motor speed difference
        speed_difference = self.__top_speed * pre_action

        # Get individual motor speeds. The sign of speedDifference
        # determines if the robot turns left or right.
        left_motor_speed = int(self.__top_speed - speed_difference)
        right_motor_speed = int(self.__top_speed + speed_difference)

        control_data = struct.pack("2H", left_motor_speed, right_motor_speed)
        self.data_sent = self.__serialmux.send_data(SPEED_SET_CHANNEL_NAME, control_data)

        # Failed to send data. Appends the data to unsent_data List.
        if self.data_sent is False:
            self.unsent_data.append((SPEED_SET_CHANNEL_NAME, control_data))

    def update(self, robot_node):
        """
        Checks if the sequence has ended and performs updates.

        Parameters
        ----------
            robot_node: The Robot interface
        """

        # Checks whether the sequence has ended if it is set to Training mode.
        if self.train_mode is True  and (
            self.done is True or self.__memory.is_memory_full() is True):

            cmd_payload = struct.pack("B", CMD_ID_SET_TRAINING_STATE)
            self.data_sent = self.__serialmux.send_data(COMMAND_CHANNEL_NAME, cmd_payload)

            # Failed to send data. Appends the data to unsent_data List.
            if self.data_sent is False:
                self.unsent_data.append((COMMAND_CHANNEL_NAME, cmd_payload))

            # Stopping condition for sequence was reached.
            self.reinitialize(robot_node)
            self.state = TRAINING

        # Checks whether the sequence has ended if it is set to driving mode.
        if (self.train_mode is False) and (self.done is True):
            self.done = False
            motorcontrol = struct.pack("2H", 0, 0)
            cmd_payload = struct.pack("B", CMD_ID_SET_READY_STATE)

            self.data_sent = self.__serialmux.send_data(
                SPEED_SET_CHANNEL_NAME, motorcontrol
            )  # stop the motors immediately

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append((SPEED_SET_CHANNEL_NAME, motorcontrol))

            self.reinitialize(robot_node)
            self.data_sent = self.__serialmux.send_data(COMMAND_CHANNEL_NAME, cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append((COMMAND_CHANNEL_NAME, cmd_payload))
            self.state = IDLE

    def normalize_sensor_data(self, sensor_data):
        """
        The normalize_sensor_data function scales the sensor data to a range between 0 and 1.
        
        Parameters
        ----------
            sensor_data: The state observed.

        Returns
        ----------
            NumPy array of float32: Normalized Sensor Data
        """

        normalized_sensor_data = np.array(sensor_data) / MAX_SENSOR_VALUE
        return normalized_sensor_data

    def determine_reward(self, sensor_data):
        """
        The function evaluates the consequences of a certain
        action performed in a certain state by calculating the resulting reward.
        A reward of 1 means that the robot is in the center of the Line.

        Parameters
        ----------
            sensor_data : The state observed.

        Returns
        ----------
            float: The Resulting Reward.

        """
        reward = self.__memory.calculate_reward(sensor_data)
        return reward

    def learn(self, states, actions, old_probs, values, rewards, advantages):
        """
        Perform training to optimize model weights.

        Parameters
        ----------
            states:     The saved states observed during interactions with the environment.
            actions:    The saved actions taken in response to the observed states.
            old_probs:  The saved probabilities of the actions taken, based on the previous policy.
            values:     The saved estimated values of the observed states.
            rewards:    The saved rewards received for taking the actions.
            advantages: the computed advantage values for each state in a given Data size. 
        """
        # scales the sensor data to a range between 0 and 1
        m_states = self.normalize_sensor_data(states)

        for _ in range(self.n_epochs):

            states = tf.convert_to_tensor(m_states)
            actions = tf.convert_to_tensor(actions)
            old_probs = tf.convert_to_tensor(old_probs)

            # optimize Actor Network weights
            self.__neural_network.compute_actor_gradient(states, actions, old_probs, advantages)

            # optimize Critic Network weights
            self.__neural_network.compute_critic_gradient(states, values, advantages)

            # Save the rewards received
            self.reward_history.append(sum(rewards))

        # saving logs in a CSV file
        self.save_logs_to_csv()

    def save_logs_to_csv(self):
        """Function for saving logs in a CSV file"""

        # Ensure the directory exists
        log_dir = DIRECTORY
        os.makedirs(log_dir, exist_ok=True)
        log_file = os.path.join(log_dir, FILE_DIRECTORY)

        with open(log_file, mode="w", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(["Actor Loss", "Critic Loss", "Reward"])
            for indx, reward in enumerate(self.reward_history):
                writer.writerow(
                    [
                        self.__neural_network.actor_loss_history[indx],
                        self.__neural_network.critic_loss_history[indx],
                        reward,
                    ]
                )

    def perform_training(self):
        """Runs the training process."""

        if self.__current_batch is None:

            # Grab sample from memory
            self.__current_batch = self.__memory.generate_batches()

        # Perform training with mini batches.
        if self.__training_index < len(self.__current_batch[-1]):
            (
                state_arr,
                action_arr,
                old_prob_arr,
                vals_arr,
                reward_arr,
                advatage_arr,
                batches,
            ) = self.__current_batch
            batch = batches[self.__training_index]

            # pylint: disable=too-many-arguments
            self.learn(
                state_arr[batch],
                action_arr[batch],
                old_prob_arr[batch],
                vals_arr[batch],
                reward_arr[batch],
                advatage_arr[batch]
            )
            self.__training_index += 1

        # Training completed.
        else:
            self.__training_index = 0
            self.__current_batch = None
            self.done = False
            self.__memory.clear_memory()
            self.state = IDLE
            self.num_episodes += 1
            cmd_payload = struct.pack("B", CMD_ID_SET_READY_STATE)
            self.data_sent = self.__serialmux.send_data(COMMAND_CHANNEL_NAME, cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append((COMMAND_CHANNEL_NAME, cmd_payload))

            # Minimize standard deviation until the minimum standard deviation is reached
            self.__std_dev = self.__std_dev * STD_DEV_FACTOR
            self.__std_dev = max(self.__std_dev, MIN_STD_DEV)

    def reinitialize(self, robot_node):
        """
        Re-initialization of position and orientation of The ROBOT.

        Parameters
        ----------
            robot_node: The Robot interface
        """
        trans_field = robot_node.getField(TRANSLATION_FIELD)
        rot_field = robot_node.getField(ROTATION_FIELD)
        initial_position = POSITION_DATA
        initial_orientation = ORIENTATION_DATA
        trans_field.setSFVec3f(initial_position)
        rot_field.setSFRotation(initial_orientation)


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
