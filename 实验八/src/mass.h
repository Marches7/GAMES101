#ifndef MASS_H
#define MASS_H

#include "CGL/CGL.h"
#include "CGL/vector2D.h"

using namespace CGL;

struct Mass {
  Mass(Vector2D position, float mass, bool pinned)
      : start_position(position), position(position), last_position(position),
        mass(mass), pinned(pinned) {}

  float mass; // 质点质量
  bool pinned; // 质点是否固定

  Vector2D start_position; // 质点起始位置
  Vector2D position; // 质点位置？

  // explicit Verlet integration 显式Verlet法

  Vector2D last_position; // 质点上一个位置

  // explicit Euler integration 显式欧拉法

  Vector2D velocity; // 速率
  Vector2D forces; // 受力
};

#endif /* MASS_H */
