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


class Memory:  # pylint: disable=too-many-instance-attributes
    """Class for store and manage experience tuples during reinforcement learning"""

    def __init__(self, batch_size, max_length):
        self.__states = []
        self.__probs = []
        self.__vals = []
        self.__actions = []
        self.__rewards = []
        self.__dones = []
        self.__max_length = max_length
        self.__batch_size = batch_size

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
        np.random.shuffle(indices)

        # Create batches by dividing the indices into groups of the batch_size
        batches = [indices[indx : indx + self.__batch_size] for indx in batch_start]
        print("Number of Steps:", n_states)

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
        """Calculate total rewards"""
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
            prob: The probability of taking the action.
            val: The estimated value of the state.
            reward: The reward received.
            done: Whether the episode is done.
        """
        self.__states.append(state)
        self.__actions.append(action)
        self.__probs.append(probs)
        self.__vals.append(vals)
        self.__rewards.append(reward)
        self.__dones.append(done)

    def clear_memory(self):
        """Remove transitions from the replay buffer."""

        self.__states = []
        self.__probs = []
        self.__vals = []
        self.__actions = []
        self.__rewards = []
        self.__dones = []

    def is_memory_full(self):
        """
        Checks whether Memory  has reached its maximum capacity.

        Returns
        ----------
        - Bool: Memory is full or not
        """

        is_full = len(self.__states) >= self.__max_length
        return is_full
