#ifndef _RL_H
#define _RL_H

typedef unsigned char uchar;
typedef unsigned long ulong;
typedef enum ACTION {m1, m2} ACTION;

extern double lr;
extern double discount;

typedef struct state {
  uchar old_device;
  uchar new_device;
  ulong hits;
  ulong p1_hits;
} state;
/*
typedef struct policy {
  ACTION **pi;
  ulong x, y;
} policy;
*/
typedef struct qvalue{
    double * Q;
    ulong x0,x1,y,z;
}qvalue;


ACTION rl_schedule_page(state * s, qvalue * Q);
void updateQValue(ulong index, int ps_count, state * s, qvalue * Q, long reward);
#endif