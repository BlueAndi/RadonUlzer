""" Implementation of a Trajectory Buffer """

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

import numpy as np

################################################################################
# Variables
################################################################################


################################################################################
# Classes
################################################################################


class Memory:  # pylint: disable=too-many-instance-attributes
    """Class for store and manage experience tuples during Reinforcement learning."""

    # pylint: disable=too-many-arguments
    def __init__(self, batch_size, max_length, gamma, gae_lambda, min_buffer_length=512):
        self.__states = []
        self.__probs = []
        self.__vals = []
        self.__actions = []
        self.__rewards = []
        self.__dones = []
        self.__advatages = []
        self.__batch_size = batch_size
        self.__max_length = max_length
        self.__min_buffer_length = min_buffer_length
        self.__gamma = gamma
        self.__gae_lambda = gae_lambda
        self.__current_index = 0

    def generate_batches(self):
        """
        Generates batches of data for training.

        Returns
        ----------
        Numpy-Array: States
        Numpy-Array: Actions
        Numpy-Array: Probs
        Numpy-Array: Vals
        Numpy-Array: Rewards
        Numpy-Array: Raw advantages
        Numpy-Array: Normalized advantages
        List: Batches
        """

        # Determine the number of states
        n_states = len(self.__states)

        # Calculate start indices for each batch
        batch_start = np.arange(0, n_states, self.__batch_size)

        # Create indices for the states and mix them randomly
        indices = np.arange(n_states, dtype=np.int64)
        np.random.shuffle(indices)

        # Create batches by dividing the indices into groups of the batch_size
        batches = [indices[indx : indx + self.__batch_size] for indx in batch_start]

        # the computed advantage values for each state in a given Data size.
        self.__advatages = self.calculate_advantages(self.__rewards,
                                                    self.__vals, self.__dones)

        # Normalize advantages over the complete trajectory before splitting
        # them into mini-batches. This keeps the scale of the policy updates
        # stable while preserving whether an advantage is above or below the
        # trajectory average.
        advantage_mean = np.mean(self.__advatages)
        advantage_std = np.std(self.__advatages)
        normalized_advantages = (
            self.__advatages - advantage_mean
        ) / (advantage_std + 1e-8)

        return (
            np.array(self.__states, dtype=np.float32),
            np.array(self.__actions, dtype=np.float32),
            np.array(self.__probs, dtype=np.float32),
            np.array(self.__vals, dtype=np.float32),
            np.array(self.__rewards, dtype=np.float32),
            np.array(self.__advatages, dtype=np.float32).reshape(-1, 1),
            np.array(normalized_advantages, dtype=np.float32).reshape(-1, 1),
            batches,

        )

    def get_sum_rewards(self) -> float:
        """
        Calculate total rewards.

        Returns
        ----------
            float: Total rewards received.
        """
        sum_rewards = sum(self.__rewards)

        return sum_rewards

    def get_episode_rewards(self) -> list[float]:
        """
        Calculate total rewards per completed episode.

        Returns
        ----------
            list[float]: Total rewards for completed episodes.
        """
        episode_rewards = []
        current_reward = 0.0

        for reward, done in zip(self.__rewards, self.__dones):
            current_reward += reward
            if done:
                episode_rewards.append(current_reward)
                current_reward = 0.0

        return episode_rewards

    def store_memory(
        self, state, action, probs, vals, reward, done
    ):  # pylint: disable=too-many-arguments
        """
        Store transitions in the replay buffer.

        Parameters
        ----------
            state: The state observed.
            action: The action taken.
            probs: The probability of taking the action.
            vals: The estimated value of the state.
            reward: The reward received.
            done: Whether the episode is done.
        """
        self.__states.append(state)
        self.__actions.append(action)
        self.__probs.append(probs)
        self.__vals.append(vals)
        self.__rewards.append(reward)
        self.__dones.append(done)
        self.__current_index += 1

    def clear_memory(self):
        """Remove transitions from the trajektories buffer."""

        self.__states = []
        self.__probs = []
        self.__vals = []
        self.__actions = []
        self.__rewards = []
        self.__dones = []
        self.__advatages = []
        self.__current_index = 0

    def is_memory_full(self):
        """
        Checks whether Memory has reached its maximum capacity.

        Returns
        ----------
        - Bool: Memory is full or not
        """
        return self.__current_index >= self.__max_length

    def is_ready_for_training(self):
        """
        Checks whether enough transitions have been collected to run a training update.

        Returns
        ----------
        - Bool: Buffer has at least min_buffer_length transitions
        """
        return self.__current_index >= self.__min_buffer_length

    def calculate_advantages(self, rewards, values, dones):
        """
        Calculate the generalized advantage estimate (GAE) for every
        transition.

        The advantage describes how much better or worse the received result
        was compared with the value predicted by the critic. The calculation
        runs backwards because each advantage depends on the already
        calculated advantage of its successor.

        Parameters
        ----------
            rewards: The rewards received.
            values: The estimated values of the states.
            dones: Indicates whether an episode ended after a transition.

        Returns
        ----------
            NumPy array of float32: The advantage for every transition.
        """
        # Critic outputs are stored as one-element arrays. Flatten them
        # so every value used in the calculation is a scalar.
        values = np.asarray(values, dtype=np.float32).reshape(-1)

        data_size = len(rewards)
        advantages = np.zeros(data_size, dtype=np.float32)

        # Accumulated advantage of the transitions following the current one.
        gae = 0.0

        if not (len(rewards) == len(values) == len(dones)):
            raise ValueError("rewards, values and dones must have the same length")

        # A backwards pass calculates all advantages in O(n).
        for index in reversed(range(data_size)):
            # The mask is 0 at an episode end and 1 while the episode
            # continues. It prevents future values from crossing that limit.
            is_not_terminal = 1.0 - dones[index]

            # The final stored transition has no stored successor. At a
            # regular episode end, its expected future value is zero.
            if index == data_size - 1:
                next_value = 0.0
            else:
                next_value = values[index + 1]

            # One-step temporal-difference error:
            delta = (
                rewards[index]
                + self.__gamma * next_value * is_not_terminal
                - values[index]
            )

            # Include the discounted advantage of following transitions.
            gae = (
                delta
                + self.__gamma
                * self.__gae_lambda
                * is_not_terminal
                * gae
            )
            advantages[index] = gae

        return advantages

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
            sensor_data : The state observed.

        Returns
        ----------
            float: Estimated position with respect to track.
        """
        estimated_pos = 0.0
        numerator = 0.0
        denominator = 0.0
        weight = 1000.0

        for idx, sensor_value in enumerate(sensor_data):
            numerator += idx * weight * sensor_value
            denominator += sensor_value

        if denominator > 0:
            estimated_pos = numerator / denominator

        return estimated_pos

    def calculate_reward(self, sensor_data):
        """
        The calculate_reward function evaluates the consequences of a certain
        action performed in a certain state by calculating the resulting reward.
        A reward of 1 means that the robot is in the center of the Line.

        Parameters
        ----------
            sensor_data : The state observed.

        Returns
        ----------
            float: the Resulting Reward
        """
        reward = 0.0
        estimated_pos = self.calculate_position(sensor_data)

        # Reward scaled between 0 and 1 based on the estimated position
        if 500 <= estimated_pos <= 3500:
            reward = 1.0 - abs(estimated_pos - 2000.0) / 1500.0

        return reward
