<?hh

require_once ('api_validation.hh');

class api_router {

  public static function route(array $input): array {
    switch ($input['action']) {
      case 'genUploadURL':
        return jump_api::genUploadURL($input);
        break;

      case 'genFileURL':
        return jump_api::genFileURL($input);
        break;

      case 'genURL':
        return jump_api::genURL($input);
        break;

      case 'delURL':
        return jump_api::delURL($input);
        break;

      case 'jumpTo':
        return jump_api::jumpTo($input);
        break;

      case 'getBalance':
        return jump_api::getBalance($input);
        break;

      default:
        return [
          'success' => false,
          'message' => 'Invalid action, try {"action":"help"}',
        ];
        break;
    }
  }
}
