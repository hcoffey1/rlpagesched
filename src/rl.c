#include "rl.h"
#include <stdlib.h>

ulong epsilon = 100;
double lr = 0.1;
double discount = 0.9;
int CYCLE = 0;

void setQValue(state s, qvalue *Q, double update) {
  // printf("Accessing %d:\n", s.hits + s.old_device * (Q->x) + s.new_device *
  // (Q->x) * (Q->y));
  Q->Q[s.hits + s.p1_hits * (Q->x0) + s.old_device * (Q->x0) * Q->x1 +
       s.new_device * (Q->x0) * (Q->x1) * (Q->y)] = update;
}

double getQValue(state s, qvalue Q) {
  // return Q.Q[s.hits + s.old_device * Q.x + s.new_device * Q.x * Q.y];
  return Q.Q[s.hits + s.p1_hits * (Q.x0) + s.old_device * Q.x0 * Q.x1 +
             s.new_device * Q.x0 * Q.x1 * Q.y];
}

void updateQValue(ulong index, int ps_count, state *s, qvalue *Q, long reward) {
  double q = getQValue(s[index], *(Q + index));
  double q_new = getQValue(s[index + ps_count], *(Q + index));
  double update = q + lr * ((double)reward + discount * q_new - q);

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
}

ACTION getAction(state s, qvalue Q) {
  s.new_device = 0;
  double m1_q = getQValue(s, Q);
  s.new_device = 1;
  double m2_q = getQValue(s, Q);

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
#endif
  return getAction(*s, *(Q));
}