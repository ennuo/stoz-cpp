#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <unordered_map>

enum class EStoozeyVersion {
    INVALID,
    V1,
    V2
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
    FRAME_DURATION
};

class SStoozeySaveVector {
    public:
        SStoozeySaveVector(int capacity);
        void u8(uint8_t value);
        void uleb128(unsigned int value);
        void str(const std::string& value);

        void Compress();
        std::vector<uint8_t> GetData();

    private:
        unsigned int offset;
        std::vector<uint8_t> data;
};

class SStoozeyLoadVector {
    public:
        SStoozeyLoadVector(const char* filename);
        uint8_t u8();
        unsigned int uleb128();
        std::string str(unsigned int size);

        void Decompress(unsigned int uncompressed_size);
        void Forward(unsigned int offset) { this->offset += offset;  }
        uint8_t* GetPointer() { return this->data.data() + this->offset; }
    private:
        unsigned int offset;
        std::vector<uint8_t> data;
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

using SStoozeyRow = std::vector<SStoozeyPixel>;
using SStoozeyGrid = std::vector<SStoozeyRow>;

class SStoozeyFrame {
    public:
        SStoozeyFrame(SStoozeyHeader header);

        SStoozeyPixel GetPixel(int x, int y);
        void SetPixel(int x, int y, SStoozeyPixel pixel);
        void Pack(SStoozeySaveVector& stoz);

        int GetGridWidth() { return this->grid_width; }
        int GetGridHeight() { return this->grid_height;  }
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
        std::unordered_map<EStoozeyHeaderValue, int> headers;
        std::vector<SStoozeyFrame> frames;
};