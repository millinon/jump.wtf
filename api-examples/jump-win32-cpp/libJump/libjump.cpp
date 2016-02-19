#include "stdafx.h"

#include "curl/curl.h"

#define EXPORT __declspec(dllexport)
#include "jump.h"

namespace jump {

	void set_ssl_verification(CURL* curl) {

#ifdef JUMP_KILL_SLL
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#else
		TCHAR buffer[512];
		GetModuleFileName(NULL, buffer, 512);

#ifdef UNICODE
		std::wstring u_path = std::wstring(buffer);
		std::string path = std::string(u_path.begin(), u_path.end());
#else
		std::string path = std::string(buffer);
#endif
		path = path.substr(0, path.find_last_of("\\\\") + 1) + "curl-ca-bundle.crt";
		curl_easy_setopt(curl, CURLOPT_CAINFO, path.c_str());
#endif
	}

	void dprint(std::string s) {
#ifdef _DEBUG
		std::cerr << s << std::endl;
#endif
	}

	typedef struct {
		std::ifstream stream;
		size_t pos;
		size_t size;
	} file_chunk_info;

	typedef struct {
		std::string output_str;
	} response_info;

	size_t write_json_response(void* ptr, size_t size, size_t count, void* info);
	size_t read_file_chunks(void* ptr, size_t size, size_t count, void* info);

	EXPORT Json::Value get_upload_url(bool isPrivate)
	{
		dprint("get_upload_url called");

		Json::FastWriter writer;
		Json::Reader reader;

		Json::Value out;

		Json::Value request;
		request["action"] = "genUploadURL";
		request["private"] = isPrivate;

		response_info response = { "" };

		std::string request_str = writer.write(request);

		CURL *curl = curl_easy_init();
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

		curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

		set_ssl_verification(curl);

		// POST options
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_str.c_str());

		// response handler
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		dprint("calling curl_easy_perform");
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (res != CURLE_OK) {
			dprint("curl_easy_perform died");
			throw JumpException(curl_easy_strerror(res));
		}

		if (!reader.parse(response.output_str, out)) {
			dprint("JSON malformed");
			throw JumpException("Failed to parse returned JSON");
		}

		if (!out["success"].asBool()) {
			throw JumpException(out["message"].asString());
		}

		return out;
	}

	EXPORT void upload_file(std::string URL, std::string filename, std::string content_type)
	{
		dprint("upload_file called");

		file_chunk_info info;

		dprint("getting file's size");
		std::ifstream tmp_stream(filename.c_str(), std::ifstream::ate | std::ifstream::binary);

		if (!tmp_stream.is_open()) {
			dprint("couldn't get file's size");
			throw JumpException("Unable to open file");
		}

		info.pos = 0;
		info.size = static_cast<size_t>(tmp_stream.tellg());

		info.stream.open(filename.c_str(), std::ifstream::binary);

		std::string ct_header = "Content-Type: " + content_type;

		CURL *curl = curl_easy_init();
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, ct_header.c_str()));

		curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

		set_ssl_verification(curl);

		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_file_chunks);
		curl_easy_setopt(curl, CURLOPT_READDATA, &info);

		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)info.size);

		dprint("calling curl_easy_perform");
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (res != CURLE_OK) {
			dprint("curl_easy_perform died");
			throw JumpException(curl_easy_strerror(res));
		}
	}

	EXPORT Json::Value gen_jump_file_url(std::string tmp_key, std::string extension, std::string del_password, bool isPrivate, unsigned clicks)
	{
		dprint("get_jump-file_url called");

		Json::Value out;
		Json::Reader reader;
		Json::FastWriter writer;

		response_info response = { "" };

		Json::Value request;
		request["action"] = "genFileURL";
		request["tmp-key"] = tmp_key;
		request["extension"] = extension;
		request["private"] = isPrivate;
		request["password"] = del_password;

		if (isPrivate) {
			request["clicks"] = clicks;
		}

		std::string req_str = writer.write(request);// request.dump();

		CURL *curl = curl_easy_init();
		CURLcode res;

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

		curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

		set_ssl_verification(curl);

		// POST options
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

		// handle the response
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		dprint("calling curl_easy_perform");
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (res != CURLE_OK) {
			dprint("curl_easy_perform died");
			throw JumpException(curl_easy_strerror(res));
		}

		if (!reader.parse(response.output_str, out)) {
			dprint("JSON malformed");

			throw JumpException("Failed to parse returned JSON");
		}

		if (!out["success"].asBool()) {
			throw JumpException(out["message"].asString());
		}

		return out;
	}

	EXPORT Json::Value gen_jump_url(std::string URL, std::string del_password, bool isPrivate, unsigned clicks) {
		{
			dprint("get_jump_url called");

			Json::Value out;
			Json::Reader reader;
			Json::FastWriter writer;


			response_info response = { "" };

			Json::Value request;
			request["action"] = "genURL";
			request["input-url"] = URL;
			request["private"] = isPrivate;
			request["password"] = del_password;

			if (isPrivate) {
				request["clicks"] = clicks;
			}

			std::string req_str = writer.write(request);

			CURL *curl = curl_easy_init();
			CURLcode res;

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

			curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

			set_ssl_verification(curl);

			// POST options
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

			// handle the response
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			dprint("calling curl_easy_perform");
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (res != CURLE_OK) {
				throw JumpException(curl_easy_strerror(res));
			}

			if (!reader.parse(response.output_str, out)) {
				dprint("JSON malformed");
				throw JumpException("Failed to parse returned JSON");
			}

			if (!out["success"].asBool()) {
				throw JumpException(out["message"].asString());
			}

			return out;
		}
	}

	// read the response, add onto the output string
	size_t write_json_response(void* ptr, size_t size, size_t count, void* out) {
		dprint("write_json_response callback called");

		response_info* info = static_cast<response_info*>(out);

		std::string next = "";
		next.append(static_cast<char*>(ptr), 0, size * count);

		info->output_str += next;

		return size * count;
	}

	// consume as much of the file contents as curl will take
	size_t read_file_chunks(void* ptr, size_t size, size_t count, void* out) {

		dprint("read_file_chunks callback called");

		file_chunk_info* info = static_cast<file_chunk_info*>(out);

		size_t write_size = (std::min)(info->size - info->pos, size * count);

		if (write_size > 0) {
			info->stream.read(static_cast<char*>(ptr), write_size);
			info->pos += write_size;
		}

		return write_size;
	}
}