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
import numpy as np  # pylint: disable=import-error
import tensorflow as tf  # pylint: disable=import-error
import tensorflow_probability as tfp  # pylint: disable=import-error
from trajectory_buffer import Buffer
from networks import Networks

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
POSITION_DATA = [-0.24713614078815466, 0.01, 0.013994298332013683]
ORIENTATION_DATA = [
    -1.0564747468923541e-06,
    8.746699709178704e-07,
    0.9999999999990595,
    1.5880805820884731,
]
MAX_SENSOR_VALUE = 1000
MIN_STD_DEV = 0.1  # Minimum standard deviation
STD_DEV_FACTOR = 0.9995  # Discounter standard deviation factor

################################################################################
# Classes
################################################################################


class Agent:  # pylint: disable=too-many-instance-attributes
    """The Agent class represents an intelligent agent that makes decisions to
    control motors based on the position of the robot."""

    # pylint: disable=too-many-arguments
    def __init__(
        self,
        smp_server,
        gamma=0.99,
        alpha=0.0003,
        gae_lambda=0.95,
        policy_clip=0.2,
        batch_size=64,
        chkpt_dir="models/",
        top_speed=2000,
        max_buffer_length=65536,
    ):
        self.__serialmux = smp_server
        self.__alpha = alpha
        self.__policy_clip = policy_clip
        self.__chkpt_dir = chkpt_dir
        self.train_mode = False
        self.__top_speed = top_speed
        self.__buffer = Buffer(batch_size, max_buffer_length, gamma, gae_lambda)
        self.__neural_network = Networks(self.__alpha)
        self.__training_index = 0  # Track batch index during training
        self.__current_batch = None  # Saving of the current batch which is in process
        self.__std_dev = 0.9
        self.n_epochs = 10
        self.done = False
        self.action = None
        self.value = None
        self.adjusted_log_prob = None
        self.num_episodes = 0
        self.state = "IDLE"
        self.data_sent = True
        self.unsent_data = []
        self.critic_loss_history = []
        self.actor_loss_history = []
        self.reward_history = []

    def set_train_mode(self):
        """Set the Agent mode to train mode."""
        self.train_mode = True
        self.state = "READY"

    def set_drive_mode(self):
        """Set the Agent mode to drive mode."""
        self.train_mode = False
        self.state = "READY"
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
        self.__buffer.store_memory(state, action, probs, value, reward, done)

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
        self.data_sent = self.__serialmux.send_data("SPEED_SET", control_data)

        # Failed to send data. Appends the data to unsent_data List.
        if self.data_sent is False:
            self.unsent_data.append(("SPEED_SET", control_data))

    def update(self, robot_node):
        """
        Checks if the sequence has ended and performs updates.

        Parameters
        ----------
            robot_node: The Robot interface
        """

        # Checks whether the sequence has ended if it is set to Training mode.
        if (self.train_mode is True) and (
            (self.done is True) or (self.__buffer.is_memory_full() is True)
        ):
            cmd_payload = struct.pack("B", CMD_ID_SET_TRAINING_STATE)
            self.data_sent = self.__serialmux.send_data("CMD", cmd_payload)

            # Failed to send data. Appends the data to unsent_data List.
            if self.data_sent is False:
                self.unsent_data.append(("CMD", cmd_payload))

            # Stopping condition for sequence was reached.
            self.reinitialize(robot_node)
            self.state = "TRAINING"

        # Checks whether the sequence has ended if it is set to driving mode.
        if (self.train_mode is False) and (self.done is True):
            self.done = False
            motorcontrol = struct.pack("2H", 0, 0)
            cmd_payload = struct.pack("B", CMD_ID_SET_READY_STATE)

            self.data_sent = self.__serialmux.send_data(
                "SPEED_SET", motorcontrol
            )  # stop the motors immediately

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append(("SPEED_SET", motorcontrol))

            self.data_sent = self.__serialmux.send_data("CMD", cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append(("CMD", cmd_payload))

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

        sensor_data = np.array(sensor_data) / MAX_SENSOR_VALUE
        return sensor_data

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
            float: the Resulting Reward

        """
        reward = self.__buffer.calculate_reward(sensor_data)
        return reward

    # pylint: disable=too-many-arguments
    # pylint: disable=too-many-locals
    def learn(self, states, actions, old_probs, values, rewards, dones):
        """
        Perform training to optimize model weights.

        Parameters
        ----------
            states:    The saved states observed during interactions with the environment.
            actions:   The saved actions taken in response to the observed states.
            old_probs: The saved probabilities of the actions taken, based on the previous policy.
            values:    The saved estimated values of the observed states.
            rewards:   The saved rewards received for taking the actions.
            dones:     The saved flags indicating whether the target sequence or episode has 
                        been completed.
        """
        for _ in range(self.n_epochs):

            #the computed advantage values for each state in a given batch of experiences.
            advantages = self.__buffer.calculate_advantages(rewards, values, dones)

            # optimize Actor Network weights
            with tf.GradientTape() as tape:
                states = tf.convert_to_tensor(states)
                actions = tf.convert_to_tensor(actions)
                old_probs = tf.convert_to_tensor(old_probs)

                # Forward pass through the actor network to get the action mean
                predict_mean = self.__neural_network.actor_network(states)

                # Create the normal distribution with the predicted mean
                new_dist = tfp.distributions.Normal(predict_mean, self.__std_dev)

                # Invert the tanh transformation to recover the original actions before tanh
                untransformed_actions = tf.atanh(actions)

                new_log_prob = new_dist.log_prob(untransformed_actions)

                # Compute the log of the Jacobian for the tanh transformation
                jacobian_log_det = tf.math.log(1 - tf.square(actions))
                adjusted_new_log_prob = new_log_prob - jacobian_log_det

                # The ratio between the new model and the old model’s action log probabilities
                prob_ratio = tf.exp(adjusted_new_log_prob - old_probs)

                # If the ratio is too large or too small, it will be
                # clipped according to the surrogate function.
                weighted_probs = prob_ratio * advantages
                clipped_probs = tf.clip_by_value(
                    prob_ratio, 1 - self.__policy_clip, 1 + self.__policy_clip
                )
                weighted_clipped_probs = clipped_probs * advantages

                # Policy Gradient Loss
                actor_loss = -tf.reduce_mean(
                    tf.minimum(weighted_probs, weighted_clipped_probs)
                )

            # calculate gradient
            actor_params = self.__neural_network.actor_network.trainable_variables
            actor_grads = tape.gradient(actor_loss, actor_params)
            self.__neural_network.actor_optimizer.apply_gradients(
                zip(actor_grads, actor_params)
            )

            # optimize Critic Network weights
            with tf.GradientTape() as tape:

                # The critical value represents the expected return from state 𝑠𝑡.
                # It provides an estimate of how good it is to be in a given state.
                critic_value = self.__neural_network.critic_network(states)

                # the total discounted reward accumulated from time step 𝑡
                estimate_returns = advantages + values

                # Generate loss
                critic_loss = tf.math.reduce_mean(
                    tf.math.pow(estimate_returns - critic_value, 2)
                )

                # calculate gradient
                critic_params = self.__neural_network.critic_network.trainable_variables
                critic_grads = tape.gradient(critic_loss, critic_params)
                self.__neural_network.critic_optimizer.apply_gradients(
                    zip(critic_grads, critic_params)
                )
                self.actor_loss_history.append(actor_loss.numpy())
                self.critic_loss_history.append(critic_loss.numpy())
                self.reward_history.append(sum(rewards))

                # saving logs in a CSV file
                self.save_logs_to_csv()

    def save_logs_to_csv(self):
        """Function for saving logs in a CSV file"""

        # Ensure the directory exists
        log_dir = "logs"
        os.makedirs(log_dir, exist_ok=True)
        log_file = os.path.join(log_dir, "training_logs.csv")

        with open(log_file, mode="w", encoding="utf-8", newline="") as file:
            writer = csv.writer(file)
            writer.writerow(["Actor Loss", "Critic Loss", "Reward"])
            for indx, reward in enumerate(self.reward_history):
                writer.writerow(
                    [
                        self.actor_loss_history[indx],
                        self.critic_loss_history[indx],
                        reward,
                    ]
                )

    def perform_training(self):
        """Runs the training process."""

        if self.__current_batch is None:

            # Grab sample from memory
            self.__current_batch = self.__buffer.generate_batches()

        # Perform training with mini batches.
        if self.__training_index < len(self.__current_batch[-1]):
            (
                state_arr,
                action_arr,
                old_prob_arr,
                vals_arr,
                reward_arr,
                dones_arr,
                batches,
            ) = self.__current_batch
            batch = batches[self.__training_index]

            self.learn(
                state_arr[batch],
                action_arr[batch],
                old_prob_arr[batch],
                vals_arr[batch],
                reward_arr[batch],
                dones_arr[batch],
            )
            self.__training_index += 1

        # Training completed.
        else:
            self.__training_index = 0
            self.__current_batch = None
            self.done = False
            self.__buffer.clear_memory()
            self.state = "IDLE"
            self.num_episodes += 1
            cmd_payload = struct.pack("B", CMD_ID_SET_READY_STATE)
            self.data_sent = self.__serialmux.send_data("CMD", cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append(("CMD", cmd_payload))

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
        trans_field = robot_node.getField("translation")
        rot_field = robot_node.getField("rotation")
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
