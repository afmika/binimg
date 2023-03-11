#pragma once
#include "abstraction.h"

inline void check(const Bytes& data, const std::string& label) {
    if (!data.isLoaded()) {
        std::cerr << "Error: " << label << " not loaded";
        exit(1);
    }
}

inline void writeOffset(uint8_t* byte, const uint8_t data, const uint8_t offset) {
    *byte = (*byte & ~0b11) | ((data >> offset) & 0b11);
}

inline void readOffset(uint8_t* byte, uint8_t* data, const uint8_t offset) {
    *data |= (*byte & 0b11) << offset;
}

void writeText(uint8_t* byte, size_t& cursor, const std::string& text) {
    for (int i = 0; i < text.size(); i++) {
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            writeOffset(byte + cursor++, text[i], offset);
        }
    }
}

void readText(uint8_t* byte, size_t& cursor, std::string& container, const size_t size) {
    char text[size];
    for (int i = 0; i < size; i++) {
        text[i] = 0;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            readOffset(byte + cursor++, reinterpret_cast<uint8_t*>(text + i), offset);
        }
    }
    text[size] = '\0';
    container = std::string(text);
}

void writeInt32(uint8_t* byte, size_t& cursor, const size_t& value) {
    for (int k = 24; k >= 0; k -= 8) {
        uint8_t data = (value >> k) & 0xFF;
        int offset = 8;
        while(offset >= 0) {
            offset -= 2;
            writeOffset(byte + cursor++, data, offset);
        }
    }
}

void readInt32(uint8_t* byte, size_t& cursor, size_t& value) {
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
    check(img, "container image");
    check(file, "file to hide");

    std::string filename = file.getNameOrDefault();
    size_t target_size = file.size();
    size_t n_pixels = img.size() / img.compSize();
    // 1 pixel for the header (32 bits)
    std::string header = "bimg";
    // -9 = -1 (header 8 bits) - 8 (filename size 32 bits integer + filesize 32 bits integer) 
    size_t available_size = n_pixels - 9 - 2 * filename.size();

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

    std::cout << "Header written\n";
    // body
    // note: add redundancy to uniformize the noise
    while(true) {
        for (size_t i = 0; i < target_size; i++) {
            int offset = 8;
            while(offset >= 0) {
                assert(p < LONG_LONG_MAX);
                if (p >= img.size()) return; // stop redundancy
                offset -= 2;
                writeOffset(img.content + p++, file.content[i], offset);
            }
        }
    }
}

void decode(const Image& img, File& file) {
    check(img, "container image");
    std::string header, filename;
    size_t target_size = 0;
    size_t filename_size = 0;
    size_t p = 0;

    // header
    readText(img.content, p, header, 4);
    if (header.compare("bimg") != 0) {
        std::cerr << "Error: container does not contain a file";
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

    std::cout << "File \"" << filename << "\" detected\n";
    std::cout << "Loading " << target_size << " bytes from container\n";
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