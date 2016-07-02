# jump.wtf
A link shortening and file hosting service using AWS resources through PHP / Hack

---

## Introduction

Jump.wtf is a project I started at the end of 2014 to become more familiar with AWS resources. It allows users to submit URLS or files, and returns a shortened link.

All of the work is done in PHP, which is gradually being converted to Hack. I'm not very experienced with PHP or Hack, so don't yell at me if the code isn't pretty. I decided to try to get it working on HHVM to take advantage of HHVM's JIT compilation and type system.

You can see the project in action [here](https://jump.wtf); I'm using HHVM as a FastCGI server, passed through Apache, all on Ubuntu 14.04. I've included the Apache configuration files for jump.wtf in the conf directory. 

## Project structure

The `www/` directory of the project is a mirror of the files I have in the root directory served by Nginx and HHVM. All Hack files with the `.hh` extension are interpreted by HHVM, which communicates with Nginx through FastCGI.

The file `main.hh` is the web-facing script that calls different main functions. I was previously using a regex in Apache to decide which function to call, but it has worked better to simply examine the URI in Hack.

The main functions are divided into five different tasks:

* `include/api.hh`: serves the core API
* `index.hh`: Index page
* `go.hh`: Forwarding page
* `result.hh`: Results page
* `submit.hh`: Submission script

A typical interaction is a user goes to the site root, which is served by `index.hh`. If the user submits a link or file, their browser sends a POST request that gets handled by `submit.hh`. If all goes well, the user is redirected to `result.hh`, presenting the generated URL. If the generated key is in the form "someString", then when a visiting user accesses "jump.wtf/someString", `api.hh` handles the request by looking up the key `someString`. If `api.hh` finds a matching key, then the user is redirected to the corresponding URL. If not, then the user is redirected to the site index.

The rest of the resources needed for the site are arranged into a few directories:

* `www/blackhole/`: A trapdoor for bad bots
* `www/include/`: Hack files that are not web-facing
* `www/include/config/`: Configuration information needed at runtime
* `www/htdocs/`: static HTML resources (JS / CSS)
* `www/uploads/`: Temporary storage for uploaded files

`www/vendor` contains Facebook's xhp-lib, a required library.

The data on the submitted links is intended to all be stored remotely by AWS resources; the project should ultimately store no state on the local machine. This is violated currently by the `blackhole/` trapdoor, and the temporary storage of uploaded files in the `uploads/` directory.

There is additionally a `bin/` directory with a few scripts I've written:

* `deploy.sh`: prepares static resources
* `minify.js`: minifies JavaScript and CSS resources, with versioning
* `putFile.hh`: uploads a file, optionally with a custom key, I use this for `robots.txt`
* `pre-commit.sh`: does some basic testing, I use this as my pre-commit hook
* `validate-api.hh`: makes sure that the API as defined by `api.hh` and `api_ref.hh` is sane
* `make-code.hh`: generates a promo code with X custom URLs and Y large files

## To-Do

TODO, in no particular order:

* Check submissions against known malicious files and URLs
* Check submitters against known malicious IP addresses
* Rate limit submissions
* Rate limit (non-existing) key lookups
* Handle uploads to Glacier non-synchronously
* Improve Hack code style
* Improve file upload processing
* Add submission-time options to the user:
 * Expiration date

## Setup

If you'd like to set up a copy of jump.wtf for non-commercial use, I would be more than happy to help you out - contact me if you are struggling with the AWS account details or web server setup.

If you want to set up your own version of jump.wtf, you need to have an AWS account with access to S3 and DynamoDB at a bare minimum. I also include access to CloudFront, to provide a CDN for user-uploaded files, and Glacier, to automatically save backups of uploaded files. Credentials for this user need to be in `credentials.ini`, following the example provided by `credentials.ini.example`. I decided to use two separate IAM users, since one of them exposes its IAM access key when it generates links.

You need to have a web server set up so that it can statically serve the files in `www/htdocs`, and so that it can forward certain requests with the `*.hh` files by proxy to HHVM. Refer to the files in `www/include/config/`. I previously used an Apache configuration, but Nginx seems to be easier to configure and possibly better performing. The interface between HHVM and your web server is fairly flexible, as the web server only needs to support FCGI.

It is important to note that the only files that are served statically are in the `www/htdocs` directory - no other files should be accessible from the Internet. I was previously making this distinction by checking the prefix of the URI against `/htdocs/`, but I decided that it makes far more sense to distinguish static files by subdomain, and serving all other requests through `main.hh`.

## How it works

The core of jump.wtf is DynamoDB; a DynamoDB table stores keys and the links associated with them. Submitting a link or file consists of adding a row to the DynamoDB table, and redirecting to a link or file is the same as performing a lookup in the DynamoDB table.

Files submitted to jump.wtf are sent to one of two S3 buckets.

As a very simple example, the site's favicon isn't actually stored on my web server. The url https://jump.wtf/favicon.ico is associated with a row in the table with the primary key `favicon`. The matching URL in the table is `https://f.jump.wf/favicon.ico`, which is mapped by CloudFront to the file `favicon.ico` in a S3 bucket. 'favicon' wasn't an actually generated key; I manually uploaded the file to S3 and inserted a row into the table.

Requests are decoded into a uniform format (from JSON or multipart/formdata or HTTP GET), and routed into the API handler, which performs validation and routes the request to the appropriate method.

There are a few calls to Memcached for handling non-private links, in case a non-CDN-backed link gets posted in a public place, attracting a lot of traffic. Even though AWS should have fast networking internally, removing an unneccessary call to DynamoDb can hopefully improve performance.

### Link generation process

When a request is submitted to `submit.hh`, the file or URL goes through some basic checking. A key is then generated from the strings in `www/include.config/key_config.hh`. My version produces a key matching `[A-Za-z0-9]{4}`, but by modifying the configuration, you could make it use words instead of characters to produce a different scheme of keys (think of Gfycat filenames). The generated keys are then checked against DynamoDB to prevent duplicate key problems. Since the only URIs used in the process of generating links are `/s` and `/r`, almost any key scheme will work so long. If the dependence on `/s` and `/r` can be removed, then any key scheme at all will work.

Once a key is selected, the metadata is sent to DynamoDB with the new key as the primary index. If the submission is marked as having a click limit (e.g. a click limit of 1 click), then the submission is marked as being private. Any submissions that are private have a positive click limit stored in the DynamoDB table that gets decremented each time that it is accessed. If the user submitted a file, it is uploaded to the CDN-backed S3 bucket (or the non CDN-backed version if it is private).

Once the key has been selected and the data uploaded, the user is presented with a link. If they submitted a (non-private) file, they are also presented a CDN-backed link in the form of "https://f.jump.wtf/newKey.ext", to allow the user to take advantage of the CDN. If the user submitted a private file, they are presented a link in the form "https://jump.wtf/newKey.ext", which will be transformed into a signed AWS URL upon being accessed. If the user submitted a URL, they are presented a link in the form "https://jump.wtf/newKey".

### Link forwarding process

When a request is submitted to `go.hh`, the key is extracted as someURI in the URL: "https://jump.wtf/someURI" (where someURI matches the regular expression "^[A-Za-z0-9]{4}$").

They key is then queried as the primary index from the DynamoDB table. Since public file uploads are stored as a CDN-backed link, public files and links are handled in the same manner. If the link is to a private file, `go.hh` generates a signed URL to the file in the private S3 bucket, only valid for fifteen minutes. This is to prevent direct linking to the file in S3, which would make it impossible to track the number of clicks on a private link.

If the link is marked as private, then the remaining clicks for the link are checked and decremented if verified. If the number of remaining clicks is less than one, then the user is redirected to the site root.

If the decremented number of clicks is zero, then the url is marked as inactive in DynamoDB. If the inactive link is a file, then `go.hh` also deletes the file from S3 and attempts to invalidate it in CloudFront.

If all goes well, the user is redirected to the corresponding link with a "Location:" header.

### Link deletion process

When a link to be deleted is submitted to `submit.hh`, the link is first checked as existing, and the link's stored deletion password checked with the one stored in DynamoDB (salted and hashed).

If the request is verified, then the link is marked as inactive in DynamoDB. If the link is a file, the file is deleted from the corresponding S3 bucket, and invalidated in CloudFront.

It's worth noting that since the `jumpTo` method can be accessed through HTTP GET, URLs and files that have been deleted may still be accessible by someone who previously followed a link. However, jump.wtf is not intended to securely hide information - someone could always write down the long-form URL or save the 'private' file, so it can be accessed after deletion. Private files avoid this risk by requiring URLs to be signed, but there's nothing stopping someone from downloading a file before it has been deleted or before its click limit has been reached.

## API

Out of interest from a few people in adding an API for extensibility, I've started writing an API to allow applications to create jump.wtf links. Since I needed to make sure that the API follows a consistent input format, I decided to write the API specification in a way that can be used for input validation and for documentation. The documentation can be browsed in an (ugly) JSON format [here](https://jump.wtf/a), and HTTP POST requests are accepted at the same URL. This is certainly not a sane way of designing an API, but it is very fun to work with in terms of defining remote procedure calls. In a sense, the data being used to define the API methods is also being used as code defining how the API methods are implemented.

Here's an example of a command to generate a link:
```bash
$ curl -H 'Content-Type: application/json' -X POST -d '{"action":"genURL","input-url":"https://example.com"}' https://jump.wtf/a
{"success":true,"url":"https:\/\/jump.wtf\/a2uB"}
```

This example uses the command-line cURL program to call the `genURL` method, but the API works with any language that speaks HTTP(S). In the `api-examples/` directory, there is an example of a few Win32 C++ programs that use the jump.wtf API.

The API supports the following methods, indicated by an `action` field:

* `genURL`: generates a jump.wtf shortened URL
* `genUploadURL`: generates a jump.wtf URL that a file can be uploaded to, which can then be used in `genFileURL`
* `genFileURL`: generates a jump.wtf shortened URL from an already-uploaded file
* `jumpTo`: resolves a jump.wtf URL to its long form
* `delURL`: deletes a previously generated URL
* `getBalance`: looks up a promo code's balance

There is a help interface provided over HTTP GET [here](https://jump.wtf/a/action=help), which serves as documentation. The documentation is generated from the API itself, instead of a separate document. This means that details of each method, like the parameters required and the types of each parameter, are used for validating API calls, and are encoded as JSON to explain how to properly make API calls. Certain properties, like regular expressions used to verify parameters, and values representing acceptable limits (such as the length of an input URL) are stored as configuration files, meaning that updating the configuration immediately updates the code used for both generating the documentation and for processing input.

The HTML-exposed functionality has all been re-routed through the API, so that validation is uniform regardless of the input method. This is nice, because even if I modify the HTML form (possibly introducing bugs along the way), I can be certain that the API is still providing a consistent interface.

## Promo Codes

Promotional codes were added as a way to add some value to the site, by allowing users to include codes with submissions to gain extra features. Promo codes are stored in a separate DynamoDB table (again, a relational database would be equal or better for this use case), such that each code is associated with a number of custom URLs and large file 'credits'. A custom URL allows a user to request a certain key, using the key (instead of a randomly generated one) as the short-form link, or to upload a file that is a (configurable) number of times larger than the default maximum file size. As an example of usage, I was able to assign the URL https://jump.wtf/jump\_src to this repository by using the web interface, entering my generated promotional code in the `promo-code` field, and `jump\_src` in the `custom-url` field. Promo codes are generated by adding a row to the promo code table through any AWS interface.

## Dependencies

The HTML part of the site is built off of [Bootstrap](http://getbootstrap.com/).

The tabbed form was created with the help of [this](http://webdesignerhut.com/create-pure-css-tabs/).

The AWS stuff uses aws.phar provided by [Amazon](https://github.com/aws/aws-sdk-php/releases/latest). Currently, version 3 of the SDK works.

HTML output is generated by Facebook's [xhp-lib](https://github.com/facebook/xhp-lib).

The bad bot trapdoor is a modified version of the code by Jeff Starr available [here](http://perishablepress.com/blackhole-bad-bots/).

`result.hh` uses [clipboard.js](https://github.com/zenorocha/clipboard.js) to provide "Click to copy" functionality.

As an easter egg, clippy.js is included on the page from [here](https://www.smore.com/clippy-js).

I've included these resources (except aws.phar) with the rest of the project files; if there are issues with this inclusion, please just let me know and I'll work with you to correct the problem.
