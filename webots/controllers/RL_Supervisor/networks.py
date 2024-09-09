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

import tensorflow as tf               # pylint: disable=import-error
import tensorflow_probability as tfp  # pylint: disable=import-error
from tensorflow import keras          # pylint: disable=import-error
from keras import layers              # pylint: disable=import-error
from keras.regularizers import l2     # pylint: disable=import-error

################################################################################
# Variables
################################################################################

# Constants
NUM_SENSORS = 5  # Assuming 5 sensor inputs

################################################################################
# Classes
################################################################################


class Models:  # pylint: disable=too-many-instance-attributes
    """Class for building networks of actors and critics."""

    def __init__(self, actor_alpha, critic_alpha, std_dev, policy_clip):
        self.__actor_learning_rate = actor_alpha
        self.__critic_learning_rate = critic_alpha
        self.actor_network = self.build_actor_network()
        self.critic_network = self.build_critic_network()
        self.std_dev = std_dev
        self.policy_clip = policy_clip
        self.actor_optimizer = keras.optimizers.Adam(self.__actor_learning_rate)
        self.critic_optimizer = keras.optimizers.Adam(self.__critic_learning_rate)
        self.critic_loss_history = []
        self.actor_loss_history = []

    def build_actor_network(self):
        """Build Actor Network."""

        state_input = layers.Input(shape=(NUM_SENSORS,))
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

        return keras.models.Model(inputs=state_input, outputs=mean)

    def build_critic_network(self):
        """Build Critic Network"""

        state_input = layers.Input(shape=(NUM_SENSORS,))
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

        return keras.models.Model(inputs=state_input, outputs=value)

    def compute_critic_gradient(self, states, values, advantages):
        """ optimize Critic Network weights.
        
        Parameters
        ----------
            states:     The saved states observed during interactions with the environment.
            values:     The saved estimated values of the observed states.
            advantages: the computed advantage values for each state in a given Data size. 
        """

        with tf.GradientTape() as tape:

            # The critical value represents the expected return from state 𝑠𝑡.
            # It provides an estimate of how good it is to be in a given state.
            critic_value = self.critic_network(states)

            # the total discounted reward accumulated from time step 𝑡
            estimate_returns = advantages + values

            # Generate loss
            critic_loss = tf.math.reduce_mean(
                tf.math.pow(estimate_returns - critic_value, 2)
            )
            # calculate gradient
            critic_params = self.critic_network.trainable_variables
            critic_grads = tape.gradient(critic_loss, critic_params)
            self.critic_optimizer.apply_gradients(
                zip(critic_grads, critic_params)
            )

            # save the critic Loss
            self.critic_loss_history.append(critic_loss.numpy())

    def calculate_adjusted_log_probability(self, states, actions):
        """ The function computes the logarithmic probability of a given action.

        Parameters
        ----------
            states:  The saved states observed during interactions with the environment.
            values:  The saved estimated values of the observed states.

        Returns
        ---------- 
            A TensorFlow tensor with the data type float32: The logarithmic probability 
                                                            of a given action

        """
        # Forward pass through the actor network to get the action mean
        predict_mean =  self.actor_network(states)

        # Create the normal distribution with the predicted mean
        new_dist = tfp.distributions.Normal(predict_mean, self.std_dev)

        # Invert the tanh transformation to recover the original actions before tanh
        untransformed_actions = tf.atanh(actions)

        new_log_prob = new_dist.log_prob(untransformed_actions)

        # Compute the log of the Jacobian for the tanh transformation
        # adding 1e-6 ensures that the Value remains stable and avoids potential issues
        # during computation
        jacobian_log_det = tf.math.log(1 - tf.square(actions) + 1e-6)

        adjusted_log_prob = new_log_prob - jacobian_log_det

        return adjusted_log_prob

    def compute_actor_gradient(self, states, actions, old_probs, advantages):
        """ optimize Actor Network weights.

        Parameters
        ----------
            states:     The saved states observed during interactions with the environment.
            actions:    The saved actions taken in response to the observed states.
            old_probs:  The saved probabilities of the actions taken, based on the previous policy.
            values:     The saved estimated values of the observed states.
            advantages: the computed advantage values for each state in a given Data size. 
        """

        with tf.GradientTape() as tape:

            adjusted_new_log_prob = self.calculate_adjusted_log_probability(states, actions)

            # The ratio between the new model and the old model’s action log probabilities
            prob_ratio = tf.exp(adjusted_new_log_prob - old_probs)

            # If the ratio is too large or too small, it will be
            # clipped according to the surrogate function.
            weighted_probs = prob_ratio * advantages
            clipped_probs = tf.clip_by_value(
                prob_ratio, 1 - self.policy_clip, 1 + self.policy_clip
            )
            weighted_clipped_probs = clipped_probs * advantages

            # Policy Gradient Loss
            actor_loss = -tf.reduce_mean(
                tf.minimum(weighted_probs, weighted_clipped_probs)
            )

        # calculate gradient
        actor_params = self.actor_network.trainable_variables
        actor_grads = tape.gradient(actor_loss, actor_params)
        self.actor_optimizer.apply_gradients(
            zip(actor_grads, actor_params)
            )

        # save the Actor Loss
        self.actor_loss_history.append(actor_loss.numpy())


################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
