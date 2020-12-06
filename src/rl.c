#include "rl.h"

double lr = 0.1;
double discount = 0.9;

/*
def q_learn_ep(policyType):
    """Performs an episode of Q-Learning"""
    t = 0
    term = False
    G = 0
    episode_r = []
    while not term and t <= EP_LIMIT:
        old_state = [a_cord.copy(), b_cord.copy()]
        action = pi([a_cord, b_cord], policyType)
        reward = get_reward(old_state, action)
        term = update_state(action)

        # Penalty if we exceed episode limit
        if t >= EP_LIMIT:
            reward = -10

        episode_r.append(reward)

        if not term:
            update = get_Q(old_state, action) + a*(reward + GAMMA*get_Q(
                [a_cord, b_cord], pi([a_cord, b_cord], GREEDY)) -
get_Q(old_state, action))

        # If we are in the terminal state, future state value is 0
        else:
            update = get_Q(old_state, action) + a * \
                (reward - get_Q(old_state, action))

        set_Q(old_state, action, update)

        t += 1

    for r in episode_r:
        G = GAMMA*G + r

    return G
    */

// Problem Model
// Model per page chosen

// State
// current device, hits, maybe hits from earlier epoch as well?


ACTION getAction(state s, qvalue Q) { return m1; }

void setQValue(state s, qvalue Q, double update) {
    Q.Q[s.old_device][s.hits*Q.x][s.new_device*Q.x*Q.y] = update;
    //Q.Q[s.old_device][s.hits*Q.x][s.new_device*Q.x*Q.y] = q + lr * (prev_reward + discount*Q.Q[s.new_device][])
}

double getQValue(state s, qvalue Q) {
    return Q.Q[s.old_device][s.hits*Q.x][s.new_device*Q.x*Q.y];
}

// Make decision for page
ACTION rl_schedule_page(state s, qvalue Q) {
    return getAction(s, Q);
}
// Actions
// Place in m1
// Place in m2

// Reward Model
// Negative delay value for epoch?