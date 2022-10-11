#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <tuple>

enum EStoozeyImageMode {
    L,
    RGB,
    RGBA,
    MAX
};

enum EStoozeyHeaderValue {
    VERSION,
    IMAGE_MODE,
    WIDTH,
    HEIGHT,
    PIXEL_SIZE,
    FRAME_COUNT,
    FRAME_DURATION,
    MAX
};

struct SStoozeyHeader {
    uint8_t version;
    uint8_t image_mode;
    uint32_t width;
    uint32_t height;
    uint32_t pixel_size;
    uint32_t frame_count;
    uint32_t frame_duration;
};

using SStoozeyPixel = uint32_t;
using SStoozeyGrid = std::vector<std::vector<SStoozeyPixel>>;

class SStoozeyFrame {
    public:
        SStoozeyFrame(uint32_t image_mode, uint32_t width, uint32_t height, uint32_t pixel_size = 1);

        SStoozeyPixel GetPixel(uint32_t x, uint32_t y);
        void SetPixel(uint32_t x, uint32_t y, SStoozeyPixel pixel);

        void Pack(std::vector<uint8_t> data);
    private:
        std::tuple<uint32_t, uint32_t> GetCellPosition(uint32_t x, uint32_t y);

        uint8_t image_mode;

        uint32_t image_width;
        uint32_t image_height;

        uint32_t grid_width;
        uint32_t grid_height;

        uint32_t pixel_size;

        SStoozeyGrid grid;
};

class SStoz {
    public:
        static SStoz Load(const char* filename);
        static SStoz FromImage(const char* filename);

        uint32_t GetWidth();
        uint32_t GetHeight();
        uint8_t GetImageMode();

        std::vector<uint8_t> GetImageData(uint32_t frame);
        std::vector<uint8_t> Pack();
    private:
        SStoozeyHeader header;
        std::vector<SStoozeyFrame> frames;
};