#!/usr/bin/env node
'use strict';

var compressor = require('node-minify'),
    fs = require('fs'),
    path = require('path'),
    ifname = "",
    vprint,
    argv,
    extensions = ["css", "js"];

function usage_exit() {
    console.log("Usage: minify [-v] file.(" + extensions.join("|") + ") [file2 file3...]");
    process.exit();
}

// check for optional verbose option
if (process.argv[2] === "-v") {
    vprint = console.log;
    argv = process.argv.slice(3);
} else {
    argv = process.argv.slice(2);
    vprint = function(){}
}

// at least one filename is required
if (argv.length <= 0) {
    usage_exit();
}

argv.forEach(function (ifname) {
    
    if (!ifname) {
        return;
    }

    var extension = path.extname(ifname).substring(1),
        basename = path.basename(ifname),
        base = path.basename(ifname, "." + extension),
        dir = path.dirname(ifname) + "/",
        regex = new RegExp("^" + base + "-\\d+\\.min\\." + extension + "$"),
        lfname = basename + ".latest",
        matches = 0,
        mtime,
        latest,
        last;

    if (extensions.indexOf(extension) <= -1) {
        console.log("Error: extension '." + extension + "' not supported");
        return;
    }

    fs.stat(ifname, function (err, stats) {

        if (err) {
            console.log(err.toString());
            return;
        }

        if (!stats) {
            console.log("Error: couldn't stat " + ifname);
            return;
        }

        mtime = stats.mtime;

        vprint(basename + ' mtime: ' + mtime);

        fs.readdir(dir, function (err, files) {

            if (err) {
                console.log(err.toString());
                return;
            }

            files = files.filter(function (fname) {
                return regex.test(fname);
            });


            matches = files.length;

            files.forEach(function (fname) {
                var time = fs.statSync(dir + fname).mtime;
                if (!latest || time > latest) {
                    latest = time;
                    last = fname;
                }
            });
            

            if (latest) {
                if  (files.length > 0) {
                    vprint(basename + " minified " + files.length + " times, last " + latest);
                } else {
                    vprint(basename + " never minified");
                }
            }

            if (latest && mtime <= latest) {
                console.log(last + " is up to date, skipping");
                return;
            }

            var ofname = base + "-" + matches + ".min." + extension;
            console.log("Minifying " + basename + " to " + ofname);

            compressor.minify({
                compressor: 'yui-' + extension,
                input: ifname,
                output: dir + ofname,
                callback: function (err) {
                    if (err) {
                        console.log(err.toString());
                    } else {

                        console.log("Wrote " + dir + ofname);

                        fs.chmod(dir + ofname, "0644", function (err) {
                            if (err) {
                                console.log("Couldn't chmod " + ofname);
                            }

                            fs.writeFile(dir + lfname, ofname, {mode: "0644"}, function (err) {
                                if (err) {
                                    console.log("Couldn't write " + lfname);
                                } else {
                                    console.log("Wrote " + lfname);
                                }
                            });
                        });
                    }
                }
            });
        });
    });
});
