//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection pos = intersect(ray); // 计算视线与场景中物体交点
    if(!pos.happened) //如果没有打到任何物体，则直接返回
        return Vector3f(0, 0, 0);
    if(pos.m->hasEmission()) // 打到光源
    {
        if(depth == 0) // 第一次打到光源
            return pos.m->getEmission(); // 返回光源亮度
        else
            return Vector3f(0, 0, 0); // 吸收光线，不做进一步的弹射
    }
    // Vector3f wo = pos.m->sample(ray.direction, pos.normal); // 计算出射方向
    // float pdf_light = pos.m->pdf(ray.direction, wo, pos.normal);
    float pdf_light;
    Intersection light_inter; // 与光源交点
    sampleLight(light_inter, pdf_light); // 采样光源
    // std::cout<<"pdf_light = "<<pdf_light<<std::endl;
    // Vector3f wi = pos.coords - ray.origin; //计算入射光方向
    // Vector3f wo = pos.m->sample(wi, pos.normal);
    Vector3f emit = light_inter.emit; // 光源辐射的亮度
    Vector3f x = light_inter.coords; // 光源坐标
    Vector3f N = pos.normal.normalized(); // 交点的法向量(需要单位化)
    Vector3f ws = (light_inter.coords - pos.coords).normalized(); // 计算出射方向(需要单位化)
    Vector3f NN = light_inter.normal.normalized(); // 光源所在平面法向量(需要单位化)
    Vector3f wo = ray.direction; // 入射方向
    Vector3f L_dir;
    Ray l2p(light_inter.coords, - ws); // 从light_inter发出一条光线到pos
    double dis = (light_inter.coords - pos.coords).norm();
    Intersection inter = intersect(l2p);
    if(inter.happened && std::abs(inter.distance - dis) < 0.01) // 打到光源(解释：打到的表面是发光，说明打到了光源)
    {
        L_dir = emit * pos.m->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) 
                            / std::pow((x - pos.coords).norm(), 2) / pdf_light;
    }
    // std::cout<<"L_dir.y = "<<L_dir.y<<std::endl;
    Vector3f L_indir(0, 0, 0);

    //俄罗斯轮盘赌，计算其他物体反射的光线对着色点的影响
    float ksi = get_random_float();
    
    if(ksi < RussianRoulette)
    {
        // std::cout << "ksi1 = " << ksi <<std::endl;
        Vector3f wi = pos.m->sample(wo, N).normalized(); // 采样一个出射方向
        // Vector3f wi;
        // std::cout << "ksi2 = " << ksi <<std::endl;
        Ray r(pos.coords, wi);
        Intersection q = intersect(r);
        if(q.happened && !q.m->hasEmission()) //打到了并且不会发光，说明打到了不发光的物体
        {
            L_indir = castRay(r, depth + 1) * q.m->eval(wo, wi, N) * dotProduct(wi, N) 
                        / q.m->pdf(wo, wi, N) / RussianRoulette;
        }
    }
    return L_dir + L_indir;
}
