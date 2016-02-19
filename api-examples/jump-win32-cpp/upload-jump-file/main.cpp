#include "stdafx.h"

#include <iostream>

#include "jump.h"

#include "json/json.h"

int main(int argc, char * argv[])
{
	Json::Value json;

	if (argc < 2) {
		std::cerr << "Usage: upload-jump-file.exe path/to/file1 [path/to/file2] [...]" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; i++) {

		std::string file = argv[i];
		std::string extension;

		if (file.find(".") == std::string::npos) {
			extension = ".txt";
		}
		else {
			extension = file.substr(file.find_last_of("."));
		}

		std::cout << "Trying to upload " << file << std::endl;

		try {
			json = jump::get_upload_url();
			std::cout << "    Got upload URL " << std::endl;
		}
		catch (jump::JumpException je) {
			std::cerr << je.what() << std::endl;
			continue;
		}

		try {
			jump::upload_file(json["URL"].asString(), file, json["content-type"].asString());
			std::cout << "    Uploaded file" << std::endl;
		}
		catch (jump::JumpException je) {
			std::cerr << je.what() << std::endl;
			continue;
		}

		try {
			json = jump::gen_jump_file_url(json["tmp-key"].asString(), extension);
			std::cout << "    Got jump.wtf URL: " << json["url"].asString() << std::endl;
			if (json.isMember("cdn-url"))
				std::cout << "    CDN URL: " << json["cdn-url"].asString() << std::endl;
		}
		catch (jump::JumpException je) {
			std::cerr << je.what() << std::endl;
			continue;
		}
	}

	return 0;
}