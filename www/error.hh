<?hh

require_once ('html/header.hh');

require_once ('html/footer.hh');

function error_message(int $status): mixed {
  switch ($status) {
    case 403:
      return
        <p>
          The link you were trying to reach is off-limits.<br />
          Sorry about that!<br />
        </p>;

    case 404:
      return
        <p>
          The link you were looking for could not be found.<br />
          Sorry about that!<br />
        </p>;

    case 409:
      return
        <p>
          The link you clicked has been flagged. Sorry! <br />
        </p>;

    default:
      return error_message(404);
  }
}

function error_page(int $status, string $uri): void {

  error_log("Generated status $status for uri: $uri");

  http_response_code($status);

  echo
    <x:doctype>
      <html lang="en">
        {gen_head($status)}
        <body>
          {gen_nav()}
          <div class="jumbotron">
            <div class="container centered">
              <h1>Oh no!</h1>
              {error_message($status)}
              <br /><br /><br /><br />
              <a href=".">Go back</a>
            </div>
          </div>
          {gen_footer()}
          {gen_footer_scripts()}
        </body>
      </html>
    </x:doctype>
  ;
}
