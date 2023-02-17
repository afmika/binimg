#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <algorithm>
#include <iostream>
#include <fstream>

class Bytes {
public:
    uint8_t* content = nullptr;
    Bytes(uint8_t* bytes_seq) {
        content = bytes_seq;
    }
    virtual size_t size() const = 0; 
    virtual void loadFromFile(std::string filename) = 0;
    virtual void saveToFile(std::string filename) const = 0;
    virtual void printInfos() const = 0;
    inline bool isLoaded() const {
        return content != nullptr && size() != 0;
    }
};

class Image : public Bytes {
private:
    int n_comp = 4;
public:
    int width = 0;
    int height = 0;

    Image(): Bytes(nullptr) {};

    Image(uint8_t* content, int width, int height)
    : Bytes((uint8_t*) malloc(width * height * n_comp)) {
        n_comp = 4;
    }

    inline size_t size() const { 
        return static_cast<size_t>(width * height * n_comp);
    }

    inline size_t compSize() const {
        return static_cast<size_t>(n_comp);
    }
    
    void loadFromFile(std::string filename) {
        // https://github.com/nothings/stb/blob/master/stb_image.h#L139
        content = stbi_load(filename.c_str(), &width, &height, &n_comp, 0);
    }

    void saveToFile(std::string filename) const {
        stbi_write_png(filename.c_str(), width, height, n_comp, content, 0);
    }

    void printInfos() const {
        printf("\nwidth = %d, height = %d, comp_per_px %d, total %d\n", width, height, compSize(), size());
    }

    ~Image() {
        free(content);
    }
};

class File : public Bytes {
private:
    int n_bytes = 0;
    std::string filename = "";
public:
    File(): Bytes(nullptr)  {}
    
    size_t size() const {
        return n_bytes;
    } 

    void loadFromFile(std::string _filename) {
        filename = _filename;
        n_bytes = File::computeSize(filename);
        content = (uint8_t*) malloc(n_bytes);
        
        std::ifstream file(filename, std::ios::binary);
        int p = 0;
        for (; !file.eof(); content[p++] = file.get());
        assert((p - 1) == size());
        file.close();
    }

    void saveToFile(std::string filename) const {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        file.write((char *) content, size());
    }

    void printInfos() const {
        printf("\nsize = %d, name = %s\n", size(), filename.c_str());
    }

    static size_t computeSize(std::string filename) {
        std::ifstream in(filename, std::ios::ate | std::ios::binary);
        int size = in.tellg();
        in.close();
        return size;
    }

    ~File() {
        free(content);
    }
};

void check(const Image& img) {
    if (!img.isLoaded()) {
        std::cerr << "Image not loaded (invalid or corrupted?)";
        exit(1);
    }
}

// test filter
void BlackWhite(const Image& img) {
    check(img);
    for (size_t p = 0; p < img.size(); p += 4) {
        int total = img.content[p + 0] 
            + img.content[p + 1] 
            + img.content[p + 2] 
            + img.content[p + 3];
        img.content[p + 0] = total / 4;
        img.content[p + 1] = total / 4;
        img.content[p + 2] = total / 4;
        img.content[p + 3] = total / 4;
    }
}

void encode(const Image& img, const Bytes& file) {
    check(img);
    int n_bytes;
    for (size_t p = 0; p < img.size(); p += 4) {
        img.content[p + 0] = 100;
        img.content[p + 1] = 200;
        img.content[p + 2] = 100;
        img.content[p + 3] = rand() % 255;
    }
}

void decode(const Image& img) {
    check(img);
}

int main() {
    Image test;
    test.loadFromFile("b.jpg");
    if (test.isLoaded()) {
        test.printInfos();
        BlackWhite(test);
        test.saveToFile("sample.png");
    }

    File file;
    file.loadFromFile("b.jpg");
    if (file.isLoaded()) {
        file.printInfos();
        file.saveToFile("qq.jpg");
    }
}