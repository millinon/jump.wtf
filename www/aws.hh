<?hh

//require("aws.phar"); // HHVM doesn't like this?
require ("aws-phar/aws-autoloader.php");

use Aws\Common\Aws;

use Aws\S3\Exception\S3Exception;
use Aws\S3\Exception\AccessDeniedException;
use Aws\Glacier\Exception\GlacierException;
use Aws\DynamoDb\Exception\DynamoDbException;
use Aws\CloudFront\Exception\CloudFrontException;

function mk_aws() {
  return Aws::factory('config/aws.json');
}
