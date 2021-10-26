#include "Image.hpp"

#include <dirent.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace image;

Color::Color(color_t r, color_t g, color_t b) {
    this->r = r;
    this->g = g;
    this->b = b;
}

void Color::print() const {
    std::cout << "[" << (unsigned int)this->r << ";" << (unsigned int)this->g
              << ";" << (unsigned int)this->b << "]";
}

Image::Image() {
    this->width = 0;
    this->height = 0;
}

Image::Image(unsigned width, unsigned height, std::vector<Color> colors) {
    assert(colors.size() == width * height);
    this->width = width;
    this->height = height;
    this->colors = colors;
}

void Image::print() const {
    std::cout << "[" << std::endl;
    for (unsigned l = 0; l < this->width; l++) {
        for (unsigned c = 0; c < this->width; c++) {
            this->colors[l * this->width + c].print();
            std::cout << " ; ";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

color_t Image::getMaxColor() const {
    color_t max = 0;
    for (Color each : this->colors) {
        if (each.r > max)
            max = each.r;
        else if (each.g > max)
            max = each.g;
        else if (each.b > max)
            max = each.b;
    }
    return max;
}

void _listDirectories(char *path) {
    DIR *dir;
    struct dirent *diread;
    std::vector<char *> files;

    if ((dir = opendir(path)) != nullptr) {
        while ((diread = readdir(dir)) != nullptr)
            files.push_back(diread->d_name);
        closedir(dir);
    } else {
        perror("opendir");
        return;
    }
    for (auto file : files) std::cout << file << "| ";
    std::cout << std::endl;
}

/**
 * @param filename: ONLY PPM FILES FOR NOW
 */
Image ImageLoader::load(std::string const filename) {
    std::ifstream fp;

    //_listDirectories(".");

    fp.open(filename);
    if (!fp.is_open()) {
        std::cerr << "<!> ImageLoader::load(" << filename
                  << ") -> cannot open file!" << std::endl;
        exit(-1);
    }
    fp.seekg(3);
    unsigned width, height, max_color = 0;
    fp >> width;
    assert(width > 0);
    fp.get();
    fp >> height;
    assert(height > 0);
    fp.get();
    fp >> max_color;
    assert(max_color == 255);
    fp.get();
    std::cout << "width: " << width << "; height: " << height
              << "; max_color: " << max_color << std::endl;
    std::vector<Color> colors;
    int r, g, b = 0;
    char current;
    for (unsigned i = 0; i < width * height; i++) {
        current = fp.get();
        // std::cout << "current = " << current << std::endl;
        r = current - '0';
        current = fp.get();
        // std::cout << "current = " << current << std::endl;
        g = current - '0';
        current = fp.get();
        // std::cout << "current = " << current << std::endl;
        b = current - '0';
        // std::cout << "r: " << r << "; g: " << g << "; b: " << b <<
        // std::endl;
        Color col((color_t)r, (color_t)g, (color_t)b);
        // col.print();
        colors.push_back(col);
    }
    std::cout << "lecture OK" << std::endl;
    Image img(width, height, std::vector<Color>(colors));
    img.print();
    fp.close();
    return img;
}

char _colorValueToAscii(color_t value) {
    return (char)static_cast<char>(((int)value) + '0');
}

/**
 * @param filename: ONLY PPM FILES FOR NOW
 */
void ImageLoader::save(std::string const filename, Image const &image) {
    std::ofstream fp;

    //_listDirectories(".");

    fp.open(filename, std::ios_base::out | std::ios_base::binary);
    if (!fp.is_open()) {
        std::cerr << "<!> ImageLoader::save(" << filename
                  << ") -> cannot open file!" << std::endl;
        exit(-1);
    }

    image.print();

    fp << "P6\n"
       << image.width << ' ' << image.height << '\n'
       << 255 << std::endl;
    for (Color current : image.colors)
        fp << _colorValueToAscii(current.r) << _colorValueToAscii(current.g)
           << _colorValueToAscii(current.b);
    // fp << current.r << current.g << current.b;

    std::cout << "ecriture OK" << std::endl;
    fp.close();
}
