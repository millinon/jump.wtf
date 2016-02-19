# jump-win32-cpp

This directory contains a Visual Studio 2015 solution that includes some examples of using the API with C++.


`libJump` produces a DLL that exports a few functions for things like uploading a file.


`jump-gyazo-win` is a program that, when opened, will take a screenshot of the area you select with your mouse, upload it to jump.wtf, open the generated link, and copy it to your clipboard.


`upload-jump-file` takes filenames as command-line arguments, and uploads each, outputting the generated URLs.


Building
------
Visual Studio 2015 should be able to build the solution without any configuration. The output directories will contain `libjump.dll`, `jump-gyazo-win.exe`, and `upload-jump-file.exe`, in addition to a bunch of DLL and PDB files. The DLLs are required to use the programs, but the PDBs are only used for debugging.


The output directory will also contain `curl-ca-bundle.crt`, which is used for SSL certificate verification. It must be in the same directory as the executables, unless you disable SSL certificate verification.


Dependencies
------

`jump-gyazo-win` uses a modified version of [Gyazowin](https://github.com/gyazo/Gyazowin).

All three projects use [jsoncpp](https://github.com/open-source-parsers/jsoncpp), amalgated and modified to export a few extra classes.

`libJump` uses [rmt\_curl](https://www.nuget.org/packages/rmt_curl/), which itself depends on rmt\_zlib, rmt\_libssh2, and rmt\_openssl. The NuGet package manager in Visual Studio will prompt you to install these packages upon opening the package manager for each project.

The file `ca-bundle.crt` is from [cURL](https://curl.haxx.se/ca/cacert.pem); you may want to replace it with your own certificate authority bundle. You may also choose to disable SSL certificate verification by modifying `jump.h`.
