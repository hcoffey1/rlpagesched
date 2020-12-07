#include "rl.h"
#include <stdlib.h>

ulong epsilon = 100;
double lr = 0.1;
double discount = 0.9;
int CYCLE = 0;
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

void setQValue(state s, qvalue *Q, double update) {
  //printf("Accessing %d:\n", s.hits + s.old_device * (Q->x) + s.new_device * (Q->x) * (Q->y));
  Q->Q[s.hits + s.p1_hits*(Q->x0) + s.old_device * (Q->x0) * Q->x1 + s.new_device * (Q->x0) * (Q->x1) * (Q->y)] =
      update;
}

double getQValue(state s, qvalue Q) {
  //return Q.Q[s.hits + s.old_device * Q.x + s.new_device * Q.x * Q.y];
  return Q.Q[s.hits + s.p1_hits*(Q.x0) + s.old_device * Q.x0 * Q.x1 + s.new_device * Q.x0 * Q.x1 * Q.y];
}

void updateQValue(ulong index, int ps_count, state *s, qvalue *Q, long reward) {
  double q = getQValue(s[index], *(Q + index));
  double q_new = getQValue(s[index + ps_count], *(Q + index));
  double update = q + lr * ((double)reward + discount * q_new - q);
  //printf("Reward : %lf %lf %lf %lf %ld\n",q, lr, discount, q_new, reward);
  //printf("%lf\n", update);
  //printf("Reward : %lf %lf %lf %lf %ld\n",q, lr, discount, q_new, reward);
  //exit(1);

  //This function is cursed
  setQValue(s[index], Q + index, update);

#if 0
    printf("=================================================\n");
    printf("INDEX %d\n", index);
    printf("Updated : %d %d %d\n", s->hits, s->old_device, s->new_device);
    for(int i = 0; i < Q->x; i++)
    {
        for(int j = 0; j < Q->y; j++)
        {
            printf("[");
            for(int z = 0; z < Q->z; z++)
            {
                printf("%lf", (Q+index)->Q[i + j * Q->x + z * Q->x * Q->y]);
                printf("\t");
            }
            printf("]\t");
        }
        printf("\n");
    }
    printf("=================================================\n");
    //if(s->hits == 1)
    //{
      //exit(1);
    //}
#endif
  // double q = Q->Q[s[index + ps_count*0].old_device][s[index +
  // ps_count*0].hits*Q->x][s[index + ps_count*0].new_device*Q->x*Q->y];
  // Q->Q[s.old_device][s.hits*Q->x][s.new_device*Q->x*Q->y] = q + lr * (reward
  // + discount*Q->Q[s.new_device][])
}

ACTION getAction(state s, qvalue Q) {
  s.new_device = 0;
  double m1_q = getQValue(s, Q);
  s.new_device = 1;
  double m2_q = getQValue(s, Q);

  #if 0
  if(s.hits == 1)
  {
    printf("HITS 1 : %lf : %lf\n", m1_q, m2_q);
  }
  #endif

  ulong r = rand() % 10000;
  if (r >= epsilon) {
    return (m1_q > m2_q) ? m1 : m2;
  } else {
    return (r % 2) ? m1 : m2;
  }
}

// Make decision for page
ACTION rl_schedule_page(state *s, qvalue *Q) {
  #if 0
  printf("Action State: %d %d %d\n", s->hits, s->old_device, s->new_device);
  printf("Chose action %d\n", getAction(*s, *(Q)));
  #endif
   return getAction(*s, *(Q)); 
   }
// Actions
// Place in m1
// Place in m2

// Reward Model
// Negative delay value for epoch?