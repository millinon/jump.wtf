#!/usr/bin/env hhvm
<?hh

set_include_path(dirname(__FILE__).'/../www/include');

define('VERIFY', true);

require ('api/api.hh');

/*  The documentation / API validation is pretty solid, but it's fairly complex in terms of hierarchy, so it's easy to mistakenly add a field to the API in the wrong place. This file will verify that for each action defined for the API, the action's fields are well defined. If the API as defined in api_documentation.hh is provably correct, and the validation provably evaluates the API correctly, then the output of the validator will be correct or will throw an exception.
 */

$doc = api_config::api_methods();

function key_error($action, $field, $key, $error) {
  echo "\n{$action}[$field][$key] -- $error\n\n";
  exit(1);
}

$required_param_fields = ['description' => 'string', 'type' => 'string'];

foreach (array_keys($doc) as $action) {

  echo "Checking action $action...\n";

  $action_ref = $doc[$action];

  $params = $doc[$action]['params'];

  $required_fields_found = 0;

  foreach (array_keys($params) as $param_name) {

    echo "    Checking parameter $action.$param_name...\n";

    $param_ref = $params[$param_name];

    foreach (array_keys($required_param_fields) as $field) {
      if (!isset($param_ref[$field])) {
        key_error($action, 'params', $field, "$field field missing");
      } else if (gettype($param_ref[$field]) !==
                 $required_param_fields[$field]) {
        key_error(
          $action,
          'params',
          $field,
          "field has wrong type ".
          gettype($param_ref[$field]).
          ", should be ".
          $required_param_fields[$field],
        );
      }
    }

    if (gettype($param_ref) !== 'array') {
      key_error($action, 'params', $param_name, "is not an array");
    }

    if (isset($param_ref['regex']) && $param_ref['type'] !== 'string') {
      key_error(
        $action,
        'params',
        $param_name,
        "has a regex field, but has type ".$param_ref['type'],
      );
    } else if (isset($param_ref['min-length']) &&
               $param_ref['type'] !== 'string') {
      key_error(
        $action,
        'params',
        $param_name,
        "has a min-length field, but has type ".$param_ref['type'],
      );
    } else if (isset($param_ref['max-length']) &&
               $param_ref['type'] !== 'string') {
      key_error(
        $action,
        'params',
        $param_name,
        "has a max-length field, but has type ".$param_ref['type'],
      );
    } else if (isset($param_ref['min-value']) &&
               $param_ref['type'] !== 'integer') {
      key_error(
        $action,
        'params',
        $param_name,
        "has a min-value field, but has type ".$param_ref['type'],
      );
    } else if (isset($param_ref['max-value']) &&
               $param_ref['type'] !== 'integer') {
      key_error(
        $action,
        'params',
        $param_name,
        "has a max-value field, but has type ".$param_ref['type'],
      );
    }

    if (isset($param_ref['default'])) {
      $default_val = $param_ref['default'];

      echo "        Checking default value sanity...\n";

      if (gettype($default_val) !== $param_ref['type']) {
        key_error(
          $action,
          'params',
          $param_name,
          "default value has wrong type ".
          gettype($default_val).
          ", should be {$param_ref['type']}",
        );
      } else if (isset($param_ref['max-length']) &&
                 count($default_val) > $param_ref['max-length']) {
        key_error($action, 'params', $param_name, "failed max-length check");
      } else if (isset($param_ref['min-length']) &&
                 count($default_val) < $param_ref['min-length']) {
        key_error($action, 'params', $param_name, "failed min-length check");
      } else if (isset($param_ref['max-size']) &&
                 count($default_val) > $param_ref['max-size']) {
        key_error($action, 'params', $param_name, "failed max-size check");
      } else if (isset($param_ref['min-size']) &&
                 count($default_val) < $param_ref['min-size']) {
        key_error($action, 'params', $param_name, "failed min-size check");
      }
    }

    // check required parameters (dependency)
    if (isset($param_ref['requires-params'])) {
      echo "        Checking dependencies...\n";
      $requires = $param_ref['requires-params'];

      if (gettype($requires) !== 'array') {
        key_error(
          $action,
          'params',
          $param_name,
          'requires-params has wrong type '.
          gettype($requires).
          ', should be array',
        );
      }
      foreach ($requires as $req) {
        if (gettype($req) !== 'string') {
          key_error(
            $action,
            'params',
            $param_name,
            'requires-params can only contain strings',
          );
        } else if (!isset($action_ref['params'][$req])) {
          key_error(
            $action,
            'params',
            $param_name,
            "depends on non-existent parameter $req",
          );
        }
      }

    }

    // check min/max ranges
    echo "        Checking min/max bounds sanity...\n";
    if (isset($param_ref['min-value']) &&
        isset($param_ref['max-value']) &&
        ($param_ref['max-value'] - $param_ref['min-value']) <= 0) {
      key_error(
        $action,
        'params',
        'min-value & max-value',
        "have an invalid range",
      );
    } else if (isset($param_ref['min-length']) &&
               isset($param_ref['max-length']) &&
               ($param_ref['max-length'] - $param_ref['min-length']) <= 0) {
      key_error(
        $action,
        'params',
        'min-length & max-length',
        "have an invalid range",
      );
    }
  }

  if (!isset($action_ref['constraints'])) {
    key_error($action, 'constraints', 'NULL', 'field missing');
  }

  echo "    Checking constraint sets...\n";
  foreach ($action_ref['constraints'] as $constraint_set) {
    $set_str = '['.implode($constraint_set, ' ').']';
    echo "        Checking $set_str...\n";

    foreach ($constraint_set as $param) {
      if (!isset($action_ref['params'][$param])) {
        key_error(
          $action,
          'constraints',
          $set_str,
          "includes on non-existent parameter $param",
        );
      }
    }

    // members of each requirement set must not depend on each other, since they exclude each other
    // no, this will not find circular exclusions
    for ($i = 0; $i < count($constraint_set) - 1; $i++) {
      $param_ref = $action_ref['params'][$constraint_set[$i]];
      $param_name = $constraint_set[$i];

      for ($j = $i; $j < count($constraint_set); $j++) {
        $param2_ref = $action_ref['params'][$constraint_set[$j]];
        $param2_name = $constraint_set[$j];

        if (isset($param_ref['default'])) {
          key_error(
            $action,
            'constraints',
            $param_name,
            "parameter has a default value",
          );
        } else if (isset($param2_ref['default'])) {
          key_error(
            $action,
            'constraints',
            $param_name,
            'parameter has a default value',
          );
        } else if (isset($param_ref['requires-params']) &&
                   in_array(
                     $constraint_set[$j],
                     $param_ref['requires-params'],
                   )) {
          key_error(
            $action,
            'constraints',
            $set_str,
            'set members are mutually exclusive but $param_name depends on $param2_name',
          );
        } else if (isset($param2_ref['requires_params']) &&
                   in_array(
                     $constraint_set[$i],
                     $param2_ref['requires-params'],
                   )) {
          key_error(
            $action,
            'constraints',
            $set_str,
            'set members are mutually exclusive but $param2_name depends on $param_name',
          );
        }
      }
    }
  }

  echo '    Validating examples...'."\n";
  foreach ($action_ref['examples'] as $example) {
    try {
      api_validator::validate($example);
    } catch (ValidationException $ve) {
      echo 'Validation failed: '.(string) $ve."\n";
      exit(1);
    }
  }

  echo 'Pass\n';
}

$reject_tests =
  [
    [],
    ['noaction' => true],
    ['action' => ''],
    ['action' => 'fakeaction'],
    ['action' => 'genURL', 'fakeParam' => 'test'],
    ['action' => 'genUploadURL', 'input-url' => 'http://example.com'],
    [
      'action' => 'genURL',
      'input-url' => 'http://example.com',
      'private' => 5,
    ],
    //['action' => 'genURL', 'input-url' => 'not a URL'], // URLs aren't tested during validation, since that's done by filter_var
    [
      'action' => 'genURL',
      'input-url' => 'http://example.com',
      'clicks' => 5,
    ],
    [
      'action' => 'genURL',
      'input-url' => 'http://example.com',
      'private' => true,
      'clicks' => jump_config::MAX_CLICKS + 1,
    ],
    [
      'action' => 'genFileURL',
      'file-data' => 'aGVsbG8K',
      'tmp-key' => '5677988ddaee51.96624275',
    ],
    ['action' => 'genFileURL', 'tmp-key' => 'invalid key'],
    [
      'action' => 'genURL',
      'input-url' => 'http://example.com',
      'private' => true,
      'clicks' => 0,
    ],
    ['action' => 'jumpTo', 'jump-key' => 'https://jump.wtf/foo'],
    [
      'action' => 'jumpTo',
      'jump-key' => 'looooooooooooooooooooooooooooooooooooooooooooongKey',
    ],
    [
      'action' => 'delURL',
      'password' => 'a password',
      'jump-key' => 'https://jump.wtf/fooBar',
    ],
    ['action' => 'delURL', 'password' => '', 'jump-key' => 'foo'],
    [
      'action' => 'genURL',
      'input-url' => 'https://jump.wtf',
      'custom-url' => 'fooBar',
    ],
    ['action' => 'delURL', 'jump-key' => 'realKey'],
    [
      'action' => 'genFileURL',
      'file-data' => 'd2hvb3BzIHRoaXMgaXMgYW4gaW52YWxpZCBleHRlbnNpb24K',
      'extension' => '.loooooooong.invalid.extension',
    ],
  ];

echo "Running tests on invalid input...\n";

foreach ($reject_tests as $test) {
  try {
    api_validator::validate($test);
  } catch (ValidationException $ve) {
    echo "    Pass: ".(string) $ve."\n";
    continue;
  }
  echo "Failed on input: ";
  var_dump($test);
  exit(1);
}

echo "    All passed\n";

/* vim: set ft=php ts=4 sw=4 tw=0 et :*/
