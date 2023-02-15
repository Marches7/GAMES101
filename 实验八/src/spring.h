#ifndef SPRING_H
#define SPRING_H

#include "CGL/CGL.h"
#include "mass.h"

using namespace std;

namespace CGL {

struct Spring {
  Spring(Mass *a, Mass *b, float k)
      : m1(a), m2(b), k(k), rest_length((a->position - b->position).norm()) {}

  float k; // 劲度系数
  double rest_length; // 无外力长度

  Mass *m1; // 质点1
  Mass *m2; // 质点2
}; // struct Spring
}
#endif /* SPRING_H */
