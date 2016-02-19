#pragma once

#ifndef EXPORT
#define EXPORT
#endif

#include "json/json.h"

namespace jump {

	// uncomment this to disable SSL certificate verification in libcurl
//#define JUMP_KILL_SSL

	EXPORT Json::Value get_upload_url(bool isPrivate = false);
// Fetch an S3 file upload URL that can be used with HTTP PUT
//
// isPrivate            --    whether or not the uploaded file will have a click limit


	EXPORT void upload_file(std::string URL, std::string filename, std::string content_type = "application/octet-stream");
// Use HTTP PUT to upload a file to the provided URL
//
// URL                  --    web URL to which the file should be uploaded
// content_type         --    MIME type to send with the Content-Type header, must match 'content-type' returned in get_upload_url


	EXPORT Json::Value gen_jump_file_url(std::string tmp_key, std::string extension = ".txt", std::string del_password = "", bool isPrivate = false, unsigned clicks = 1);
// After a file has been uploaded to S3, generate a jump.wtf link
//
// tmp_key            --    key returned by get_upload_url
// extension          --    desired extension of file URL
// del_password       --    optional password for link deletion
// isPrivate          --    whether or not the file will have a click limit
// clicks             --    how many times the file can be accessed before it is deleted, requires isPrivate to be true


	EXPORT Json::Value gen_jump_url(std::string URL, std::string del_password = "", bool isPrivate = false, unsigned clicks = 1);
// Generate a jump.wtf link for a web URL
//
// URL                --    web URL to link to: http://example.com/someURI
// del_password       --    optional password for link deletion
// isPrivate          --    whether or not the file will have a click limit
// clicks             --    how many times the link can be accessed before it is removed, requires isPrivate to be true

class JumpException : public std::exception {
public:
	explicit JumpException(char const* m) : message(m) {}
	explicit JumpException(const std::string& m) : message(m) {}
	const char* what() const throw () { return message.c_str(); }

protected:
	std::string message;
};
}