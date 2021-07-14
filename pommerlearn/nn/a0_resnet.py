"""
@file: a0_resnet.py
Created on 25.05.20
@project: PommerLearn
@author: queensgambit

Description of the alpha zero architecture for pytorch:
Mastering the game of Go without human knowledge, Silver et al.
'The input features st are processed by a residual tower that consists of a single
convolutional block followed by either 19 or 39 residual blocks.
The convolutional block applies the following modules:
(1) A convolution of 256 filters of kernel size 3 ×​3 with stride 1
(2) Batch normalization 18
(3) A rectifier nonlinearity
Each residual block applies the following modules sequentially to its input:
(1) A convolution of 256 filters of kernel size 3 ×​3 with stride 1
(2) Batch normalization
(3) A rectifier nonlinearity
(4) A convolution of 256 filters of kernel size 3 ×​3 with stride 1
(5) Batch normalization
(6) A skip connection that adds the input to the block
(7) A rectifier nonlinearity
The output of the residual tower is passed into two separate ‘heads’ for
computing the policy and value. The policy head applies the following modules:
(1) A convolution of 2 filters of kernel size 1 ×​1 with stride 1
(2) Batch normalization
(3) A rectifier nonlinearity
(4) A fully connected linear layer that outputs a vector of size 192 +​ 1 =​ 362,
corresponding to logistic probabilities for all intersections and the pass move
The value head applies the following modules:
(1) A convolution of 1 filter of kernel size 1 ×​1 with stride 1
(2) Batch normalization
(3) A rectifier nonlinearity
(4) A fully connected linear layer to a hidden layer of size 256
(5) A rectifier nonlinearity
(6) A fully connected linear layer to a scalar
(7) A tanh nonlinearity outputting a scalar in the range [−​1, 1]
The overall network depth, in the 20- or 40-block network, is 39 or 79 parameterized layers, respectively,
for the residual tower, plus an additional 2 layers for the policy head and 3 layers for the value head.
We note that a different variant of residual networks was simultaneously applied
to computer Go33 and achieved an amateur dan-level performance; however, this
was restricted to a single-headed policy network trained solely by supervised learning.
Neural network architecture comparison. Figure 4 shows the results of a comparison between network architectures.
Specifically, we compared four different neural networks:
(1) dual–res: the network contains a 20-block residual tower, as described above,
followed by both a policy head and a value head. This is the architecture used in AlphaGo Zero.
(2) sep–res: the network contains two 20-block residual towers. The first tower
is followed by a policy head and the second tower is followed by a value head.
(3) dual–conv: the network contains a non-residual tower of 12 convolutional
blocks, followed by both a policy head and a value head.
(4) sep–conv: the network contains two non-residual towers of 12 convolutional
blocks. The first tower is followed by a policy head and the second tower is followed
by a value head. This is the architecture used in AlphaGo Lee.
Each network was trained on a fixed dataset containing the final 2 million
games of self-play data generated by a previous run of AlphaGo Zero, using
stochastic gradient descent with the annealing rate, momentum and regularization hyperparameters described for
the supervised learning experiment; however, cross-entropy and MSE components were weighted equally,
since more data was available.'
"""
import torch
import torch.nn as nn
from torch.nn import Sequential, Conv2d, BatchNorm2d, ReLU, LeakyReLU, Sigmoid, Tanh, Linear, Hardsigmoid, Hardswish

from nn.PommerModel import PommerModel
from nn.builder_util import get_act, _Stem, _PolicyHead, _ValueHead


def init_weights(m):
    if isinstance(m, nn.Linear):
        nn.init.kaiming_normal_(m.weight.data, nonlinearity="relu")


class ResidualBlock(torch.nn.Module):
    """
    Definition of a residual block without any pooling operation
    """

    def __init__(self, channels, bn_mom, act_type):
        """
        :param channels: Number of channels used in the conv-operations
        :param bn_mom: Batch normalization momentum
        :param act_type: Activation function to use
        """
        super(ResidualBlock, self).__init__()
        self.act_type = act_type

        self.body = Sequential(Conv2d(in_channels=channels, out_channels=channels, kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type),
                               Conv2d(in_channels=channels, out_channels=channels, kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type))

    def forward(self, x):
        """
        Implementation of the forward pass of the residual block.
        Uses a broadcast add operation for the shortcut and the output of the residual block
        :param x: Input to the ResidualBlock
        :return: Sum of the shortcut and the computed residual block computation
        """
        return x + self.body(x)


class AlphaZeroResnet(PommerModel):
    """ Creates the alpha zero net description based on the given parameters."""

    def __init__(
        self,
        n_labels=6,
        channels=256,
        nb_input_channels=18,
        board_height=11,
        board_width=11,
        channels_value_head=1,
        channels_policy_head=2,
        num_res_blocks=19,
        value_fc_size=256,
        bn_mom=0.9,
        act_type="relu",
        select_policy_from_plane=False,
    ):
        """
        :param n_labels: Number of labels the for the policy
        :param channels: Used for all convolution operations. (Except the last 2)
        :param channels_policy_head: Number of channels in the bottle neck for the policy head
        :param channels_value_head: Number of channels in the bottle neck for the value head
        :param num_res_blocks: Number of residual blocks to stack. In the paper they used 19 or 39 residual blocks
        :param value_fc_size: Fully Connected layer size. Used for the value output
        :param bn_mom: Batch normalization momentum
        :return: net description
        """

        super(AlphaZeroResnet, self).__init__(nb_input_channels=nb_input_channels, board_height=board_height, board_width=board_height,
                                              is_stateful=False, state_batch_dim=None)

        res_blocks = []
        for i in range(num_res_blocks):
            res_blocks.append(ResidualBlock(channels, bn_mom, act_type))

        self.body = Sequential(_Stem(channels=channels, bn_mom=bn_mom, act_type=act_type,
                                     nb_input_channels=nb_input_channels),
                               *res_blocks)

        # create the two heads which will be used in the hybrid fwd pass
        self.value_head = _ValueHead(board_height, board_width, channels, channels_value_head, value_fc_size, bn_mom, act_type)
        self.policy_head = _PolicyHead(board_height, board_width, channels, channels_policy_head, n_labels,
                                       bn_mom, act_type, select_policy_from_plane)

    def forward(self, flat_input):
        """
        Implementation of the forward pass of the full network
        Uses a broadcast add operation for the shortcut and the output of the residual block
        :param x: Input to the ResidualBlock
        :return: Value & Policy Output
        """
        # input shape processing
        x, state_bf = self.unflatten(flat_input)

        out = self.body(x)

        value = self.value_head(out)
        policy = self.policy_head(out)

        return value, policy

    def get_state_shape(self, batch_size: int):
        raise NotImplementedError
