//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"


inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;

// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;

    // change the spp value to change sample ammount
    int spp = 32;
    std::cout << "SPP: " << spp << "\n";
    for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                      imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;

            Vector3f dir = normalize(Vector3f(-x, y, 1));
            for (int k = 0; k < spp; k++){
                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
            }
            m++;
        }
        UpdateProgress(j / (float)scene.height);
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}

// // Implementation of Path Tracing
// Vector3f Scene::castRay(const Ray& ray, int depth) const
// {
// 	Intersection inter = intersect(ray);

// 	if (inter.happened)//如果光线与场景有交点
// 	{
// 		//如果打到光源
// 		if (inter.m->hasEmission())
// 		{
// 			// 如果射线第一次打到光源，直接返回光源光
// 			if (depth == 0)
// 			{
// 				return inter.m->getEmission();
// 			}
// 			//若非射线经弹射打到光源，则在打到物体是判断，这里不做处理，返回0
// 			else return Vector3f(0, 0, 0);
// 		}

// 		// 如果打到物体
// 		Vector3f L_dir(0, 0, 0);
// 		Vector3f L_indir(0, 0, 0);

// 		//均匀采样光源物体，取光源上一点
// 		Intersection lightInter;
// 		float pdf_light = 0.0f;
// 		sampleLight(lightInter, pdf_light);

// 		// 物体表面法线
// 		auto& N = inter.normal;
// 		// 灯光表面法线
// 		auto& NN = lightInter.normal;
// 		//物体点坐标
// 		auto& objPos = inter.coords;
// 		//光源点坐标
// 		auto& lightPos = lightInter.coords;

// 		auto diff = lightPos - objPos;
// 		auto lightDir = diff.normalized();
// 		float lightDistance = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

// 		//从物体打向光源的感光线(感光线为光线传播的逆方向)
// 		Ray light(objPos, lightDir);
// 		//该光线与场景求交
// 		Intersection light2obj = intersect(light);

// 		// 如果反射击中光源
// 		if (light2obj.happened && (light2obj.coords - lightPos).norm() < 1e-2)
// 		{
// 			//获取改材质的brdf，这里的brdf为漫反射（brdf=Kd/pi）
// 			Vector3f f_r = inter.m->eval(ray.direction, lightDir, N);
// 			//直接光照光 = 光源光 * brdf * 光线和物体角度衰减 * 光线和光源法线角度衰减 / 光线距离 / 该点的概率密度（1/该光源的面积）
// 			L_dir = lightInter.emit * f_r * dotProduct(lightDir, N) * dotProduct(-lightDir, NN) / lightDistance / pdf_light;
// 		}

// 		//俄罗斯轮盘赌，确定是否继续弹射光线
// 		if (get_random_float() < RussianRoulette)
// 		{
// 			//获取半平面上的随机弹射方向
// 			Vector3f nextDir = inter.m->sample(ray.direction, N).normalized();
// 			//定义弹射光线
// 			Ray nextRay(objPos, nextDir);
// 			//获取相交点
// 			Intersection nextInter = intersect(nextRay);
// 			//如果有相交，且是与物体相交
// 			if (nextInter.happened && !nextInter.m->hasEmission())
// 			{
// 				//该点间接光= 弹射点反射光 * brdf * 角度衰减 / pdf(认为该点四面八方都接收到了该方向的光强，为1/(2*pi)) / 俄罗斯轮盘赌值(强度矫正值)
// 				float pdf = inter.m->pdf(ray.direction, nextDir, N);
// 				Vector3f f_r = inter.m->eval(ray.direction, nextDir, N);
// 				L_indir = castRay(nextRay, depth + 1) * f_r * dotProduct(nextDir, N) / pdf / RussianRoulette;
// 			}
// 		}
// 		//最后返回直接光照和间接光照
// 		return L_dir + L_indir;
// 	}
// 	//如果光线与场景无交点
// 	return Vector3f(0, 0, 0);
// }