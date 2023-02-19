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
    size_t n_bytes = 0;
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
        if (!file.is_open()) {
            std::cerr << "Unable to open file " << filename << "\n";
            exit(1);
        }
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
        if (!file.is_open()) {
            std::cerr << "Unable to open file " << filename << "\n";
            exit(1);
        }
        file.write((char *) content, size());
    }

    void setName(std::string name) {
        filename = name;
    }

    std::string getNameOrDefault() const {
        if (filename.compare("") == 0)
            return "binimg-generated";
        return filename;
    }

    void printInfos() const {
        printf("\nsize = %d, name = %s\n", size(), filename.c_str());
    }

    static size_t computeSize(std::string filename) {
        std::ifstream in(filename, std::ios::ate | std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Unable to open file " << filename << "\n";
            exit(1);
        }
        size_t size = in.tellg();
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

inline void check(const Bytes& img) {
    if (!img.isLoaded()) {
        std::cerr << "File not loaded (invalid or corrupted?)";
        exit(1);
    }
}

inline void writeOffset(uint8_t* byte, const uint8_t data, const uint8_t offset) {
    *byte = (*byte & ~0b11) | ((data >> offset) & 0b11);
}

inline void readOffset(uint8_t* byte, uint8_t* data, const uint8_t offset) {
    *data |= (*byte & 0b11) << offset;
}

inline void writeText(uint8_t* byte, size_t& cursor, const std::string& text) {
    for (int i = 0; i < text.size(); i++) {
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            writeOffset(byte + cursor++, text[i], offset);
        }
    }
}

inline void readText(uint8_t* byte, size_t& cursor, std::string& container, const size_t size) {
    char text[size];
    for (int i = 0; i < size; i++) {
        text[i] = 0;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            readOffset(byte + cursor++, (uint8_t*)(text + i), offset);
        }
    }
    text[size] = '\0';
    container = std::string(text);
}

inline void writeInt32(uint8_t* byte, size_t& cursor, const size_t& value) {
    for (int k = 24; k >= 0; k -= 8) {
        uint8_t data = (value >> k) & 0xFF;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            writeOffset(byte + cursor++, data, offset);
        }
    }
}

inline void readInt32(uint8_t* byte, size_t& cursor, size_t& value) {
    value = 0;
    for (int k = 24; k >= 0; k -= 8) {
        uint8_t data = 0;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            readOffset(byte + cursor++, &data, offset);
        }
        value |= static_cast<uint32_t>(data) << k;
    }
}

void encode(const Image& img, const File& file) {
    check(img);
    check(file);

    std::string filename = file.getNameOrDefault();
    size_t target_size = file.size();
    size_t n_pixels = img.size() / img.compSize();
    // 1 pixel for the header (32 bits)
    std::string header = "bimg";
    // -9 = -1 (header 8 bits) - 8 (filename size 32 bits integer + filesize 32 bits integer) 
    size_t available_size = n_pixels - header.size() - filename.size() - 9;

    std::cout << "Target size " << target_size << "\n";
    std::cout << "Available size " << available_size << "\n";

    if (target_size > available_size) {
        std::cerr << "\nImage cannot contain target file (not enough space)";
        std::cerr << "\n" << target_size << " > " << available_size;
        exit(1);
    }

    // header
    size_t p = 0;
    assert(header.size() == 4);
    // 2 pixels for the 'bimg' header
    writeText(img.content, p, header);
    // 2nd pixel (4 bytes) : filename size
    writeInt32(img.content, p, filename.size());
    // next pixels are going to be the filename
    writeText(img.content, p, filename);
    // size of the file
    writeInt32(img.content, p, target_size);

    // body
    // note: add redundancy to uniformize the noise
    while(true) {
        for (size_t i = 0; i < target_size; i++) {
            int offset = 8;
            while(offset >= 0) {
                if (p >= img.size()) return; // stop redundancy
                offset -= 2;
                writeOffset(img.content + p++, file.content[i], offset);
            }
        }
    }
}

void decode(const Image& img, File& file) {
    check(img);
    std::string header, filename;
    size_t target_size = 0;
    size_t filename_size = 0;
    size_t p = 0;

    // header
    readText(img.content, p, header, 4);
    if (header.compare("bimg") != 0) {
        std::cerr << "Error: container does not contain a file";
        std::cerr << "Header" << header;
        exit(1);
    }

    // filename size
    readInt32(img.content, p, filename_size);
    assert(filename_size > 0);
    // extract filename
    readText(img.content, p, filename, filename_size);
    assert(filename.size() == filename_size);
    // extract binary file size
    readInt32(img.content, p, target_size);

    std::cout << "\nLoading " << target_size << " bytes from container\n";
    file.allocate(target_size);
    for (size_t i = 0; i < target_size; i++) {
        file.content[i] = 0;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            readOffset(img.content + p++, file.content + i, offset);
        }
    }
    file.setName("decoded." + filename);
}

void help() {
    std::cout << "bingimg v1 - afmika\n";
    std::cout << "# Usage\n";
    std::cout << "  binimg encode container input\n";
    std::cout << "  binimg decode container output\n";
    std::cout << "# Examples \n";
    std::cout << "  binimg decode examples/decode-me.png\n";
    std::cout << "  binimg encode container.png rickroll.mp4\n";
    std::cout << "  binimg decode encoded.png\n";
}

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        help();
        return 0;
    }

    if (argc == 3 || argc == 4) {
        int pos = 1;
        std::string command(argv[pos++]);
        std::string img_container(argv[pos++]);
        Image img;
        img.loadFromFile(img_container);
        if (command.compare("encode") == 0 && argc == 4) {
            std::string filename(argv[pos++]);
            std::cout << "Injecting " << filename << "\n";
            std::cout << "Into container " << img_container << "\n";
            std::cout << "Please wait ...\n";
            
            File file;
            file.loadFromFile(filename);
            encode(img, file);

            std::string result_name = "encoded.png";
            img.saveToFile(result_name);
            std::cout << "Saved as " << result_name;
            return 0;
        }
        
        if (command.compare("decode") == 0) {
            std::cout << "Decoding file stored inside " << img_container << "\n";
            std::cout << "Please wait ...\n";

            File dest_file;
            decode(img, dest_file);
            std::string filename = dest_file.getNameOrDefault();
            if (argc == 4)
                filename = std::string(argv[pos++]);

            dest_file.saveToFile(filename);
            std::cout << "Saved as " << filename;
            return 0;
        }
    }

    std::cout << "Invalid command\n";
    help();
    return 0;
}