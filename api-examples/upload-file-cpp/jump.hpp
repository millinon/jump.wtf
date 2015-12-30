#include <fstream> // for std::ifstream
#include <curl/curl.h>

#include "json.hpp"
using json = nlohmann::json;

#ifndef JUMP_WTF_H
#define JUMP_WTF_H

namespace jump {

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

    json get_upload_url(bool isPrivate=false);
    // Fetch an S3 file upload URL that can be used with HTTP PUT
    //
    // isPrivate            --    whether or not the uploaded file will have a click limit

    
    void upload_file(std::string URL, std::string filename, std::string content_type = "application/octet-stream");
    // Use HTTP PUT to upload a file to the provided URL
    //
    // URL                  --    web URL to which the file should be uploaded
    // content_type         --    MIME type to send with the Content-Type header, must match 'content-type' returned in get_upload_url


    json get_jump_file_url(std::string tmp_key, std::string extension = ".txt", std::string del_password = "", bool isPrivate = false, unsigned clicks = 1);
    // After a file has been uploaded to S3, generate a jump.wtf link
    //
    // tmp_key            --    key returned by get_upload_url
    // extension          --    desired extension of file URL
    // del_password       --    optional password for link deletion
    // isPrivate          --    whether or not the file will have a click limit
    // clicks             --    how many times the file can be accessed before it is deleted, requires isPrivate to be true

    
    json get_jump_url(std::string URL, std::string del_password = "", bool isPrivate = false, unsigned clicks = 1);
    // Generate a jump.wtf link for a web URL
    //
    // URL                --    web URL to link to: http://example.com/someURI
    // del_password       --    optional password for link deletion
    // isPrivate          --    whether or not the file will have a click limit
    // clicks             --    how many times the link can be accessed before it is removed, requires isPrivate to be true

    class JumpException : public std::exception {
        public:
            explicit JumpException(char const* m): message(m){}
            explicit JumpException(const std::string& m): message(m){}
            const char* what() const throw (){return message.c_str();}

        protected:
            std::string message;
    };

    json get_upload_url(bool isPrivate)
    {
        json out;

        json request;
        request["action"] = "genUploadURL";
        request["private"] = isPrivate;

        response_info response = {""};

        std::string request_str = request.dump();

        CURL *curl = curl_easy_init();
        CURLcode res;

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

        curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

        // POST options
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_str.c_str());

        // response handler
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        out = json::parse(response.output_str);

        if(res != CURLE_OK){
            throw JumpException(curl_easy_strerror(res));
        } else if( ! static_cast<bool>(out["success"]) ) {
            throw JumpException(out["message"].get<std::string>());
        }

        return out;
    }

    void upload_file(std::string URL, std::string filename, std::string content_type)
    {

        file_chunk_info info;

        std::ifstream tmp_stream(filename.c_str(), std::ifstream::ate | std::ifstream::binary);

        if(!tmp_stream.is_open()){
            throw JumpException("Unable to open file");
        }

        info.pos = 0;
        info.size = tmp_stream.tellg();
        info.stream.open(filename.c_str(), std::ifstream::binary);

        std::string ct_header = "Content-Type: " + content_type;

        CURL *curl = curl_easy_init();
        CURLcode res;

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, ct_header.c_str()));

        curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_file_chunks);
        curl_easy_setopt(curl, CURLOPT_READDATA, &info);

        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) info.size);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK){
            throw JumpException(curl_easy_strerror(res));
        }
    }

    json get_jump_file_url(std::string tmp_key, std::string extension, std::string del_password, bool isPrivate, unsigned clicks)
    {
        json out;

        response_info response = {""};

        json request;
        request["action"] = "genFileURL";
        request["tmp-key"] = tmp_key;
        request["extension"] = extension; 
        request["private"] = isPrivate;
        request["password"] = del_password;

        if(isPrivate){
            request["clicks"] = clicks;
        }

        std::string req_str = request.dump();

        CURL *curl = curl_easy_init();
        CURLcode res;

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

        curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

        // POST options
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

        // handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        out = json::parse(response.output_str); 

        if(res != CURLE_OK){
            throw JumpException(curl_easy_strerror(res));
        } else if( ! out["success"].get<bool>() ) {
            throw JumpException(out["message"].get<std::string>());
        }

        return out;
    }

    json get_jump_url(std::string URL, std::string del_password, bool isPrivate, unsigned clicks){
        {
            json out;

            response_info response = {""};

            json request;
            request["action"] = "genURL";
            request["input-url"] = URL;
            request["private"] = isPrivate;
            request["password"] = del_password;

            if(isPrivate){
                request["clicks"] = clicks;
            }

            std::string req_str = request.dump();

            CURL *curl = curl_easy_init();
            CURLcode res;

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json"));

            curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

            // POST options
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

            // handle the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            out = json::parse(response.output_str); 

            if(res != CURLE_OK){
                throw JumpException(curl_easy_strerror(res));
            } else if( ! out["success"].get<bool>() ) {
                throw JumpException(out["message"].get<std::string>());
            }
        }
    }

    // read the response, add onto the output string
    size_t write_json_response(void* ptr, size_t size, size_t count, void* out){

        response_info* info = static_cast<response_info*>(out); 

        info->output_str = info->output_str.append(static_cast<char*>(ptr), 0, size * count);

        return size * count;
    }

    // consume as much of the file contents as curl will take
    size_t read_file_chunks(void* ptr, size_t size, size_t count, void* out){

        file_chunk_info* info = static_cast<file_chunk_info*>(out);

        size_t write_size = std::min(info->size - info->pos, size * count);

        if(write_size > 0){
            info->stream.read(static_cast<char*>(ptr), write_size);
            info->pos += write_size;
        }

        return write_size;
    }
}
#endif
