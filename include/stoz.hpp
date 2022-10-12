#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <memory>

enum class EStoozeyVersion {
    INVALID,
    V1,
    V2,
};

enum class EStoozeyImageMode {
    L,
    RGB,
    RGBA,
};

enum class EStoozeyHeaderValue {
    VERSION,
    IMAGE_MODE,
    WIDTH,
    HEIGHT,
    PIXEL_SIZE,
    FRAME_COUNT,
    FRAME_DURATION,
};

struct SStoozeyHeader {
    EStoozeyVersion version = EStoozeyVersion::V2;
    EStoozeyImageMode image_mode = EStoozeyImageMode::RGB;
    int width = 0;
    int height = 0;
    int pixel_size = 1;
    int frame_count = 1;
    int frame_duration = 0;
};

struct SStoozeyPixel {
    uint8_t r = 0x0;
    uint8_t g = 0x0;
    uint8_t b = 0x0;
    uint8_t a = 0xff;
};

using SStoozeyGrid = std::vector<std::vector<SStoozeyPixel>>;

class SStoozeyFrame {
    public:
        SStoozeyFrame(SStoozeyHeader header);

        SStoozeyPixel GetPixel(int x, int y);
        void SetPixel(int x, int y, SStoozeyPixel pixel);

        void Pack(std::vector<int>& data);
    private:
        std::tuple<int, int> GetCellPosition(int x, int y);

        EStoozeyImageMode image_mode;

        int image_width;
        int image_height;

        int grid_width;
        int grid_height;

        int pixel_size;

        SStoozeyGrid grid;
};

class SStoz {
    public:
        SStoz(SStoozeyHeader header);

        static std::shared_ptr<SStoz> Load(const char* filename);
        static std::shared_ptr<SStoz> FromImage(const char* filename);

        int GetWidth();
        int GetHeight();
        int GetFrameCount();
        EStoozeyImageMode GetImageMode();

        bool IsAnimated();

        std::vector<uint8_t> GetImageData(int frame_index);
        std::vector<uint8_t> Pack();
    private:
        SStoozeyHeader header;
        std::vector<SStoozeyFrame> frames;
};