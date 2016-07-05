<?hh

class ValidationException extends Exception {
  protected $msg;

  public function __construct(string $message = "") {
    parent::__construct();
    $this->msg = $message;
  }

  public function __toString() {
    return $this->msg;
  }
}

class api_validator {

  public static function validate(array $input): array {
    // validating the API against the documentation is terrible, but I spent a bunch
    // of time on the documentation in a format that's PHP-readable, so I'll use it

    if (!isset($input['action'])) {
      throw new ValidationException(
        'Missing action parameter -- try {"action": "help"}',
      );
    }

    $func = $input['action'];
    if (!in_array($func, array_keys(jump_api::$doc))) {
      throw new ValidationException("Unknown action $func");
    }

    $action_ref = jump_api::$doc[$func];
    $params = $action_ref['params'];

    foreach (array_keys($input) as $in_param) {
      if (!isset($action_ref[$in_param])) {
        if ($in_param === 'action')
          continue;
        if (!isset($action_ref['params'][$in_param])) {
          throw new ValidationException("Unknown parameter $in_param");
        }
      }

      $param_ref = $params[$in_param];

      if (isset($param_ref['requires-params'])) {
        foreach ($param_ref['requires-params'] as $dependency) {
          if (!isset($input[$dependency])) {
            throw new ValidationException(
              "Parameter $in_param requires that $dependency is set",
            );
          }
        }
      }
    }

    foreach (array_keys($params) as $param) {

      $param_ref = $params[$param];

      if (!isset($input[$param])) {
        if (isset($param_ref['default'])) {
          $input[$param] = $param_ref['default'];
          // default values have been validated in validate_api.hh
        }
      } else {
        $val = $input[$param];

        // check param type
        if (gettype($val) !== $param_ref['type']) {
          throw new ValidationException(
            "bad type for $param parameter, expected {$param_ref['type']}",
          );
        } else if ($param_ref['type'] === 'integer') {

          // check param bounds
          if (isset($param_ref['max-value']) &&
              intval($val) > $param_ref['max-value']) {
            throw new ValidationException("bad value for $param parameter");
          } else if (isset($param_ref['min-value']) &&
                     intval($input[$param]) < $param_ref['min-value']) {
            throw new ValidationException("bad value for $param parameter");
          }

        } else if ($param_ref['type'] === 'string') {

          if (isset($param_ref['regex'])) {
            if (!preg_match($param_ref['regex'], $input[$param])) {
              throw new ValidationException(
                "input $param didn't match parameter regex: ".
                $param_ref['regex'],
              );
            }
          }

          if (isset($param_ref['min-length']) &&
              strlen($val) < $param_ref['min-length']) {
            throw new ValidationException(
              "length for $param parameter out of range",
            );
          } else if (isset($param_ref['max-length']) &&
                     strlen($val) > $param_ref['max-length']) {
            throw new ValidationException(
              "length for $param parameter out of range",
            );
          }
        }

        if (isset($param_ref['requires-params'])) {
          // check param constraints
          foreach ($param_ref['requires-params'] as $constraint) {
            if (!isset($input[$constraint])) {
              throw new ValidationException(
                "$param requires $constraint to be set",
              );
            }
          }
        }
      }
    }

    // check for missing required parameters -- these are validated as exclusive sets
    foreach ($action_ref['constraints'] as $param_set) {
      $total = 0;
      foreach ($param_set as $param) {
        if (isset($input[$param])) {
          $total++;
        }
      }
      if ($total === 0) {
        throw new ValidationException(
          'One of ['.implode($param_set, ' ').'] must be set',
        );
      } else if ($total > 1) {
        throw new ValidationException(
          'Only one of ['.
          implode($param_set, ' ').
          "] may be set, but $total found",
        );
      }
    }

    // /paramarama

    return $input;
  }
}
