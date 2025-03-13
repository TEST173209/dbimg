#include <iostream>

#include <opencv2/opencv.hpp>

#include "argparse/argparse.hpp"
#include "img.hpp"

#define PROGRAM "DBIMG"
#define VERSION "0.0.1"
#define AUTHOR "TEST"
#define DESCRIPTION "A tool for making phantom tank images"

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program(PROGRAM, VERSION);
	program.add_description(DESCRIPTION);

    // 图像参数
    program.add_argument("-f", "--front")
        .help("front image (show in white background)")
		.metavar("FILE")
        .required();
    program.add_argument("-b", "--back")
        .help("back image (show in black background)")
		.metavar("FILE")
        .required();

    // 输入输出参数
    auto& io_group = program.add_mutually_exclusive_group();
    io_group.add_argument("-o", "--output")
		.metavar("FILE")
        .help("output image");
    io_group.add_argument("-i", "--input")
		.metavar("FILE")
        .help("input image");

    // 开关参数
    program.add_argument("-u", "--unpack")
        .help("unpack image")
        .default_value(false)
        .implicit_value(true);
    program.add_argument("-c", "--compress")
        .help("compress image (set front and back base on ratio)")
        .default_value(false)
        .implicit_value(true);

    // 高级参数
    program.add_argument("-r", "--ratio")
        .help("compress ratio (0.0 - 1.0), default 0.5")
        .default_value(0.5)
        .implicit_value(true)
        .nargs(1)
        .scan<'g', float>();
    program.add_argument("-m", "--multicolor")
		.help("make colorful phantom tank")
		.default_value(false)
		.implicit_value(true);


    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
		std::cerr << "Error: " << err.what() << std::endl;
		std::cerr << "See " << PROGRAM << " --help for more information" << std::endl;
        return EXIT_FAILURE;
    }

    std::string front_img_path = program.get<std::string>("--front");
    std::string back_img_path = program.get<std::string>("--back");
    bool compress = program["--compress"] == true;
    bool unpack = program["--unpack"] == true;
    float ratio = program.get<float>("--ratio");
	bool multicolor = program["--multicolor"] == true;

    if (!unpack) {
        if (!program.is_used("--output")) {
            std::cerr << "output image is required" << std::endl;
            return EXIT_FAILURE;
        }

        std::string output_img_path = program.get<std::string>("--output");
        cv::Mat front = cv::imread(front_img_path);
        cv::Mat back = cv::imread(back_img_path);

        if (front.empty() || back.empty()) {
            std::cerr << "Failed to load images" << std::endl;
            return EXIT_FAILURE;
        }

        if (compress) {
            compress_img(front, back, ratio);
        }

        cv::Mat phantom_tank = make_phantom_tank(front, back, multicolor);
        save_image(output_img_path, phantom_tank);
    }
    else {
        if (!program.is_used("--input")) {
            std::cerr << "input image is required" << std::endl;
            return EXIT_FAILURE;
        }

        std::string input_img_path = program.get<std::string>("--input");
        cv::Mat phantom_tank = cv::imread(input_img_path);

        if (phantom_tank.empty()) {
            std::cerr << "Failed to load phantom tank image" << std::endl;
            return EXIT_FAILURE;
        }

        auto [front, back] = unpack_phantom_tank(phantom_tank, multicolor);
        if (compress) {
            decompress_img(front, back, ratio);
        }

        save_image(front_img_path, front);
        save_image(back_img_path, back);
    }

    return EXIT_SUCCESS;
}