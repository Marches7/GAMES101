// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    // 采用叉乘法来判断
    Vector3f p = {x, y, 0};
    Vector3f p0 = _v[0] - p;
    Vector3f p1 = _v[1] - p;
    Vector3f p2 = _v[2] - p;
    
    float c1 = p0.cross(p1).z(); // 取向量叉乘后的z轴坐标来判断正负
    float c2 = p1.cross(p2).z();
    float c3 = p2.cross(p0).z();

    if(c1>=0 && c2>=0 && c3>=0 || c1<0 && c2<0 && c3<0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

// Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    int xmin = std::min(std::min(v[0].x(), v[1].x()), v[2].x());
    int xmax = std::max(std::max(v[0].x(), v[1].x()), v[2].x());
    int ymin = std::min(std::min(v[0].y(), v[1].y()), v[2].y());
    int ymax = std::max(std::max(v[0].y(), v[1].y()), v[2].y());
    // iterate through the pixel and find if the current pixel is inside the triangle
    for(int x = xmin; x<=xmax; x++)
    {
        for(int y = ymin; y<=ymax; y++)
        {
            if(insideTriangle(x+0.5, y+0.5, t.v)) // 注意使用像素中心点来判断
            {
                // If so, use the following code to get the interpolated z value.
                float alpha, beta, gamma;
                std::tie(alpha, beta, gamma) = computeBarycentric2D(x+0.5, y+0.5, t.v); // 函数返回值是一个tuple元组, 可以使用std::tie接收
                float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                if(z_interpolated < depth_buf[get_index(x, y)])
                {
                    depth_buf[get_index(x, y)] = z_interpolated;
                    Vector3f point = {(float)x, (float)y, 0};
                    set_pixel(point, t.getColor());
                }
            }
        }
    }
}

// //****************提高部分************************//
// // Screen space rasterization
// void rst::rasterizer::rasterize_triangle(const Triangle& t) {
//     auto v = t.toVector4();
    
//     // TODO : Find out the bounding box of current triangle.
//     int xmin = std::min(std::min(v[0].x(), v[1].x()), v[2].x());
//     int xmax = std::max(std::max(v[0].x(), v[1].x()), v[2].x());
//     int ymin = std::min(std::min(v[0].y(), v[1].y()), v[2].y());
//     int ymax = std::max(std::max(v[0].y(), v[1].y()), v[2].y());
//     // iterate through the pixel and find if the current pixel is inside the triangle
//     for(int x = xmin; x<=xmax; x++)
//     {
//         for(int y = ymin; y<=ymax; y++)
//         {
//             // 提高部分 MSAA超采样(3x3)
//             std::vector<Vector3f> sample_list;
//             int inside_num=0;
//             for(int dx = -1; dx<=1; dx++)
//             {
//                 for(int dy = -1; dy<=1; dy++)
//                 {
//                     if(insideTriangle(x+dx+0.5, y+dy+0.5, t.v)) // 如果不在三角形内则默认加上背景色RGB{0, 0, 0}
//                     {
//                         sample_list.push_back(t.getColor());
//                         inside_num++;
//                     }    
//                 }
//             }
//             Vector3f sample_color={0, 0, 0};
//             for(auto & s : sample_list)
//             {
//                 sample_color += s;
//             }
//             sample_color /= 9;
//             // std::cout<<sample_color.x()<<std::endl;
//             // If so, use the following code to get the interpolated z value.
//             float alpha, beta, gamma;
//             std::tie(alpha, beta, gamma) = computeBarycentric2D(x+0.5, y+0.5, t.v); // 函数返回值是一个tuple元组, 可以使用std::tie接收
//             float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
//             float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
//             z_interpolated *= w_reciprocal;
//             // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
//             if(z_interpolated < depth_buf[get_index(x, y)])
//             {
//                 depth_buf[get_index(x, y)] = z_interpolated;
//             }
//             if(inside_num>0)
//             {
//                 // std::cout<<"inside_num = "<<inside_num<<std::endl;
//                 Vector3f point = {(float)x, (float)y, 0};
//                 set_pixel(point, sample_color);
//             }
//         }
//     }
    
// }

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on