#ifndef IMG_HPP
#define IMG_HPP

#include <opencv2/opencv.hpp>


void compress_img(cv::Mat& front, cv::Mat& back, float ratio = 0.5);
void decompress_img(cv::Mat& front, cv::Mat& back, float ratio = 0.5);

cv::Mat make_phantom_tank(cv::Mat& front, cv::Mat& back, const bool colorful = false);
std::pair<cv::Mat, cv::Mat> unpack_phantom_tank(cv::Mat& phantom_tank, const bool colorful = false);

void save_image(const std::string& filename, const cv::Mat& image);

#endif // !IMG_HPP
