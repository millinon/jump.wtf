<?hh

require_once ('config/jump_config.hh'); // to enable validation
require_once ('config/key_config.hh');

class api_config {

  public static function api_methods(): array {

    $success_retval = [
      'description' => 'Whether or not the operation succeeded',
      'type' => 'boolean',
    ];

    $url_retval = [
      'success' => $success_retval,
      'url' => [
        'description' => 'Generated jump.wtf URL',
        'type' => 'string',
      ],
    ];

    $file_retval = array_merge(
      $url_retval,
      [
        'cdn-url' => [
          'description' => 'Direct link to CDN-backed file, if public',
          'type' => 'string',
        ],
      ],
    );

    $private_param = [
      'description' =>
        'Generated link should expire after a certain number of accesses',
      'type' => 'boolean',
      'default' => false,
      'requires-params' => [],
    ];

    $clicks_param = [
      'description' =>
        'Number of accesses after which the link should expire',
      'type' => 'integer',
      'default' => 1,
      'min-value' => 1,
      'max-value' => jump_config::MAX_CLICKS,
      'requires-params' => ['private'],
    ];

    return
      [
        'genUploadURL' =>
          [
            'description' =>
              'Generate an S3 upload URL valid for a HTTP POST file upload',
            'params' =>
              ['private' => $private_param],
            'constraints' =>
              [],
            'returns' =>
              [
                'success' => $success_retval,
                'URL' =>
                  [
                    'description' =>
                      'URL that a file can be uploaded to with HTTP POST',
                    'type' => 'string',
                  ],
                'tmp-key' =>
                  [
                    'description' =>
                      'Temporary key used for action genFileURL',
                    'type' => 'string',
                  ],
                'expires' =>
                  [
                    'description' =>
                      'ISO 8601 formatted time at which the URL will no longer be valid',
                    'type' => 'string',
                  ],
                'max-length' =>
                  [
                    'description' =>
                      'Maximum length, in bytes, of the file that can be uploaded',
                    'type' => 'integer',
                  ],
                'content-type' =>
                  [
                    'description' =>
                      'Content-Type header that must be used for the upload to S3',
                    'type' => 'string',
                  ],
              ],
            'examples' =>
              [
                ['action' => 'genUploadURL'],
                ['action' => 'genUploadURL', 'private' => true],
              ],
          ],
        'genFileURL' =>
          [
            'description' =>
              'Generate a jump.wtf URL for a file',
            'params' =>
              [
                'private' => $private_param,
                'clicks' => $clicks_param,
                'tmp-key' =>
                  [
                    'description' =>
                      'tmp-key field from the genUploadURL query used to upload the file',
                    'type' => 'string',
                    'regex' => '/^[0-9a-f]{14}\\.[0-9]{8}$/',
                  ],
                'file-data' =>
                  [
                    'description' =>
                      'Base64-encoded file data to store as a file and upload to S3',
                    'type' => 'string',
                    'min-length' => 1,
                    'max-length' => jump_config::MAX_LOCAL_FILE_SIZE, // the base64 encoded version might not be the exact same length as the decoded version, oh well
                  ],
                'local-file' => [
                  'description' => 'Local filename to upload to S3',
                  'note' => 'For integerernal use only',
                  'type' => 'string',
                  'regex' => '/^[0-9a-f]{14}\\.[0-9]{8}$/',
                  'max-length' => 23,
                ],
                'extension' =>
                  [
                    'description' =>
                      'Extension to be appended to the filename in S3',
                    'type' => 'string',
                    'default' => '.txt',
                    'min-length' => 1,
                    'max-length' => jump_config::MAX_EXT_LENGTH,
                    'regex' => '/^\.\\w+(\\.\\w+)*$/',
                  ],
                /*'save-backup' =>
                 [
                 'description' =>
                 'Whether or not to save a backup of the file in Glacier',
                 'type' => 'boolean',
                 'default' => true,
                 ],*/
                'password' => [
                  'description' => 'Optional password to delete the file',
                  'type' => 'string',
                  'default' => '',
                  'max-length' => jump_config::MAX_PASS_LEN,
                ],
                'content-type' =>
                  [
                    'description' =>
                      'Content type of file -- default is detected from the extension, and falls back to application/octet-stream',
                    'type' => 'string',
                    'default' => 'application/octet-stream',
                    'max-length' => 40,
                    'regex' => '/^[\\w.-]+\/[\\w.-]+$/',
                  ],
              ],
            'constraints' =>
              [['file-data', 'local-file', 'tmp-key']],
            'returns' =>
              $file_retval,
            'examples' =>
              [
                [
                  'action' => 'genFileURL',
                  'file-data' => 'aGVsbG8gd29ybGQK',
                ],
                [
                  'action' => 'genFileURL',
                  'tmp-key' => '567709c438b644.69604071',
                  'extension' => '.png',
                ],
              ],
          ],
        'genURL' =>
          [
            'description' =>
              'Generate a jump.wtf URL for a web link',
            'params' =>
              [
                'input-url' =>
                  [
                    'description' =>
                      'Link to generate a shortened jump.wtf URL for',
                    'type' => 'string',
                    'max-length' => jump_config::MAX_URL_LEN,
                  ],
                'private' => $private_param,
                'clicks' => $clicks_param,
                'password' => [
                  'description' => 'Optional password to delete the URL',
                  'type' => 'string',
                  'default' => '',
                  'max-length' => jump_config::MAX_PASS_LEN,
                ],
              ],
            'constraints' =>
              [['input-url']],
            'returns' =>
              $url_retval,
            'examples' =>
              [
                [
                  'action' => 'genURL',
                  'input-url' => 'https://example.com/',
                ],
                [
                  'action' => 'genURL',
                  'input-url' => 'https://jump.wtf',
                  'private' => true,
                  'clicks' => 2,
                ],
              ],
          ],
        'jumpTo' =>
          [
            'description' =>
              'Get the long-form URL to the link or file that a jump.wtf link pointegers to',
            'params' =>
              [
                'jump-key' =>
                  [
                    'description' =>
                      'Key to jump to: "https://jump.wtf/fooBar.baz" expects "fooBar"',
                    'type' => 'string',
                    'min-length' => key_config::MIN_LENGTH,
                    'max-length' => key_config::MAX_LENGTH,
                    'regex' => '/^'.key_config::regex.'$/',
                  ],
                'jump-url' =>
                  [
                    'description' => 'jump.wtf link to jump to',
                    'type' => 'string',
                    'max-length' =>
                      strlen('https://jump.wtf//.') +
                      key_config::length +
                      jump_config::MAX_EXT_LENGTH,
                    'min-length' =>
                      strlen('http://jump.wtf/') + key_config::MIN_LENGTH,
                  ],
              ],
            'constraints' =>
              [['jump-key', 'jump-url']],
            'returns' =>
              [
                'url' =>
                  [
                    'description' =>
                      'Long-form URL associated with the given key',
                    'type' => 'string',
                  ],
                'is-file' =>
                  [
                    'description' =>
                      'Whether or not the link goes to a jump.wtf hosted file',
                    'type' => 'boolean',
                  ],
                'expires' =>
                  [
                    'description' =>
                      'ISO 8601 Link expiration time for a private file',
                    'type' => 'string',
                  ],
              ],
            'examples' =>
              [
                [
                  'action' => 'jumpTo',
                  'jump-url' => 'https://jump.wtf/foo',
                ],
                ['action' => 'jumpTo', 'jump-key' => 'foo'],
              ],
          ],
        'delURL' =>
          [
            'description' =>
              'Delete a previously generated jump.wtf URL',
            'params' =>
              [
                'jump-key' =>
                  [
                    'description' =>
                      'Key to jump to: "https://jump.wtf/fooBar.baz" expects "fooBar"',
                    'type' => 'string',
                    'regex' => '/^'.key_config::regex.'$/',
                  ],
                'jump-url' => [
                  'description' => 'jump.wtf link to jump to',
                  'type' => 'string',
                ],
                'password' => [
                  'description' => 'Optional password to delete the URL',
                  'type' => 'string',
                  'min-length' => 1,
                  'max-length' => jump_config::MAX_PASS_LEN,
                ],
              ],
            'constraints' =>
              [['jump-key', 'jump-url'], ['password']],
            'returns' =>
              ['success' => $success_retval],
            'examples' =>
              [
                [
                  'action' => 'delURL',
                  'jump-key' => 'foo',
                  'password' => 'my_secret_password',
                ],
                [
                  'action' => 'delURL',
                  'jump-url' => 'https://jump.wtf/foo',
                  'password' => 'my_secret_password',
                ],
              ],
          ],
      ];
  }

  public static function api_help(): array {
    $methods = self::api_methods();

    return array_merge(
      $methods,
      [
        'help' => [
          'description' => 'Show help information for the API',
          'params' => [
            'topic' => [
              'description' => 'API action to show help text for',
              'type' => 'string',
              'default' => 'help',
            ],
          ],
          'topics' => [
            'all' => 'Show all help information',
            'genUploadURL' => 'Generate temporary file upload URL',
            'genFileURL' => 'Generate a jump.wtf link from a file',
            'genURL' => 'Generate a jump.wtf link from a web URL',
            'delURL' => 'Delete an existing jump.wtf link',
            'jumpTo' => 'Resolve a jump.wtf link',
            'constraints' => 'How to read the constraints',
            'help' => 'Show help information',
          ],
          'examples' => [
            ['action' => 'help'],
            ['action' => 'help', 'topic' => 'genURL'],
          ],
          'constraints' => [],
        ],
      ],
    );
  }

}
