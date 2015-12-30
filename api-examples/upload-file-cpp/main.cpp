#include <iostream> // for cout and cerr
#include <fstream> // for ifstream
using namespace std;


#include <curl/curl.h>

#include "json.hpp"
using json = nlohmann::json;

typedef struct {
    ifstream stream;
    size_t pos;
    size_t size;
} file_chunk_info;

size_t write_json_response(void* ptr, size_t size, size_t count, void* out); /* out is a json, I just assume that it's only going to be called once */
size_t read_file_chunks(void* ptr, size_t size, size_t count, void* info); /* info is a file_chunk_info, that CURL will let me see every time this func is called */

int get_upload_url(json* out);
int upload_file(string URL, string filename, string content_type);
int get_jump_url(string tmp_key, string extension, json* out);

int main(int argc, char * argv[])
{

    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl;

    json response[2];

    if(argc < 2){
        cerr << "Usage: " << argv[0] << " path/to/file1 [path/to/file2] [...]" << endl;
        return 1;
    }

    for(int i = 1; i < argc; i++){

        string file = argv[i];
        string extension = file.substr(file.find_last_of("."));

        cout << "Trying to upload " << file << endl;

        if(extension == "") extension = ".txt"; 

        if(get_upload_url(&(response[0]))) continue;
        cout << "    Got upload URL" << endl;

        if(upload_file(response[0]["URL"], file, response[0]["content-type"])) continue;
        cout << "    Uploaded file" << endl;

        if(get_jump_url(response[0]["tmp-key"], extension, &response[1]));
        cout << "    Got jump.wtf URL: " << response[1]["url"] << endl;
        if(response[1].find("cdn-url") != response[1].end())
            cout << "    CDN URL: " << response[1]["cdn-url"] << endl;
    }
    
    curl_global_cleanup();
    
    return 0;
}

int get_upload_url(json* out)
{
    json request;
    request["action"] = "genUploadURL";

    string req_str = request.dump();

    CURL *curl = curl_easy_init();
    CURLcode res;

    struct curl_slist *chunk = curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

    // POST options
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

    // handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK){
        cerr << "genUploadURL Request failed: " << curl_easy_strerror(res) << endl;
        return 1;
    } else if(out->at("success") != true){
        cerr << "Request failed: " << endl << out->dump(4) << endl;
        return 1;
    }

    return 0;
}

int upload_file(string URL, string filename, string content_type)
{

    file_chunk_info info;

    ifstream tmp_stream(filename.c_str(), ifstream::ate | ifstream::binary);

    if(!tmp_stream.is_open()){
        cerr << "Unable to open " << filename << endl;
        return 1;
    }

    info.pos = 0;
    info.size = tmp_stream.tellg();
    info.stream.open(filename.c_str(), ifstream::binary);

    string ct_header = "Content-Type: " + content_type;

    CURL *curl = curl_easy_init();
    CURLcode res;
    
    struct curl_slist *chunk = curl_slist_append(NULL, ct_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_file_chunks);
    curl_easy_setopt(curl, CURLOPT_READDATA, &info);

    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t) info.size);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if(res != CURLE_OK){
        cerr << "Upload failed: " << curl_easy_strerror(res) << endl;
        return 1;
    }

    return 0;
}

int get_jump_url(string tmp_key, string extension, json* out)
{

    json request;
    request["action"] = "genFileURL";
    request["tmp-key"] = tmp_key;
    request["extension"] = extension;

    string req_str = request.dump();

    CURL *curl = curl_easy_init();
    CURLcode res;

    struct curl_slist *chunk = curl_slist_append(curl_slist_append(NULL, "Content-Type: application/json"), "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    curl_easy_setopt(curl, CURLOPT_URL, "https://jump.wtf/a");

    // POST options
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_str.c_str());

    // handle the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_json_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK){
        cerr << "genFileURL Request failed: " << curl_easy_strerror(res) << endl;
        return 1;
    } else if(out->at("success") != true){
        cerr << "Request failed: " << endl << out->dump(4) << endl;
        return 1;
    }

    return 0;
}

size_t write_json_response(void* ptr, size_t size, size_t count, void* out){
    string str = "";
    str.append((char*) ptr, 0, size * count);

    *reinterpret_cast<json*>(out) = json::parse(str);

    return size * count;
}

size_t read_file_chunks(void* ptr, size_t size, size_t count, void* info){

    file_chunk_info* curinfo = reinterpret_cast<file_chunk_info*>(info);

    size_t remaining = curinfo->size - curinfo->pos;

    size_t write_size = min(remaining, size * count);

    if(remaining > 0){
        curinfo->stream.read(reinterpret_cast<char*>(ptr), write_size);
        curinfo->pos += write_size;
    }
    
    return write_size;
}
