#include <iostream>

#include "jump.hpp"
using namespace jump;

int main(int argc, char * argv[])
{

    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl;

    json response[2];

    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " path/to/file1 [path/to/file2] [...]" << std::endl;
        return 1;
    }

    for(int i = 1; i < argc; i++){

        std::string file = argv[i];
        std::string extension = file.substr(file.find_last_of("."));

        std::cout << "Trying to upload " << file << std::endl;

        if(extension == "") extension = ".txt"; 

        try {
            response[0] = jump::get_upload_url();
            std::cout << "    Got upload URL " << std::endl;
        } catch(jump::JumpException je){
            std::cerr << je.what() << std::endl;
            continue;
        }

        try {
            jump::upload_file(response[0]["URL"], file, response[0]["content-type"]);
            std::cout << "    Uploaded file" << std::endl;
        } catch(jump::JumpException je){
            std::cerr << je.what() << std::endl;
            continue;
        }

        try {
            response[1] = jump::get_jump_file_url(response[0]["tmp-key"], extension);
            std::cout << "    Got jump.wtf URL: " << response[1]["url"] << std::endl;
            if(response[1].find("cdn-url") != response[1].end())
                std::cout << "    CDN URL: " << response[1]["cdn-url"] << std::endl;
        } catch(JumpException je){
            std::cerr << je.what() << std::endl;
            continue;
        }
    }

    curl_global_cleanup();

    return 0;
}
