#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <stb_image.h>
#include <stoz.hpp>

SStoz::SStoz(SStoozeyHeader header) {
    this->header = header;
    this->frames = std::vector<SStoozeyFrame>();
    this->frames.reserve(header.frame_count);
    for (int i = 0; i < header.frame_count; ++i)
        this->frames.push_back(SStoozeyFrame(header));
}

int SStoz::GetWidth() { return this->header.width; }
int SStoz::GetHeight() { return this->header.height; }
int SStoz::GetFrameCount() { return this->header.frame_count; }
EStoozeyImageMode SStoz::GetImageMode() { return this->header.image_mode; }

bool SStoz::IsAnimated() { return this->header.frame_duration != 0; }

std::vector<uint8_t> SStoz::GetImageData(int frame_index) {
    std::vector<uint8_t> data;
    data.reserve(this->header.width * this->header.height * 4);
    SStoozeyFrame& frame = this->frames[frame_index];
    for (int x = 0; x < this->header.width; ++x) {
        for (int y = 0; y < this->header.height; ++y) {
            SStoozeyPixel pixel = frame.GetPixel(x, y);

            if (this->header.image_mode == EStoozeyImageMode::L) {
                data.push_back(pixel.r);
                continue;
            }

            if (this->header.image_mode == EStoozeyImageMode::RGB) {
                data.push_back(pixel.r);
                data.push_back(pixel.g);
                data.push_back(pixel.b);
                continue;
            }

            if (this->header.image_mode == EStoozeyImageMode::RGBA) {
                data.push_back(pixel.r);
                data.push_back(pixel.g);
                data.push_back(pixel.b);
                data.push_back(pixel.a);
                continue;
            }
        }
    }
    return data;
}

std::vector<uint8_t> SStoz::Pack() {
    std::vector<uint8_t> data;
    data.reserve((this->header.width + this->header.height) * 8);



    


    // TODO: Implement!

    return data;
}

SStoozeyFrame::SStoozeyFrame(SStoozeyHeader header) {
    this->image_mode = header.image_mode;
    this->image_width = header.width;
    this->image_height = header.height;
    this->pixel_size = header.pixel_size;

    this->grid_width = (int) std::ceil(header.width / header.pixel_size);
    this->grid_height = (int) std::ceil(header.height / header.pixel_size);

    this->grid = SStoozeyGrid();
    this->grid.reserve(this->grid_height);

    for (int i = 0; i < this->grid_height; ++i)
        this->grid.push_back(std::vector<SStoozeyPixel>(this->grid_height));
}

std::tuple<int, int> SStoozeyFrame::GetCellPosition(int x, int y) {
    return {
        std::max((int) 0, (int) std::min(this->grid_width - 1, (int) std::floor(x / this->pixel_size))),
        std::max((int) 0, (int) std::min(this->grid_height - 1, (int) std::floor(y / this->pixel_size)))
    };
}

SStoozeyPixel SStoozeyFrame::GetPixel(int x, int y) {
    auto grid_position = this->GetCellPosition(x, y);
    auto [grid_x, grid_y] = grid_position;
    return this->grid[grid_y][grid_x];
}

void SStoozeyFrame::SetPixel(int x, int y, SStoozeyPixel pixel) {
    auto grid_position = this->GetCellPosition(x, y);
    auto [grid_x, grid_y] = grid_position;
    this->grid[grid_y][grid_x] = pixel;
}

std::shared_ptr<SStoz> SStoz::FromImage(const char* filename) {
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);

    if (image == nullptr)
        throw std::runtime_error("Image failed to load!");

    SStoozeyHeader header {
        .image_mode = ((channels > 3) ? EStoozeyImageMode::RGBA : EStoozeyImageMode::RGB),
        .width = width,
        .height = height,
    };

    auto stoz = std::make_shared<SStoz>(header);
    SStoozeyFrame& frame = stoz->frames[0];
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            unsigned char* pixel_pos = (image + (((y * width) + x) * channels));

            SStoozeyPixel pixel = {
                .r = *((uint8_t*)(pixel_pos)),
                .g = *((uint8_t*)(pixel_pos + 1)),
                .b = *((uint8_t*)(pixel_pos + 2)),
                .a = (uint8_t) ((channels > 3) ? (*((uint8_t*)(pixel_pos + 3))) : (0xFF)),
            };

            frame.SetPixel(x, y, pixel);
        }
    }
    
    stbi_image_free(image);

    return stoz;
}

