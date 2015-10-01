<?hh

//require("p/aws.phar"); // HHVM doesn't like this?
require("p/aws-phar/aws-autoloader.php");

use Aws\Common\Aws;
//use Aws\Common\Enum;
//use Aws\Common\Enum\Region;

//use Aws\DynamoDb\DynamoDbClient;
//use Aws\DynamoDb\Enum\Type;

//use Aws\S3\Enum\Group;
//use Aws\S3\Model\AcpBuilder;
//use Aws\S3\S3Client;

use Aws\S3\Exception\S3Exception;
use Aws\Glacier\Exception\GlacierException;
use Aws\DynamoDb\Exception\DynamoDbException;
use Aws\CloudFront\Exception\CloudFrontException;

function mk_aws(){
	return Aws::factory('p/config/aws.json');
}
/*
$aws = Aws::factory('p/aws.json');

$dyclient = $aws->get('DynamoDb');
$s3client = $aws->get('S3');
$cfclient = $aws->get('CloudFront');
$glclient = $aws->get('Glacier');
*/
