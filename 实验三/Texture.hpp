//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        // 限制u,v制为0～1之间，不然会出现指针越界
        if(u<0) u=0;
        if(v<0) v=0;
        if(u>1) u=1;
        if(v>1) v=1;

        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    //提高部分
    Eigen::Vector3f getColorBilinear(float u, float v)
    {
        // 限制u,v制为0～1之间，不然会出现指针越界
        if(u<0) u=0;
        if(v<0) v=0;
        if(u>1) u=1;
        if(v>1) v=1;

        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        //左下角
        auto u_img00 = std::floor(u_img);
        auto v_img00 = std::floor(v_img);
        //左上角
        auto u_img01 = std::floor(u_img);
        auto v_img01 = std::ceil(v_img);
        //右上角
        auto u_img11 = std::ceil(u_img);
        auto v_img11 = std::ceil(v_img);
        //右下角
        auto u_img10 = std::ceil(u_img);
        auto v_img10 = std::floor(v_img);

        //获取颜色
        auto color00 = image_data.at<cv::Vec3b>(v_img00, u_img00);
        auto color01 = image_data.at<cv::Vec3b>(v_img01, u_img01);
        auto color10 = image_data.at<cv::Vec3b>(v_img10, u_img10);
        auto color11 = image_data.at<cv::Vec3b>(v_img11, u_img11);

        auto color0 = (v_img - v_img00)*color01 + (v_img01 - v_img)*color00;
        auto color1 = (v_img - v_img00)*color10 + (v_img01 - v_img)*color11;
        auto color = (u_img - u_img00)*color1 + (u_img10 - u_img)*color0;
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
};
#endif //RASTERIZER_TEXTURE_H
