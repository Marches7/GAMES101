#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
//        Comment-in this part when you implement the constructor
        // mass point 质点
        Vector2D unitPosition = (end - start) / ((double)num_nodes - 1.0); // 每个质点的单位距离
        // for(int i=0; i<num_nodes; ++i) {
        //     Vector2D pos = start + (end - start) * ((double)i / ((double)num_nodes - 1.0));          
        //     masses.push_back(new Mass(pos, node_mass, false));
        //     // masses[i]->forces = Vector2D(0, 0);
        // }

        // for(int i=0; i<num_nodes-1; ++i) {
        //     springs.push_back(new Spring(masses[i], masses[i+1], k));
        // }
        for(int i = 0; i < num_nodes; i++)
        {
            Vector2D position = start + (double)i * unitPosition; // 确定每个质点的位置
            Mass *m = new Mass(position, node_mass, false);
            masses.push_back(m);
        }

        for (auto &i : pinned_nodes) {
            // masses[i]->position = unitPosition;
            masses[i]->pinned = true; // 固定结点
        }
        // 在结点间建立弹簧
        
        for(int i = 0; i < num_nodes -1; i++)
        {
            Spring *s = new Spring(masses[i], masses[i+1], k);
            springs.push_back(s);
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            s->m2->forces += - (s->k) * (s->m2->position - s->m1->position) / (s->m2->position - s->m1->position).norm() 
                * ((s->m2->position - s->m1->position).norm() - s->rest_length);
            s->m1->forces += - s->m2->forces;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += m->mass * gravity;
                // TODO (Part 2): Add global damping
                // m->forces += - m->velocity * 0.01; //添加一个与速度相反的力，达到减速的目的

                m->velocity = m->velocity + m->forces / m->mass * delta_t;
                m->position = m->position + m->velocity * delta_t; // 半隐式欧拉法，显式欧拉法是根据上一时刻的速度，因此绳子会飞掉
                
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            // s->m2->forces += - (s->k) * (s->m2->position - s->m1->position) / (s->m2->position - s->m1->position).norm() 
            //     * ((s->m2->position - s->m1->position).norm() - s->rest_length);
            // s->m1->forces += - s->m2->forces;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->forces += m->mass * gravity;
                // TODO (Part 4): Add global Verlet damping
                // m->forces += - m->velocity * 0.01; //添加一个与速度相反的力，达到减速的目的
                Vector2D a = m->forces / m->mass;
                float damping_factor = 0.005; // 阻尼系数
                m->position = m->position + (1 - damping_factor) * (m->position - m->last_position) + a * delta_t * delta_t;
                m->last_position = temp_position; // 保存上一个位置
                
            }
        }
    }
}
