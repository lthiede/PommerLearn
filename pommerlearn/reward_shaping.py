import numpy as np


def get_reward(reward, obs, old_obs, action, last_action, agent_id):

    agent_nr = agent_id + 10
    pos = get_position(obs, agent_nr)
    old_pos = get_position(old_obs, agent_nr)

    # reward stage 0:
    # teach the agent to move and not make invalid actions (move into walls, place bombs when you have no ammo)
    ammo = old_obs[15][0][0]
    if action != 5:
        if pos == old_pos:
            reward -= 0.03
    elif ammo == 0:
        reward -= 0.03

    # reward stage 1: teach agent to bomb walls (and enemies)
    # compute adjacent squares
    adj = [(i, j) for i in (-1, 0, 1) for j in (-1, 0, 1) if not ((i == j) or i + j == 0)]
    pos_arr = np.array(pos)
    adjacent = [pos_arr, pos_arr, pos_arr, pos_arr]
    adjacent = adjacent - np.asarray(adj)
    # limit adjacent squares to only include inside board
    adjacent = np.clip(adjacent, 0, 10)
    if action == 5 and ammo > 0:
        wood_board = obs[1]
        enemy_1_board = obs[11]
        teammate_board = obs[12]
        enemy_2_board = obs[13]
        for xy in adjacent:
            wood_val = wood_board[xy[0]][xy[1]]
            enemy_1_val = enemy_1_board[xy[0]][xy[1]]
            teammate_val = teammate_board[xy[0]][xy[1]]
            enemy_2_val = enemy_2_board[xy[0]][xy[1]]
            if wood_val == 1:
                reward += 0.2
            elif enemy_1_val == 1 or enemy_2_val == 1:
                reward += 0.5
            elif teammate_val == 1:
                reward -= 0.2

    # reward stage2: teach agent to not stand on or beside bombs
    # reward /= 4
    bomb_life = 11 - obs[5] * 11
    # if we stand on a bomb or next to bomb
    just_placed_bomb = np.logical_xor(last_action == 5, action == 5)
    if bomb_life[pos] > 0 and not just_placed_bomb:
        reward -= 0.1 * (9 - bomb_life[pos])
    for xy in adjacent:
        if bomb_life[xy[0]][xy[1]] > 0:
            reward -= 0.05 * (9 - bomb_life[xy[0]][xy[1]])

    # reward agent for picking up power-ups
    blast_strength = obs[14][0][0]
    old_blast_strength = old_obs[14][0][0]
    can_kick = obs[17][0][0]
    old_can_kick = old_obs[17][0][0]
    max_ammo = obs[16][0][0]
    old_max_ammo = old_obs[16][0][0]
    reward += (can_kick - old_can_kick) * 0.05
    reward += (max_ammo - old_max_ammo) * 0.05
    reward += (blast_strength - old_blast_strength) * 0.05

    return reward


def get_position(obs, agent_nr):

    raw_pos = np.where(obs[agent_nr] == np.amax(obs[agent_nr]))

    return raw_pos[0][0], raw_pos[1][0]
