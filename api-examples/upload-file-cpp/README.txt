upload-to-jump

Uploads files specified on the command line to jump.wtf. Uploads are public.

I don't use C++ so there probably plenty of style violations, and I don't check many error conditions.

Compile with "make", clean with "make clean". Requires libcurl, and uses json.hpp from github.com/nlohmann/json/. It works on Linux, but needs testing on everything else.

For a version that compiles with Visual Studio 2015 Express, download and extract https://f.jump.wtf/KdSf.7z -- this contains 32-bit and 64-bit versions of libcurl and openssl. Please obey export laws pertaining to encryption provided by openssl.
