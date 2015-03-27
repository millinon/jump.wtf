# jump.wtf
A link shortening and file hosting service using AWS resources through PHP / Hack

---

## Introduction

Jump.wtf is a project I started at the end of 2014 to become more familiar with AWS resources. It allows users to submit URLS or files, and returns a shortened link.

All of the work is done in PHP, which is gradually being converted to Hack. I'm not very experienced with PHP or Hack, so don't yell at me if the code isn't pretty. I decided to try to get it working on HHVM to take advantage of HHVM's JIT compilation and type system.

You can see the project in action [here](https://jump.wtf); I'm using HHVM as a FastCGI server, passed through Apache, all on Ubuntu 14.04. I've included the Apache configuration files for jump.wtf in the conf directory. I've also included a program I wrote to minify Javascript and CSS; it can be found in the bin directory.

## Project structure

The `jump/` directory of the project is a mirror of the files I have in the root directory served by Apache and HHVM.

There are four user-facing scripts that handle the site interactions. In order to simplify the process of resolving a URI to either a user-submitted link or a static HTML file, every local script or directory has a length of one character; that way, any URI longer than one character can be checked as a submitted URL. All `.hh` files have their extensions hidden by the ProxyPass directive.

These are the scripts:

* `i.hh`: Index page
* `g.hh`: Forwarding page
* `r.hh`: Results page
* `s.hh`: Submission script

A typical interaction is a user goes to the site root, which is served by `i.hh`. If the user submits a link or file, their browser sends a POST request that gets handled by `s.hh`. If all goes well, the user is redirected to `r.hh`, presenting the generated URL. If the generated key is in the form "someString", then when a visiting user accesses "jump.wtf/someString", `g.hh` handles the request. If `g.hh` finds a matching key, then the user is redirected to the corresponding URL.

The rest of the static resources needed for the site are arranged into a few directories:

* `b/`: A trapdoor for bad bots
* `h/`: HTML resources (JS / CSS)
* `p/`: PHP/HHVM resources
* `u/`: Temporary storage for uploaded files

The data on the submitted links is intended to all be stored remotely by AWS resources; the project should ultimately store no state on the local machine. This is violated currently by the `b/` trapdoor, and will also require implementation for adding rate limiting.

## To-Do

TODO, in no particular order:

* Check submissions against known malicious files and URLs
* Check submitters against known malicious IP addresses
* Rate limit submissions
* Rate limit (non-existing) key lookups
* Handle uploads to Glacier non-synchronously
* Migrate PHP files tso Hack
* Improve Hack code style
* Add error checking
* Harden HTTP server security
* Improve file upload processing
* Handle file size overflow handling
* Add submission-time options to the user:
 * Expiration date
 * Promotional codes (custom URL, extended expiration, multiple file selection, etc.)
* Check browser version before presenting "Click to copy" button

## Setup

If you want to set up your own version of jump.wtf, you need to have an AWS account with access to S3 and DynamoDB at a bare minimum. I also include access to CloudFront, to provide a CDN for user-uploaded files, and Glacier, to automatically save backups of uploaded files. Credentials for this user need to be in `p/aws.json`, following the example provided by `p/aws.json.example`.

You need to have a web server set up so that it can statically serve the files in `/h`, and so that it can forward certain requests with the `*.hh` files by proxy to HHVM. Refer to the files in `conf/`.

At some point in the future, this project may have some sort of script to automate the setup process.

## How it works

The core of jump.wtf is DynamoDB; a DynamoDB table stores keys and the links associated with them. Submitting a link or file consists of adding a row to the DynamoDB table, and redirecting to a link or file is the same as performing a lookup in the DynamoDB table.

Files submitted to jump.wtf are sent to one of two S3 buckets.

As a very simple example, the site's favicon isn't actually stored on my web server. The url https://jump.wtf/favicon.ico is associated with a row in the table with the primary key `favicon`. The matching URL in the table is `https://f.jump.wf/favicon.ico`, which is mapped by CloudFront to the file `favicon.ico` in a S3 bucket. 'favicon' wasn't an actually generated key; I manually uploaded the file to S3 and inserted a row into the table.

### Link generation process

When a request is submitted to `s.hh`, the file or URL goes through some basic checking. A key is then generated from the strings in `p/key_charset.hh`. My version produces a key matching `[A-Za-z0-9]{3,5}`, but by modifying `p/constants.hh` and `p/key_charset.hh`, you could make it use words instead of characters to produce a different scheme of keys (think of Gfycat filenames). The generated keys are then checked against the DynamoDB to prevent duplicate key problems.

Once a key is selected, the metadata is sent to DynamoDB with the new key as the primary index. If the submission is marked as having a click limit (e.g. a click limit of 1 click), then the submission is marked as being private. Any submissions that are private have a positive click limit stored in the DynamoDB table that gets decremented each time that it is accessed. If the user submitted a file, it is uploaded to the CDN-backed S3 bucket (or the non CDN-backed version if it is private).

A checksum is also submitted to DynamoDB as the secondary index, in case a checksum would be useful in the future to improve handling of duplicate submissions. This may not be the right approach or the right use of a secondary index, in which case please suggest a correction.

If the submission is a file upload, then a backup is also uploaded to Glacier for convenience, although this could be removed without affecting the experience of users.

Once the key has been selected and the data uploaded, the user is presented with a link. If they submitted a (non-private) file, they are presented a link in the form of "https://f.jump.wtf/newKey.ext", to allow the user to take advantage of the CDN. If the user submitted a private file, they are presented a link in the form "https://jump.wtf/newKey.ext". If the user submitted a URL, they are presented a link in the form "https://jump.wtf/newKey".

### Link forwarding process

When a request is submitted to `g.hh`, the key is extracted as someURI in the URL: "https://jump.wtf/someURI" (where someURI matches the regular expression "^[A-Za-z0-9]{2,5}$").

They key is then queried as the primary index from the DynamoDB table. Since public file uploads are stored as a CDN-backed link, public files and links are handled in the same manner. If the link is to a private file, `g.hh` generates a signed URL to the file in the private S3 bucket, only valid for fifteen minutes. This is to prevent direct linking to the file in S3, which would make it impossible to track the number of clicks on a private link.

If the link is marked as private, then the remaining clicks for the link are checked and decremented if verified. If the number of remaining clicks is less than one, then the user is redirected to the site root.

If the decremented number of clicks is zero, then the url is marked as inactive in DynamoDB. If the inactive link is a file, then `g.hh` also deletes the file from S3 and attempts to invalidate it in CloudFront.

If all goes well, the user is redirected to the corresponding link with a "Location:" header.

### Link deletion process

When a link to be deleted is submitted to `s.hh`, the link is first checked as existing, and the link's stored deletion password checked with the one stored in DynamoDB (salted and hashed).

If the request is verified, then the link is marked as inactive in DynamoDB. If the link is a file, the file is deleted from the corresponding S3 bucket, and invalidated in CloudFront.

## Dependencies

The HTML part of the site is built off of [Bootstrap](http://getbootstrap.com/).

The AWS stuff uses aws.phar provided by [Amazon](https://github.com/aws/aws-sdk-php/releases/latest). HHVM doesn't like `aws.phar` provided; I managed to get it working by extracting the phar into a directory `p/aws-phar`.

The bad bot trapdoor is a modified version of the code by Jeff Starr available [here](http://perishablepress.com/blackhole-bad-bots/).

`r.hh` uses [ZeroClipboard](https://github.com/zeroclipboard/zeroclipboard) to provide "Click to copy" functionality.

As an easter egg, clippy.js is included on the page from [here](https://www.smore.com/clippy-js).

I've included these resources with the rest of the project files; if there are issues with this inclusion, please just let me know and I'll work with you to correct the problem.
