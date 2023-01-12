#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    std::vector<cv::Point2f> cp;
    for(int i=0; i<=control_points.size()-1; i++)
    {
        cp.push_back(control_points[i]);
    }
    int l = cp.size();
    while(l!=1)
    {
        for(int i=0; i<l-1; i++)
        {
            cp[i] = cp[i] + t * (cp[i+1] - cp[i]);
        }
        l--;
    }
    return cp[0];
}

float p2p(cv::Point2f point1, cv::Point2f point2)
{
    return std::sqrt(std::pow(point1.x - point2.x, 2) + std::pow(point1.y - point2.y, 2));
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    std::vector<cv::Point2f> point_set;
    for(float t=0; t<=1; t+=0.001)
    {
        auto point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[4] = 255;
        // 找到与曲线上的点相邻的四个像素
        // cv::Point2f point00 = {std::floor(point.x), std::floor(point.y)};
        // cv::Point2f point01 = {std::floor(point.x), std::ceil(point.y)};
        // cv::Point2f point10 = {std::ceil(point.x), std::ceil(point.y)};
        // cv::Point2f point11 = {std::ceil(point.x), std::floor(point.y)};

        // // 找出四个像素到曲线点的距离的最小值， 并根据与曲线点的距离赋值颜色
        // float dmin = p2p(point, point00);
        // window.at<cv::Vec3b>(point00.y, point00.x)[4] = std::max(255*(1 - dmin), (float)window.at<cv::Vec3b>(point00.y, point00.x)[4]);
        // float d2 = p2p(point, point01);
        // if(d2 < dmin)
        // {
        //     window.at<cv::Vec3b>(point01.y, point01.x)[4] = std::max(255*(1 - d2), (float)window.at<cv::Vec3b>(point01.y, point01.x)[4]);
        //     dmin = d2;
        // }
        // float d3 = p2p(point, point10);
        // if(d3 < dmin)
        // {
        //     window.at<cv::Vec3b>(point10.y, point10.x)[4] = std::max(255*(1 - d3), (float)window.at<cv::Vec3b>(point10.y, point10.x)[4]);
        //     dmin = d3;
        // }
        // float d4 = p2p(point, point11);
        // if(d4 < dmin)
        // {
        //     window.at<cv::Vec3b>(point11.y, point11.x)[4] = std::max(255*(1 - d4), (float)window.at<cv::Vec3b>(point11.y, point11.x)[4]);
        // }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
