#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

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
        if(!stbi_write_png(filename.c_str(), width, height, n_comp, content, 0)) {
            std::cerr << "Error: unable to write image file\n";
            exit(1);
        }
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
        std::fstream file(filename, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: unable to open file " << filename << "\n";
            exit(1);
        }
        file.read(reinterpret_cast<char*>(content), n_bytes);
        file.close();
    }

    void loadText(std::string text) {
        n_bytes = text.size();
        content = (uint8_t *) text.c_str();
    }

    void saveToFile(std::string filename) const {
        std::fstream file(filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: unable to open file " << filename << "\n";
            exit(1);
        }
        file.write(reinterpret_cast<const char*>(content), size());
        file.close();
    }

    void setName(std::string name) {
        filename = name;
    }

    std::string getNameOrDefault() const {
        if (filename.compare("") == 0)
            return "binimg-generated";
        return File::getNameFromPath(filename);
    }

    void printInfos() const {
        printf("\nsize = %d, name = %s\n", size(), filename.c_str());
    }

    static size_t computeSize(std::string filename) {
        std::fstream in(filename, std::ios::in | std::ios::ate | std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Unable to open file " << filename << "\n";
            exit(1);
        }
        size_t size = in.tellg();
        in.close();
        return size;
    }

    static inline std::string getNameFromPath(const std::string& path) {
        return path.substr(path.find_last_of("/\\") + 1);
    }

    static inline std::string removeExtension(std::string const& path) {
        return path.substr(0, getNameFromPath(path).find_last_of('.'));
    }
    
    void allocate(size_t size) {
        n_bytes = size;
        content = (uint8_t*) malloc(size);
    }

    ~File() {
        free(content);
    }
};