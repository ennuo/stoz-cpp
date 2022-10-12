#include <iostream>
#include <stoz.hpp>
#include <fstream>

int main() {
    auto stoz = SStoz::FromImage("C:/Users/Aidan/Desktop/FeY8fbIUUAABw9b_cropped.png");
    auto image_data = stoz->GetImageData(0);

    std::ofstream file("C:/Users/Aidan/Desktop/test.bin", std::ios::binary);
    file.write((const char*) image_data.data(), image_data.size());
    file.close();
}