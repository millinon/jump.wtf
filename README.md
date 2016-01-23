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

* `api.hh`: provide a web-facing API
* `index.hh`: Index page
* `go.hh`: Forwarding page
* `result.hh`: Results page
* `submit.hh`: Submission script

A typical interaction is a user goes to the site root, which is served by `index.hh`. If the user submits a link or file, their browser sends a POST request that gets handled by `submit.hh`. If all goes well, the user is redirected to `result.hh`, presenting the generated URL. If the generated key is in the form "someString", then when a visiting user accesses "jump.wtf/someString", `go.hh` handles the request by looking up the key `someString`. If `go.hh` finds a matching key, then the user is redirected to the corresponding URL. If not, then the user is redirected to the site index.

The rest of the resources needed for the site are arranged into a few directories:

* `www/blackhole/`: A trapdoor for bad bots
* `www/config/`: Configuration information needed at runtime
* `www/htdocs/`: static HTML resources (JS / CSS)
* `www/uploads/`: Temporary storage for uploaded files

`www/vendor` contains Facebook's xhp-lib, a required library.

The data on the submitted links is intended to all be stored remotely by AWS resources; the project should ultimately store no state on the local machine. This is violated currently by the `blackhole/` trapdoor, and the temporary storage of uploaded files in the `uploads/` directory.

There is additionally a `bin/` directory with a few scripts I've written:

* `deploy.sh`: prepares static resources
* `minify.js`: minifies JavaScript and CSS resources, with versioning
* `pre-commit.sh`: does some basic testing, I use this as my pre-commit hook
* `validate-api.hh`: makes sure that the API as defined by `api.hh` and `api_ref.hh` is sane

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
 * Promotional codes (custom URL, extended expiration, multiple file selection, etc.)

## Setup

If you want to set up your own version of jump.wtf, you need to have an AWS account with access to S3 and DynamoDB at a bare minimum. I also include access to CloudFront, to provide a CDN for user-uploaded files, and Glacier, to automatically save backups of uploaded files. Credentials for this user need to be in `www/config/aws.json`, following the example provided by `www/config/aws.json.example`.

You need to have a web server set up so that it can statically serve the files in `www/htdocs`, and so that it can forward certain requests with the `*.hh` files by proxy to HHVM. Refer to the files in `www/config/`. I previously used an Apache configuration, but Nginx seems to be easier to configure and possibly better performing.

It is important to note that the only files that are served statically are in the `www/htdocs` directory - no other files should be accessible from the Internet. I was previously making this distinction by checking the prefix of the URI against `/htdocs/`, but I decided that it makes far more sense to distinguish static files by subdomain, and serving all other requests through `main.hh`.

At some point in the future, this project may have some sort of script to automate the setup process.

## How it works

The core of jump.wtf is DynamoDB; a DynamoDB table stores keys and the links associated with them. Submitting a link or file consists of adding a row to the DynamoDB table, and redirecting to a link or file is the same as performing a lookup in the DynamoDB table.

Files submitted to jump.wtf are sent to one of two S3 buckets.

As a very simple example, the site's favicon isn't actually stored on my web server. The url https://jump.wtf/favicon.ico is associated with a row in the table with the primary key `favicon`. The matching URL in the table is `https://f.jump.wf/favicon.ico`, which is mapped by CloudFront to the file `favicon.ico` in a S3 bucket. 'favicon' wasn't an actually generated key; I manually uploaded the file to S3 and inserted a row into the table.

### Link generation process

When a request is submitted to `submit.hh`, the file or URL goes through some basic checking. A key is then generated from the strings in `www/config/key_config.hh`. My version produces a key matching `[A-Za-z0-9]{4}`, but by modifying the configuration, you could make it use words instead of characters to produce a different scheme of keys (think of Gfycat filenames). The generated keys are then checked against DynamoDB to prevent duplicate key problems. Since the only URIs used in the process of generating links are `/s` and `/r`, almost any key scheme will work so long. If the dependence on `/s` and `/r` can be removed, then any key scheme at all will work.

Once a key is selected, the metadata is sent to DynamoDB with the new key as the primary index. If the submission is marked as having a click limit (e.g. a click limit of 1 click), then the submission is marked as being private. Any submissions that are private have a positive click limit stored in the DynamoDB table that gets decremented each time that it is accessed. If the user submitted a file, it is uploaded to the CDN-backed S3 bucket (or the non CDN-backed version if it is private).

A checksum is also submitted to DynamoDB as the secondary index, in case a checksum would be useful in the future to improve handling of duplicate submissions. It doesn't look like this was a correct decision, so it's probably fine to ignore the checksum -- if I were to start over with a new table, I would exclude it.

If the submission is a file upload, then a backup is also uploaded to Glacier for convenience, although this could be removed without affecting the experience of users.

Once the key has been selected and the data uploaded, the user is presented with a link. If they submitted a (non-private) file, they are presented a link in the form of "https://f.jump.wtf/newKey.ext", to allow the user to take advantage of the CDN. If the user submitted a private file, they are presented a link in the form "https://jump.wtf/newKey.ext". If the user submitted a URL, they are presented a link in the form "https://jump.wtf/newKey".

### Link forwarding process

When a request is submitted to `go.hh`, the key is extracted as someURI in the URL: "https://jump.wtf/someURI" (where someURI matches the regular expression "^[A-Za-z0-9]{4}$").

They key is then queried as the primary index from the DynamoDB table. Since public file uploads are stored as a CDN-backed link, public files and links are handled in the same manner. If the link is to a private file, `go.hh` generates a signed URL to the file in the private S3 bucket, only valid for fifteen minutes. This is to prevent direct linking to the file in S3, which would make it impossible to track the number of clicks on a private link.

If the link is marked as private, then the remaining clicks for the link are checked and decremented if verified. If the number of remaining clicks is less than one, then the user is redirected to the site root.

If the decremented number of clicks is zero, then the url is marked as inactive in DynamoDB. If the inactive link is a file, then `go.hh` also deletes the file from S3 and attempts to invalidate it in CloudFront.

If all goes well, the user is redirected to the corresponding link with a "Location:" header.

### Link deletion process

When a link to be deleted is submitted to `s.hh`, the link is first checked as existing, and the link's stored deletion password checked with the one stored in DynamoDB (salted and hashed).

If the request is verified, then the link is marked as inactive in DynamoDB. If the link is a file, the file is deleted from the corresponding S3 bucket, and invalidated in CloudFront.

## API

Out of interest from a few people in adding an API for extensibility, I've started writing an API to allow applications to create jump.wtf links. Since I needed to make sure that the API follows a consistent input format, I decided to write the API specification in a way that can be used for input validation and for documentation. The documentation can be browsed in an (ugly) JSON format [here](https://jump.wtf/a), and HTTP POST requests are accepted at the same URL. Here's an example of a command to generate a link:
```bash
$ curl -H 'Content-Type: application/json' -X POST -d '{"action":"genURL","input-url":"https://example.com"}' https://jump.wtf/a
{"success":true,"url":"https:\/\/jump.wtf\/a2uB"}
```

It makes sense to use a single API to handle all requests, so once the remaining methods are implemented, I'll begin replacing the not very reliable code that handles the form requests with code that directly accesses the API. This way, input validation can occur uniformly, and errors can be identified and handled more easily.

## Dependencies

The HTML part of the site is built off of [Bootstrap](http://getbootstrap.com/).

The tabbed form was created with the help of [this](http://webdesignerhut.com/create-pure-css-tabs/).

The AWS stuff uses aws.phar provided by [Amazon](https://github.com/aws/aws-sdk-php/releases/latest). Currently, version 3 of the SDK works.

HTML output is generated by Facebook's [xhp-lib](https://github.com/facebook/xhp-lib).

The bad bot trapdoor is a modified version of the code by Jeff Starr available [here](http://perishablepress.com/blackhole-bad-bots/).

`result.hh` uses [clipboard.js](https://github.com/zenorocha/clipboard.js) to provide "Click to copy" functionality.

As an easter egg, clippy.js is included on the page from [here](https://www.smore.com/clippy-js).

I've included these resources (except aws.phar) with the rest of the project files; if there are issues with this inclusion, please just let me know and I'll work with you to correct the problem.
