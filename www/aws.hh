<?hh

require_once("aws.phar"); 

require_once("config/aws_config.hh");

use Aws\Credentials\CredentialProvider;

use Aws\CloudFront\CloudFrontClient;
use Aws\DynamoDb\DynamoDbClient;
use Aws\Glacier\GlacierClient;
use Aws\S3\S3Client;

use Aws\S3\Exception\S3Exception;
use Aws\DynamoDb\Exception\DynamoDbException;
use Aws\CloudFront\Exception\CloudFrontException;

class awsHelper {

    // these are memoized so that the INI is only opened once each time this file is included,
    // instead of with every *client constructor
    private static $primary_provider;
    private static $URL_provider;

    public static function cf_client(): CloudFrontClient {
        return new CloudFrontClient(array_merge(aws_config::primary_cfg(), [
            'version' => 'latest',
            'credentials' => self::$primary_provider
        ]));
    }

    public static function dydb_client(): DynamoDbClient {
        return new DynamoDbClient(array_merge(aws_config::primary_cfg(), [
            'version' => 'latest',
            'credentials' => self::$primary_provider
        ]));
    }

    public static function gl_client(): GlacierClient {
        return new GlacierClient(array_merge(aws_config::primary_cfg(), [
            'version' => 'latest',
            'credentials' => self::$primary_provider
        ]));
    }

    public static function s3_client(): S3Client {
        return new S3Client(array_merge(aws_config::primary_cfg(), [
            'version' => 'latest',
            'credentials' => self::$primary_provider
        ]));
    }

    public static function s3_URL_client(): S3Client {
        return new S3Client(array_merge(aws_config::primary_cfg(), [
            'version' => 'latest',
            'credentials' => self::$URL_provider
        ]));
    }

    static function init(): void {
        self::$primary_provider = CredentialProvider::memoize(CredentialProvider::ini(aws_config::primary_cfg()['profile'], aws_config::credentials_ini));
        self::$URL_provider = CredentialProvider::memoize(CredentialProvider::ini(aws_config::URL_cfg()['profile'],aws_config::credentials_ini));
    }

}

awsHelper::init();
