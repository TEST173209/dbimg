#include "img.hpp"

#include <filesystem>

void compress_img(cv::Mat& front, cv::Mat& back, float ratio)
{
	assert(ratio >= 0.0f && ratio <= 1.0f);
	int gap = 255 * ratio;
	front = front * (1 - ratio) + gap;
	back = back * ratio;
}


void decompress_img(cv::Mat& front, cv::Mat& back, float ratio)
{
	assert(ratio >= 0.0f && ratio <= 1.0f);
	int gap = 255 * ratio;
	front = (front - gap) / (1 - ratio);
	back = back / ratio;
}

cv::Mat make_phantom_tank(cv::Mat& front, cv::Mat& back, const bool colorful)
{
	cv::Mat phantom_tank(front.rows, front.cols, CV_8UC4);
	if (colorful)
	{
		constexpr float brightness_f = 12.0f;
		constexpr float brightness_b = 7.0f;
		for (int i = 0; i < front.rows; ++i) {
			for (int j = 0; j < front.cols; ++j) {
				// 获取前景背景像素（OpenCV为BGR格式）
				cv::Vec3b f_pixel = front.at<cv::Vec3b>(i, j);
				cv::Vec3b b_pixel = back.at<cv::Vec3b>(i, j);

				// 应用亮度调整（交换B和R通道）
				float R_f = f_pixel[2] * brightness_f / 10.0f;
				float G_f = f_pixel[1] * brightness_f / 10.0f;
				float B_f = f_pixel[0] * brightness_f / 10.0f;

				float R_b = b_pixel[2] * brightness_b / 10.0f;
				float G_b = b_pixel[1] * brightness_b / 10.0f;
				float B_b = b_pixel[0] * brightness_b / 10.0f;

				// 计算颜色差值
				float delta_r = R_b - R_f;
				float delta_g = G_b - G_f;
				float delta_b = B_b - B_f;

				// 计算alpha通道
				float coe_a = 8.0f + 255.0f / 256.0f + (delta_r - delta_b) / 256.0f;
				float coe_b = 4.0f * delta_r + 8.0f * delta_g + 6.0f * delta_b
					+ ((delta_r - delta_b) * (R_b + R_f)) / 256.0f
					+ (delta_r * delta_r - delta_b * delta_b) / 512.0f;

				float A_new = 255.0f + coe_b / (2.0f * coe_a);

				// 处理边界情况
				uchar A, R, G, B;
				if (A_new <= 0) {
					A = 0;
					R = G = B = 0;
				}
				else if (A_new >= 255) {
					A = 255;
					R = cv::saturate_cast<uchar>((255.0f * R_b) / A_new);
					G = cv::saturate_cast<uchar>((255.0f * G_b) / A_new);
					B = cv::saturate_cast<uchar>((255.0f * B_b) / A_new);
				}
				else {
					A = cv::saturate_cast<uchar>(A_new);
					R = cv::saturate_cast<uchar>((255.0f * R_b) / A_new);
					G = cv::saturate_cast<uchar>((255.0f * G_b) / A_new);
					B = cv::saturate_cast<uchar>((255.0f * B_b) / A_new);
				}

				// 设置输出像素（注意OpenCV的RGBA顺序）
				phantom_tank.at<cv::Vec4b>(i, j) = cv::Vec4b(B, G, R, A);
			}
		}
	}
	else
	{
		// set gray scale
		cv::cvtColor(front, front, cv::COLOR_BGR2GRAY);
		cv::cvtColor(back, back, cv::COLOR_BGR2GRAY);

		for (int i = 0; i < front.rows; i++)
		{
			for (int j = 0; j < front.cols; j++)
			{
				int fc = (front.at<uchar>(i, j) + front.at<uchar>(i, j) + front.at<uchar>(i, j)) / 3;
				int bc = (back.at<uchar>(i, j) + back.at<uchar>(i, j) + back.at<uchar>(i, j)) / 3;
				int oa = bc - fc + 255;
				int oc = oa == 0 ? 0 : bc * 255 / oa;
				phantom_tank.at<cv::Vec4b>(i, j) = cv::Vec4b(oc, oc, oc, oa);
			}
		}
	}
	
	return phantom_tank;
}

std::pair<cv::Mat, cv::Mat> unpack_phantom_tank(cv::Mat& phantom_tank, const bool colorful)
{
	cv::Mat front(phantom_tank.rows, phantom_tank.cols, CV_8UC3);
	cv::Mat back(phantom_tank.rows, phantom_tank.cols, CV_8UC3);
	
	if (colorful)
	{
		for (int i = 0; i < phantom_tank.rows; i++)
		{
			for (int j = 0; j < phantom_tank.cols; j++)
			{
				cv::Vec4b pixel = phantom_tank.at<cv::Vec4b>(i, j);
				float A = pixel[3];
				float R = pixel[2];
				float G = pixel[1];
				float B = pixel[0];
				float coe_a = 8.0f + 255.0f / 256.0f + (R - B) / 256.0f;
				float coe_b = 4.0f * R + 8.0f * G + 6.0f * B
					+ ((R - B) * (R + B)) / 256.0f
					+ (R * R - B * B) / 512.0f;
				float A_new = 255.0f + coe_b / (2.0f * coe_a);
				float R_new = A_new <= 0 ? 0 : (255.0f * R) / A_new;
				float G_new = A_new <= 0 ? 0 : (255.0f * G) / A_new;
				float B_new = A_new <= 0 ? 0 : (255.0f * B) / A_new;
				front.at<cv::Vec3b>(i, j) = cv::Vec3b(B_new, G_new, R_new);
				back.at<cv::Vec3b>(i, j) = cv::Vec3b(B_new, G_new, R_new);
			}
		}
	}
	else
	{
		for (int i = 0; i < phantom_tank.rows; i++)
		{
			for (int j = 0; j < phantom_tank.cols; j++)
			{
				if (phantom_tank.at<uchar>(i, j) == 0)
				{
					front.at<uchar>(i, j) = 0;
					back.at<uchar>(i, j) = 0;
				}
				else
				{
					front.at<uchar>(i, j) = 255;
					back.at<uchar>(i, j) = 255;
				}
			}
		}
	}

	return std::make_pair(front, back);
}

void save_image(const std::string& filename, const cv::Mat& image)
{
	std::filesystem::path path(filename);
	if (std::filesystem::exists(path))
	{
		char overwrite;
		std::cout << "File " << filename << " already exists. Overwrite? (y/n): ";
		std::cin >> overwrite;
		if (overwrite != 'y' && overwrite != 'Y')
		{
			return;
		}
	}
	cv::imwrite(filename, image);
}
