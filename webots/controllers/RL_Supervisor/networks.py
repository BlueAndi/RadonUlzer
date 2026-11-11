"""Implementing Actor and Critic Networks"""

# MIT License
#
# Copyright (c) 2023 - 2026 Andreas Merkle <web@blue-andi.de>
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

import tensorflow as tf
import tensorflow_probability as tfp
from tensorflow import keras
from keras import layers
from keras.regularizers import l2

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

    def __init__(self, actor_alpha, critic_alpha, policy_clip) -> None:
        """
        Initialize actor and critic networks with their optimizers.

        Parameters
        ----------
            actor_alpha: Learning rate of the actor optimizer.
            critic_alpha: Learning rate of the critic optimizer.
            policy_clip: PPO policy clipping range.

        Returns
        ----------
            None
        """

        self.__actor_learning_rate = actor_alpha
        self.__critic_learning_rate = critic_alpha
        self.actor_network = self.build_actor_network()
        self.critic_network = self.build_critic_network()
        self.policy_clip = policy_clip
        self.actor_optimizer = keras.optimizers.Adam(self.__actor_learning_rate)
        self.critic_optimizer = keras.optimizers.Adam(self.__critic_learning_rate)
        self.critic_loss_history = []
        self.actor_loss_history = []

    def build_actor_network(self) -> keras.Model:
        """
        Build the actor network.

        Returns
        ----------
            keras.Model: Actor network that predicts the action mean.
        """

        state_input = layers.Input(shape=(NUM_SENSORS,))
        fc1 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(state_input)
        fc2 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(fc1)
        fc3 = layers.Dense(
            32,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(fc2)
        mean = layers.Dense(
            1,
            activation="tanh",
            kernel_initializer="zeros",
            bias_initializer="zeros",
        )(fc3)

        return keras.models.Model(inputs=state_input, outputs=mean)

    def build_critic_network(self) -> keras.Model:
        """
        Build the critic network.

        Returns
        ----------
            keras.Model: Critic network that predicts state values.
        """

        state_input = layers.Input(shape=(NUM_SENSORS,))
        fc1 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(state_input)
        fc2 = layers.Dense(
            64,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(fc1)
        fc3 = layers.Dense(
            32,
            activation="relu",
            kernel_initializer="he_normal",
            kernel_regularizer=l2(0.001),
            bias_initializer="zeros",
        )(fc2)
        value = layers.Dense(1)(fc3)  # Value output

        return keras.models.Model(inputs=state_input, outputs=value)

    def compute_critic_gradient(self, states, values, advantages) -> None:
        """
        Optimize critic network weights.

        Parameters
        ----------
            states:     The saved states observed during interactions with the environment.
            values:     The saved estimated values of the observed states.
            advantages: Computed advantage values for each state.

        Returns
        ----------
            None
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

    def calculate_adjusted_log_probability(
        self, states, actions, std_dev
    ) -> tf.Tensor:
        """
        Calculate adjusted log probabilities for sampled actions.

        Parameters
        ----------
            states: Saved states observed during environment interactions.
            actions: Saved actions evaluated under the current policy.
            std_dev: Standard deviation used for the action distribution.

        Returns
        ----------
            tf.Tensor: Adjusted log probability for each action.
        """

        # Forward pass through the actor network to get the action mean
        predict_mean =  self.actor_network(states)

        # Create the normal distribution with the predicted mean
        new_dist = tfp.distributions.Normal(predict_mean, std_dev)

        # Invert the tanh transformation to recover the original actions before tanh
        untransformed_actions = tf.atanh(actions)

        new_log_prob = new_dist.log_prob(untransformed_actions)

        # Compute the log of the Jacobian for the tanh transformation
        # adding 1e-6 ensures that the Value remains stable and avoids potential issues
        # during computation
        jacobian_log_det = tf.math.log(1 - tf.square(actions) + 1e-6)

        adjusted_log_prob = new_log_prob - jacobian_log_det

        return adjusted_log_prob

    # pylint: disable=too-many-arguments
    def compute_actor_gradient(
        self, states, actions, old_probs, advantages, std_dev
    ) -> None:
        """
        Optimize actor network weights.

        Parameters
        ----------
            states: Saved states observed during environment interactions.
            actions: Actions sampled from the previous policy.
            old_probs: Log probabilities of the sampled actions under the previous policy.
            advantages: Normalized advantage values used for the policy update.
            std_dev: Standard deviation used for the current action distribution.

        Returns
        ----------
            None
        """

        with tf.GradientTape() as tape:

            adjusted_new_log_prob = self.calculate_adjusted_log_probability(
                states, actions, std_dev
            )

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
