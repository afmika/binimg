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

    void loadText(std::string text) {
        n_bytes = text.size();
        content = (uint8_t *) text.c_str();
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
    
    inline void allocate(size_t size) {
        n_bytes = size;
        content = (uint8_t*) malloc(size);
    }

    ~File() {
        free(content);
    }
};

void check(const Bytes& img) {
    if (!img.isLoaded()) {
        std::cerr << "File not loaded (invalid or corrupted?)";
        exit(1);
    }
}

void writeOffset(uint8_t* byte, uint8_t data, uint8_t offset) {
    uint8_t temp = *byte;
    *byte = (*byte & ~0b11) | (data >> offset);
}

void readOffset(uint8_t* byte, uint8_t* data, uint8_t offset) {
    *data |= (*byte & 0b11) << offset;
}

void encode(const Image& img, const File& file) {
    check(img);
    check(file);

    size_t target_size = file.size();
    // let's do 2 bits per color component
    // -4 for the header (4 * 8 = 32 bits)
    size_t header_offset = 4;
    size_t available_size = img.size() / img.compSize() - header_offset;

    if (target_size > available_size) {
        std::cerr << "\nImage cannot contain the file (not enough place)";
        std::cerr << "\n" << target_size << " > " << available_size;
        exit(1);
    }

    // header
    // 32 bits uint, 1 byte per pixel component
    size_t p = 0;
    img.content[p++] = target_size >> 24;
    img.content[p++] = target_size >> 16;
    img.content[p++] = target_size >> 8;
    img.content[p++] = target_size >> 0;
    // body
    for (size_t i = 0; i < target_size; i++) {
        writeOffset(img.content + p++, file.content[i], 6);
        writeOffset(img.content + p++, file.content[i], 4);
        writeOffset(img.content + p++, file.content[i], 2);
        writeOffset(img.content + p++, file.content[i], 0);
    }
}

void decode(const Image& img, File& file) {
    check(img);
    size_t target_size = 0;
    size_t p = 0;
    target_size |= (0xFFFFFFFF & img.content[p++]) << 24;
    target_size |= (0xFFFFFFFF & img.content[p++]) << 16;
    target_size |= (0xFFFFFFFF & img.content[p++]) << 8;
    target_size |= (0xFFFFFFFF & img.content[p++]) << 0;

    std::cout << "\nLoading " << target_size << " bytes";
    file.allocate(target_size);
    for (size_t i = 0; i < target_size; i++) {
        // reconstruct the byte
        file.content[i] = 0;
        readOffset(img.content + p++, file.content + i, 6);
        readOffset(img.content + p++, file.content + i, 4);
        readOffset(img.content + p++, file.content + i, 2);
        readOffset(img.content + p++, file.content + i, 0);
    }
}


void testEncodeDecode() {
    // encode
    Image img;
    File file;
    img.loadFromFile("b.jpg");
    file.loadFromFile("a.jpg");
    encode(img, file);
    img.saveToFile("result.png");

    // decode
    Image img_result;
    img_result.loadFromFile("result.png");
    File dest_file;
    decode(img_result, dest_file);
    assert(dest_file.isLoaded());
    dest_file.saveToFile("decoded.png");
}


void help() {
    std::cout << "binimg encode container input" << std::endl;
    std::cout << "binimg decode container" << std::endl;
    std::cout << "Examples: " << std::endl;
    std::cout << "binimg encode container.png rickroll.mp4" << std::endl;
    std::cout << "binimg decode encoded.png" << std::endl;
}

int main(int argc, char** argv) {
    testEncodeDecode();
    return 0;
}