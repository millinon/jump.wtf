<?hh

if (!include_once('config/aws_config.hh')) {
  require_once ('config/aws_config.hh.example');
}

use Aws\Credentials\CredentialProvider;

use Aws\CloudFront\CloudFrontClient;
use Aws\DynamoDb\DynamoDbClient;
use Aws\Glacier\GlacierClient;
use Aws\S3\S3Client;

use Aws\S3\Exception\S3Exception;
use Aws\S3\Exception\AccessDeniedException;
use Aws\Glacier\Exception\GlacierException;
use Aws\DynamoDb\Exception\DynamoDbException;
use Aws\CloudFront\Exception\CloudFrontException;

class awsHelper {

  // these are memoized so that the INI is only opened once each time this file is included,
  // instead of with every *client constructor
  private static $primary_provider;
  private static $URL_provider;

  private static $primary_cfg;
  private static $URL_cfg;

  public static function cf_client(): CloudFrontClient {
    return new CloudFrontClient(
      [
        'version' => 'latest',
        'credentials' => self::$primary_provider,
        'region' => self::$primary_cfg['region'],
      ],
    );
  }

  public static function dydb_client(): DynamoDbClient {
    return new DynamoDbClient(
      [
        'version' => 'latest',
        'credentials' => self::$primary_provider,
        'region' => self::$primary_cfg['region'],
      ],
    );
  }

  public static function gl_client(): GlacierClient {
    return new GlacierClient(
      [
        'version' => 'latest',
        'credentials' => self::$primary_provider,
        'region' => self::$primary_cfg['region'],
      ],
    );
  }

  public static function s3_client(): S3Client {
    return new S3Client(
      [
        'version' => 'latest',
        'credentials' => self::$primary_provider,
        'region' => self::$primary_cfg['region'],
      ],
    );
  }

  public static function s3_URL_client(): S3Client {
    return new S3Client(
      [
        'version' => 'latest',
        'credentials' => self::$URL_provider,
        'region' => self::$URL_cfg['region'],
      ],
    );
  }

  static function init(): void {
    self::$primary_cfg = aws_config::primary_cfg();
    self::$URL_cfg = aws_config::URL_cfg();

    self::$primary_provider = CredentialProvider::memoize(
      CredentialProvider::ini(
        self::$primary_cfg['profile'],
        aws_config::credentials_ini,
      ),
    );
    self::$URL_provider = CredentialProvider::memoize(
      CredentialProvider::ini(
        self::$URL_cfg['profile'],
        aws_config::credentials_ini,
      ),
    );
  }

}

//awsHelper::init();
