"""Implementing Actor and Critic Networks"""

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

import tensorflow as tf  # pylint: disable=import-error
from tensorflow.keras import layers  # pylint: disable=import-error
from tensorflow.keras.regularizers import l2  # pylint: disable=import-error

################################################################################
# Variables
################################################################################


################################################################################
# Classes
################################################################################


class Networks:
    """Class for building networks of actors and critics"""

    def __init__(self, alpha):
        self.__learning_rate = alpha
        self.actor_network = self.build_actor_network()
        self.critic_network = self.build_critic_network()
        self.actor_optimizer = tf.keras.optimizers.Adam(self.__learning_rate)
        self.critic_optimizer = tf.keras.optimizers.Adam(self.__learning_rate)

    def build_actor_network(self):
        """Build Actor Network."""

        state_input = layers.Input(shape=(5,))  # Assuming 5 sensor inputs
        fc1 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(state_input)
        fc2 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(fc1)
        fc3 = layers.Dense(
            32,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(fc2)
        mean = layers.Dense(
            1,
            activation="tanh",
            kernel_initializer="glorot_uniform",
            bias_initializer="zeros",
        )(fc3)

        return tf.keras.models.Model(inputs=state_input, outputs=mean)

    def build_critic_network(self):
        """Build Critic Network"""

        state_input = layers.Input(shape=(5,))  # Assuming 5 sensor inputs
        fc1 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(state_input)
        fc2 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(fc1)
        fc3 = layers.Dense(
            32,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.01),
            bias_initializer="zeros",
        )(fc2)
        value = layers.Dense(1)(fc3)  # Value output

        return tf.keras.models.Model(inputs=state_input, outputs=value)


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
