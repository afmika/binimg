#include <iostream>
#include "core.h"

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
            std::cout << "Injecting \"" << File::getNameFromPath(filename) << "\"\n";
            std::cout << "Into container " << File::getNameFromPath(img_container) << "\n";
            std::cout << "Please wait ...\n";
            
            File file;
            file.loadFromFile(filename);
            encode(img, file);

            std::string result_name = "encoded." + File::removeExtension(file.getNameOrDefault()) + ".png";
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