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
from trajectory_buffer import Memory
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
STD_DEV_FACTOR = 0.99995  # Discounter standard deviation factor

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
        max_buffer_length=600,
    ):
        self.__serialmux = smp_server
        self.__gamma = gamma
        self.__alpha = alpha
        self.__gae_lambda = gae_lambda
        self.__policy_clip = policy_clip
        self.__chkpt_dir = chkpt_dir
        self.train_mode = False
        self.__top_speed = top_speed
        self.__memory = Memory(batch_size, max_buffer_length)
        self.__neural_network = Networks(self.__alpha)
        self.__training_index = 0  # Track batch index during training
        self.__current_batch = None  # Saving of the current batch which is in process
        self.__std_dev = 1
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
        self, state, action, probs, vals, reward, done
    ):  # pylint: disable=too-many-arguments
        """Store transitions in the replay buffer."""

        self.__memory.store_memory(state, action, probs, vals, reward, done)

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
        """Predicts an action based on the current state."""

        # Conversion of the state into a tensor
        m_state = self.normalize_sensor_data(state)
        state = tf.convert_to_tensor([m_state], dtype=tf.float32)

        # Calculation of probabilities by the Actor neural network
        probs = self.__neural_network.actor_network(state)

        if self.train_mode is True:
            # Create a normal distribution with the calculated probabilities and the standard deviation
            dist = tfp.distributions.Normal(probs, self.__std_dev)

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
        else:
            self.action = probs.numpy()[0]

        return self.action

    def send_motor_speeds(self, state):
        """Sends the motor speeds to the robot."""

        pre_action = self.predict_action(state)

        # Get motor speed difference
        speeddifference = self.__top_speed * pre_action

        # Get individual motor speeds. The sign of speedDifference
        # determines if the robot turns left or right.
        left_motor_speed = int(self.__top_speed - speeddifference)
        right_motor_speed = int(self.__top_speed + speeddifference)

        control_data = struct.pack("2H", left_motor_speed, right_motor_speed)
        self.data_sent = self.__serialmux.send_data("SPEED_SET", control_data)

        # Failed to send data. Appends the data to unsent_data List.
        if self.data_sent is False:
            self.unsent_data.append(("SPEED_SET", control_data))

    def update(self, robot_node):
        """Checks if the sequence has ended, performs updates, and saves the models."""

        # Checks whether the sequence has ended if it is set to Training mode.
        if (self.train_mode is True) and (
            (self.done is True) or (self.__memory.is_memory_full() is True)
        ):
            cmd_payload = struct.pack("B", CMD_ID_SET_TRAINING_STATE)
            self.data_sent = self.__serialmux.send_data("CMD", cmd_payload)

            # Failed to send data. Appends the data to unsent_data List
            if self.data_sent is False:
                self.unsent_data.append(("CMD", cmd_payload))

            self.reinitialize(robot_node)
            self.state = "TRAINING"

            # save models
            if self.__memory.get_sum_rewards() >= 1000.0:
                self.save_models()

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
        """The normalize_sensor_data function scales the sensor data to a range between 0 and 1."""

        # Normalized sensor data
        sensor_data = np.array(sensor_data) / MAX_SENSOR_VALUE

        return sensor_data

    def calculate_reward(self, sensor_data):
        """
        The calculate_reward function evaluates the consequences of a certain
        action performed in a certain state by calculating the resulting reward
        """
        estimated_pos = self.calculate_position(sensor_data)

        # Return reward between 0 and 10
        if 500 <= estimated_pos <= 2000:
            reward = ((1 / 150) * estimated_pos) - (10 / 3)
            return reward

        if 2000 < estimated_pos <= 3500:
            reward = ((-1 / 150) * estimated_pos) + (70 / 3)
            return reward

        return 0

    def calculate_position(self, sensor_data):
        """
        Determines the deviation and returns an estimated position of the robot
        with respect to a line. The estimate is made using a weighted average of
        the sensor indices multiplied by 1000, so that a return value of 0
        indicates that the line is directly below sensor 0, a return value of
        1000 indicates that the line is directly below sensor 1, 2000
        indicates that it's below sensor 2000, etc.  Intermediate values
        indicate that the line is between two sensors. The formula is:

                0*value0 + 1000*value1 + 2000*value2 + ...
            --------------------------------------------
                value0  +  value1  +  value2 + ...

        This function assumes a dark line (high values) surrounded by white
        (low values).

        Parameters
        ----------
        sensor_data : List

        Returns
        ----------
        Estimated position with respect to track.
        """

        estimated_pos = 0
        numerator = 0
        denominator = 0
        weight = 1000
        is_on_line = False

        max_sensors = len(sensor_data)
        sensor_max_value = 1000
        last_pos_value = 0

        for idx, sensor_value in enumerate(sensor_data):
            numerator += idx * weight * sensor_value
            denominator += sensor_value

            # Keep track of whether we see the line at all.
            if LINE_SENSOR_ON_TRACK_MIN_VALUE < sensor_value:
                is_on_line = True

        if False is is_on_line:

            # If it last read to the left of center, return 0.
            if last_pos_value < ((max_sensors - 1) * sensor_max_value) / 2:
                estimated_pos = 0

            # If it last read to the right of center, return the max. value.
            else:
                estimated_pos = (max_sensors - 1) * sensor_max_value

        else:
            # Check to avoid division by zero..
            if denominator != 0:
                estimated_pos = numerator / denominator

            last_pos_value = estimated_pos

        return estimated_pos

    def calculate_advantages(self, rewards, values, dones):
        """Calculate advantages for each state in a mini-batch."""

        mini_batch_size = len(rewards)

        # Create empty advantages array
        advantages = np.zeros(mini_batch_size, dtype=np.float32)

        for start_index in range(mini_batch_size):
            discount = 1
            advantage = 0

            for future_index in range(start_index, mini_batch_size - 1):

                # Calculate the temporal difference (TD)
                delta = (
                    rewards[future_index]
                    + (
                        self.__gamma
                        * values[future_index + 1]
                        * (1 - int(dones[future_index]))
                    )
                    - values[future_index]
                )

                # Accumulate the advantage using the discount factor
                advantage += discount * delta

                # Update the discount factor for the next step
                discount *= self.__gamma * self.__gae_lambda

                # Stop if a terminal state is reached
                if dones[future_index]:
                    break

            # Save the calculated advantage for the current state
            advantages[start_index] = advantage

        return advantages

    # pylint: disable=too-many-arguments
    # pylint: disable=too-many-locals
    def learn(self, states, actions, old_probs, values, rewards, dones):
        """Perform training to optimize model weights"""

        advantages = self.calculate_advantages(rewards, values, dones)

        # optimize Actor Network weights
        with tf.GradientTape() as tape:
            states = tf.convert_to_tensor(states)
            actions = tf.convert_to_tensor(actions)
            old_probs = tf.convert_to_tensor(old_probs)

            # Predicts an action based on the Saved states
            predict = self.__neural_network.actor_network(states)
            new_dist = tfp.distributions.Normal(predict, 0.1)
            new_log_prob = new_dist.log_prob(actions)
            transformed_action = tf.tanh(actions)
            jacobian_log_det = tf.math.log(1 - tf.square(transformed_action))
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

            critic_value = self.__neural_network.critic_network(states)
            returns = advantages + values
            # Generate loss
            critic_loss = tf.math.reduce_mean(tf.math.pow(returns - critic_value, 2))

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
            self.__current_batch = self.__memory.generate_batches()

        # Perform training with mini batchtes.
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
            self.__memory.clear_memory()
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
        """Re-initialization of position and orientation of The ROBOT."""
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
