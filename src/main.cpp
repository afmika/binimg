#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <algorithm>

int main() {
    int width = 400, height = 200;
    // uint8_t* ptr = (uint8_t*) malloc(width * height * 4);
    uint8_t* ptr = (uint8_t*) malloc(width * height * 4);
    for (int p = 0; p < width * height * 4; p += 4) {
        ptr[p + 0] = 100;
        ptr[p + 1] = 200;
        ptr[p + 2] = 100;
        ptr[p + 3] = rand() % 255;
    }
    stbi_write_png("test.png", width, height, 4, ptr, 0);
    free(ptr);
}