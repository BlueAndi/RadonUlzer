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

import numpy as np  # pylint: disable=import-error

################################################################################
# Variables
################################################################################


################################################################################
# Classes
################################################################################


class Buffer:  # pylint: disable=too-many-instance-attributes
    """Class for store and manage experience tuples during reinforcement learning"""

    # pylint: disable=too-many-arguments
    def __init__(self, batch_size, max_length, gamma, gae_lambda):
        self.__states = []
        self.__probs = []
        self.__vals = []
        self.__actions = []
        self.__rewards = []
        self.__dones = []
        self.__batch_size = batch_size
        self.__max_length = max_length
        self.__batch_size = batch_size
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
        Numpy-Array: dones
        List: Batches
        """

        # Determine the number of states
        n_states = len(self.__states)

        # Calculate start indices for each batch
        batch_start = np.arange(0, n_states, self.__batch_size)

        # Create indices for the states and mix them randomly
        indices = np.arange(n_states, dtype=np.int64)

        # Create batches by dividing the indices into groups of the batch_size
        batches = [indices[indx : indx + self.__batch_size] for indx in batch_start]

        return (
            np.array(self.__states),
            np.array(self.__actions),
            np.array(self.__probs),
            np.array(self.__vals),
            np.array(self.__rewards),
            np.array(self.__dones),
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

    def is_memory_full(self):
        """
        Checks whether Memory has reached its maximum capacity.

        Returns
        ----------
        - Bool: Memory is full or not
        """

        is_full = self.__current_index >= self.__max_length
        return is_full

    def calculate_advantages(self, rewards, values, dones):
        """
        The function measures how much better or worse an action 
        performed in a given state compared to the average action 
        the policy would take in that state.

        Parameters
        ----------
            rewards: The rewards received.
            values: The estimated values of the states.
            dones: Indicating whether the target sequence has been reached.

        Returns
        ----------
            NumPy array of float32: the computed advantage values for each 
            state in a given batch of experiences.
        """

        mini_batch_size = rewards.shape[0]

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
