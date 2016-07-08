/* I have this set to respond to files uploaded to the private or public buckets with the API.
 * If there's a way to restrict the file size in the generated URLs, that would be better...
 *
 * https://stackoverflow.com/questions/35143461/aws-php-sdk-limit-s3-file-upload-size-in-presigned-url
 *
 * The IAM role that this runs under needs permissions for GetObject and DeleteObject for items in tmp/ for each bucket
 */

var aws = require('aws-sdk');
var s3 = new aws.S3({ apiVersion: '2006-03-01' });

exports.handler = function(event, context) {
    var bucket = event.Records[0].s3.bucket.name;
    var key = decodeURIComponent(event.Records[0].s3.object.key.replace(/\+/g, ' '));
    s3.headObject({Bucket: bucket, Key: key}, function(err, data) {
        if (err) {
            console.log(err);
            var message = "Error getting object " + key + " from bucket " + bucket;
            console.log(message);
            context.fail(message);
        } else {
            if(data.ContentLength > 1 * 1024 * 1024){
                s3.deleteObject(params, function(err, data){
                    if(err){
                        var message = "Failed to delete object " + key;
                        console.log(message);
                        context.fail(message);
                    } else {
                        context.succeed("Deleted object " + key);
                    }
                });
            } else {
                context.succeed("Object " + key + " below file size limit");
            }
        }
    });
};
