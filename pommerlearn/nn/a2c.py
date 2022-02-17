import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.distributions import Categorical
import numpy as np
from torch.nn import Sequential, Conv2d, BatchNorm2d, ReLU, LeakyReLU, Sigmoid, Tanh, Linear, Hardsigmoid, Hardswish, \
    Flatten

from nn.PommerModel import PommerModel
from nn.builder_util import get_act, _Stem, _PolicyHead, _ValueHead


class A2CNet(PommerModel):

    def __init__(
        self,
        n_labels=6,
        channels=45,
        nb_input_channels=18 + 3,
        board_height=11,
        board_width=11,
        bn_mom=0.9,
        act_type="relu"
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

        print("INIT A2C")

        super(A2CNet, self).__init__(nb_input_channels=nb_input_channels, board_height=board_height,
                                     board_width=board_height, is_stateful=False, state_batch_dim=None)

        self.nb_flatten = board_height * board_width * channels

        self.body = Sequential(Conv2d(in_channels=nb_input_channels, out_channels=channels,
                                      kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type),
                               Conv2d(in_channels=channels, out_channels=channels,
                                      kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type),
                               Conv2d(in_channels=channels, out_channels=channels,
                                      kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type),
                               Conv2d(in_channels=channels, out_channels=channels,
                                      kernel_size=(3, 3), padding=(1, 1), bias=False),
                               BatchNorm2d(momentum=bn_mom, num_features=channels),
                               get_act(act_type),
                               Flatten(),
                               Linear(in_features=self.nb_flatten, out_features=1024),
                               get_act(act_type),
                               Linear(in_features=1024, out_features=512),
                               get_act(act_type),
                               Linear(in_features=512, out_features=512),
                               get_act(act_type),
                               Linear(in_features=512, out_features=512),
                               get_act(act_type)
                               )

        self.policy_net = nn.Linear(512, 64)
        self.value_net = nn.Linear(512, 64)

        # create the two heads which will be used in the hybrid fwd pass
        self.policy_head = nn.Linear(64, n_labels)
        self.value_head = nn.Linear(64, 1)

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
        policy_net = self.policy_net(out)
        value_net = self.value_net(out)

        value = self.value_head(value_net)
        policy = self.policy_head(policy_net)

        return value, policy

    def init_rnn(self):
        device = self.device
        s = self.rnn_hidden_size
        return torch.zeros(s).detach().numpy(), torch.zeros(s).detach().numpy()

    def discount_rewards(self, _rewards):
        R = 0
        gamma = self.gamma
        rewards = []
        for r in _rewards[::-1]:
            R = r + gamma * R
            rewards.insert(0, R)

        return rewards

    def get_state_shape(self, batch_size: int):
        raise NotImplementedError
