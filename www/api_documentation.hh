<?hh

class api_documentation {

  public static function api_doc(): array {

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
      'optional' => true,
      'default' => false,
      'requires-params' => [],
    ];

    $clicks_param = [
      'description' => 'Number of clicks after which the link should expire',
      'type' => 'integer',
      'optional' => true,
      'default' => 1,
      'min-value' => '1',
      'max-value' => jump_config::MAX_CLICKS,
      'requires-params' => ['private'],
    ];

    $password_param = [
      'description' => 'Password to delete the URL',
      'type' => 'string',
      'optional' => 'true',
      'default' => '',
      'max-length' => jump_config::MAX_PASS_LEN,
      'required-params' => [],
    ];
    return
      [
        'genUploadURL' =>
          [
            'description' =>
              'Generate an S3 upload URL valid for a HTTP POST file upload',
            'params' =>
              [
                'private' => $private_param,
                'content-type' =>
                  [
                    'description' =>
                      'Content type that should be associated with the uploaded file',
                    'type' => 'string',
                    'optional' => true,
                    'default' => 'application/octet-stream',
                    'requires-params' => [],
                  ],
              ],
            'required-params' =>
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
                    'optional' => false,
                    'required-params' => [],
                  ],
                'filedata' =>
                  [
                    'description' =>
                      'Base64-encoded file data to store as a file and upload to S3',
                    'type' => 'string',
                    'optional' => false,
                    'max-length' => jump_config::MAX_FILE_SIZE / 4, // local file transfer takes more time, so limit the size
                    'required-params' => [],
                  ],
                'filepath' =>
                  [
                    'description' =>
                      'Local filesystem path of file to upload to S3',
                    'note' => 'For internal use only',
                    'type' => 'string',
                    'optional' => false,
                    'required-params' => [],
                    'max-length' => 128,
                  ],
                'save-backup' =>
                  [
                    'description' =>
                      'Whether or not to save a backup of the file in Glacier',
                    'type' => 'boolean',
                    'optional' => true,
                    'default' => true,
                  ],
                'password' => $password_param,
              ],
            'required-params' =>
              [['filedata', 'filepath', 'tmp-key']],
            'returns' =>
              $file_retval,
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
                    'requires-params' => [],
                  ],
                'private' => $private_param,
                'clicks' => $clicks_param,
                'password' => $password_param,
              ],
            'required-params' =>
              [['input-url']],
            'returns' =>
              $url_retval,
          ],
        'jumpTo' =>
          [
            'description' =>
              'Get the long-form URL to the link or file that a jump.wtf link points to',
            'params' =>
              [
                'jump-key' =>
                  [
                    'description' =>
                      'Key to jump to: "https://jump.wtf/fooBar.baz" expects "fooBar"',
                    'type' => 'string',
                    'optional' => false,
                    'requires-params' => ['jump-url'],
                  ],
                'jump-url' => [
                  'description' => 'jump.wtf link to jump to',
                  'type' => 'string',
                  'optional' => false,
                  'requires-params' => ['jump-key'],
                ],
              ],
            'required-params' =>
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
                    'optional' => false,
                    'requires-params' => [],
                  ],
                'jump-url' => [
                  'description' => 'jump.wtf link to jump to',
                  'type' => 'string',
                  'optional' => false,
                  'requires-params' => [],
                ],
                'password' => [
                  'description' => 'Password to delete the jump.wtf URL',
                  'type' => 'string',
                  'optional' => false,
                  'requires-params' => [],
                ],
              ],
            'required-params' =>
              [['jump-key', 'jump-url']],
            'returns' =>
              ['success' => $success_retval],
          ],
        'help' =>
          [
            'description' =>
              'Show help information for the API',
            'params' =>
              [
                'topic' => [
                  'description' => 'API action to show help text for',
                  'type' => 'string',
                  'optional' => true,
                  'default' => 'help',
                  'examples' => [
                    ['action' => 'help'],
                    ['action' => 'help', 'topic' => 'help'],
                  ],
                  'requires-params' => [],
                ],
              ],
            'topics' =>
              [
                'all' => 'Show all help information',
                'genUploadURL' => 'Generate temporary file upload URL',
                'genFileURL' => 'Generate a jump.wtf link from a file',
                'genURL' => 'Generate a jump.wtf link from a web URL',
                'delURL' => 'Delete an existing jump.wtf link',
                'jumpTo' => 'Resolve a jump.wtf link',
                'help' => 'Show help information',
              ],
            'required-params' =>
              [],
          ],
      ];
  }
}
