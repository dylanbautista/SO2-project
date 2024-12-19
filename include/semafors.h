#ifndef __SEMAFOR_H__
#define __SEMAFOR_H__

#include <list.h>

struct semafor {
  int count;
  struct list_head blocked;
  struct list_head sems;
  int TID;
};

#endif