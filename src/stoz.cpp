#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "stb_image.h"
#include <stoz.hpp>
#include <fstream>
#include <functional>
#include <zlib.h>

SStoozeyLoadVector::SStoozeyLoadVector(const char* filename) {
    this->offset = 0;

    std::ifstream stream(filename, std::ios::in | std::ios::binary);
    if (!stream.good())
        throw std::runtime_error("File doesn't exist!");

    this->data = std::vector<uint8_t>((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

uint8_t SStoozeyLoadVector::u8() { return this->data[this->offset++]; }
unsigned int SStoozeyLoadVector::uleb128() {
    unsigned int result = 0;
    int index = 0;
    while (true) {
        uint8_t b = this->u8();
        result |= (b & 0x7f) << 7 * index;
        if ((b & 0x80) == 0) break;
        ++index;
    }
    return result;
}
std::string SStoozeyLoadVector::str(unsigned int size) {
    std::string s;
    s.assign((const char*) (this->data.data() + this->offset), size);
    this->offset += size;
    return s;
}

void SStoozeySaveVector::Compress() {
    unsigned long compressed_data_size = this->data.size();
    unsigned char* compressed_data = new unsigned char[compressed_data_size];
    int result = compress2(compressed_data, &compressed_data_size, this->data.data(), compressed_data_size, Z_BEST_COMPRESSION);

    this->data.clear();
    this->data.insert(this->data.end(), (uint8_t*)(compressed_data), (uint8_t*)(compressed_data + compressed_data_size));
    this->offset = 0;

    delete[] compressed_data;
}

void SStoozeyLoadVector::Decompress(unsigned int uncompressed_size) {
    unsigned long actual_size = uncompressed_size;
    unsigned char* data = new unsigned char[uncompressed_size];
    int result = uncompress(data, &actual_size, this->data.data() + this->offset, this->data.size() - this->offset);

    this->data.clear();
    this->data.insert(this->data.end(), (uint8_t*)(data), (uint8_t*)(data + actual_size));
    this->offset = 0;

    delete[] data;
}

SStoozeySaveVector::SStoozeySaveVector(int capacity) {
    this->data.reserve(capacity);
    this->offset = 0;
}

void SStoozeySaveVector::u8(uint8_t value) { this->data.push_back(value); }
void SStoozeySaveVector::uleb128(unsigned int value) {
    while (true) {
        uint8_t b = (value & 0x7f);
        value >>= 7;
        if (value > 0) b |= 128;
        this->u8(b);
        if (value == 0) break;
    }
}
void SStoozeySaveVector::str(const std::string& value) {
    for (auto& c : value)
        this->u8((uint8_t)c);
}

std::vector<uint8_t> SStoozeySaveVector::GetData() { return this->data;  }

SStoz::SStoz(SStoozeyHeader header) {
    this->headers[EStoozeyHeaderValue::VERSION] = (int) header.version;
    this->headers[EStoozeyHeaderValue::IMAGE_MODE] = (int) header.image_mode;
    this->headers[EStoozeyHeaderValue::WIDTH] = header.width;
    this->headers[EStoozeyHeaderValue::HEIGHT] = header.height;
    this->headers[EStoozeyHeaderValue::PIXEL_SIZE] = header.pixel_size;
    this->headers[EStoozeyHeaderValue::FRAME_COUNT] = header.frame_count;
    this->headers[EStoozeyHeaderValue::FRAME_DURATION] = header.frame_duration;

    this->frames = std::vector<SStoozeyFrame>();
    this->frames.reserve(header.frame_count);
    for (int i = 0; i < header.frame_count; ++i)
        this->frames.push_back(SStoozeyFrame(header));
}

int SStoz::GetWidth() { return this->headers[EStoozeyHeaderValue::WIDTH]; }
int SStoz::GetHeight() { return this->headers[EStoozeyHeaderValue::HEIGHT]; }
int SStoz::GetFrameCount() { 
    if (this->headers.contains(EStoozeyHeaderValue::FRAME_COUNT))
        return this->headers[EStoozeyHeaderValue::FRAME_COUNT];
    return 1;
}
EStoozeyImageMode SStoz::GetImageMode() { 
    return (EStoozeyImageMode)this->headers[EStoozeyHeaderValue::IMAGE_MODE];
}

bool SStoz::IsAnimated() { return this->GetFrameCount() > 1; }

std::vector<uint8_t> SStoz::GetImageData(int frame_index) {
    std::vector<uint8_t> data;

    int width = this->GetWidth(), height = this->GetHeight();
    EStoozeyImageMode image_mode = this->GetImageMode();

    data.reserve(width * height * 4);
    SStoozeyFrame& frame = this->frames[frame_index];
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            SStoozeyPixel pixel = frame.GetPixel(x, y);

            if (image_mode == EStoozeyImageMode::L) {
                data.push_back(pixel.r);
                continue;
            }

            if (image_mode == EStoozeyImageMode::RGB) {
                data.push_back(pixel.r);
                data.push_back(pixel.g);
                data.push_back(pixel.b);
                continue;
            }

            if (image_mode == EStoozeyImageMode::RGBA) {
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

void SStoozeyFrame::Pack(SStoozeySaveVector& stoz) {
    stoz.str("IMS");
    
    EStoozeyImageMode image_mode = this->image_mode;
    SStoozeyPixel pixel = this->GetPixel(0, 0);
    int count = 0;

    std::function<void()> write_block = [&] {
        stoz.uleb128(count);

        stoz.u8(pixel.r);
        stoz.u8(pixel.g);
        stoz.u8(pixel.b);
        stoz.u8(pixel.a);
    };

    if (this->image_mode == EStoozeyImageMode::L) {
        write_block = [&] {
            stoz.uleb128(count);
            stoz.u8(pixel.r);
        };
    }

    if (this->image_mode == EStoozeyImageMode::RGB) {
        write_block = [&] {
            stoz.uleb128(count);

            stoz.u8(pixel.r);
            stoz.u8(pixel.g);
            stoz.u8(pixel.b);
        };
    }

    for (int y = 0; y < this->grid_height; ++y) {
        for (int x = 0; x < this->grid_width; ++x) {
            SStoozeyPixel current_pixel = this->grid[y][x];
            if (*((uint32_t*)&current_pixel) != *((uint32_t*)&pixel)) {
                write_block();
                pixel = current_pixel;
                count = 0;
            }
            count++;
        }
    }

    if (count != 0) write_block();

    stoz.str("IME");
}

std::vector<uint8_t> SStoz::Pack() {
    SStoozeySaveVector stoz(0x100);
    SStoozeySaveVector image_vector((this->GetWidth() * this->GetHeight()) * 4);

    // Magic data
    stoz.str("STOZ");
    stoz.u8(0);

    // Headers
    stoz.str("HDS");
    for (auto header : this->headers) {
        stoz.uleb128((int) header.first);
        stoz.uleb128(header.second);
    }
    stoz.str("HDE");

    // Image data
    for (auto& frame : this->frames)
        frame.Pack(image_vector);

    // Zlib compress data

    image_vector.Compress();
    std::vector<uint8_t> compressed_data = image_vector.GetData();
    std::vector<uint8_t> stoz_data = stoz.GetData();
    stoz_data.insert(stoz_data.end(), compressed_data.begin(), compressed_data.end());

    return stoz_data;
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
        this->grid.push_back(std::vector<SStoozeyPixel>(this->grid_width));
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

std::shared_ptr<SStoz> SStoz::Load(const char* filename) {
    SStoozeyLoadVector load_vector(filename);

    if (load_vector.str(4) != "STOZ")
        throw std::runtime_error("File supplied isn't a STOZ file!");
    load_vector.u8();
    if (load_vector.str(3) != "HDS")
        throw std::runtime_error("Expected header start!");

    SStoozeyHeader header;
    // This surely isn't a problematic way to do this.
    while (strncmp((const char*)load_vector.GetPointer(), "HDE", 3) != 0) {
        unsigned int* ptr = ((unsigned int*)&header) + (load_vector.uleb128());
        *ptr = load_vector.uleb128();
    }
    load_vector.str(3);

    // Over-estimate of size since format doesn't store uncompressed size
    unsigned int uncompressed_size = (header.width * header.height) * 0x8;
    load_vector.Decompress(uncompressed_size);

    auto stoz = std::make_shared<SStoz>(header);
    for (int i = 0; i < header.frame_count; ++i) {
        SStoozeyFrame& frame = stoz->frames[i];
        if (load_vector.str(3) != "IMS")
            throw std::runtime_error("Expected frame start!");

        int grid_size = frame.GetGridHeight() * frame.GetGridHeight();
        int grid_index = 0;
        while (grid_index < grid_size) {
            int count = load_vector.uleb128();

            SStoozeyPixel pixel;
            if (header.image_mode == EStoozeyImageMode::RGBA) {
                pixel = *(SStoozeyPixel*)(load_vector.GetPointer());
                load_vector.Forward(4);
            }
            else if (header.image_mode == EStoozeyImageMode::RGB) {
                pixel = {
                    .r = load_vector.u8(),
                    .g = load_vector.u8(),
                    .b = load_vector.u8(),
                    .a = 0xFF
                };
            }
            else pixel = { .r = load_vector.u8() };

            for (int j = 0; j < count; ++j, ++grid_index) {
                int x = grid_index % frame.GetGridHeight();
                int y = (int) std::floor(grid_index / frame.GetGridWidth());

                frame.SetPixel(x, y, pixel);
            }
        }

        if (load_vector.str(3) != "IME")
            throw std::runtime_error("Expected frame end!");
    }

    return stoz;
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
