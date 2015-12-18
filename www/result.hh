<?hh

function heading(string $action): string {
  switch ($action) {
    case "del_file":
    case "del_url":
      return "Deletion successful";

    case "url_success":
      return "Link generated";

    case "file_success":
      return "File uploaded";

    case "error":
      return "Error!";

    default:
      header('location:./');
      die();
      return "";
  }
}

function message(string $action, ?string $message): mixed {
  switch ($action) {
    case "del_file":
      return <p>Link deleted; file deletion pending.</p>;
    case "del_url":
      return <p>Link deleted.</p>;

    case "url_success":
    case "file_success":
      return
        <p>
          Your link has been generated:
          <a id="newlink" href={$message} target="_blank">{$message}</a>
        </p>;

    case "error":
      return <p>{$message}</p>;
  }
}

function r_main(): void {

  session_start();

  error_log('action = '.$_SESSION['action']);
  $msg = isset($_SESSION['message']) ? $_SESSION['message'] : "";

  echo
    <x:doctype>
      <html lang="en">
        {gen_head()}
        <body>
          {gen_nav()}
          <div class="jumbotron">
            <div class="container centered">
              <h2>{heading($_SESSION['action'])}</h2>
              {message(
                $_SESSION['action'],
                $msg,
              )}
              <button
                style=
                  {($_SESSION['action'] === 'url_success' ||
                    $_SESSION['action'] === 'file_success')
                    ? ""
                    : "display:none"}
                id="copybutton"
                class="btn btn-default"
                data-clipboard-text=
                  {isset($_SESSION['message']) ? $_SESSION['message'] : ""}>
                <span class="glyphicon glyphicon-share" aria-hidden="true">
                </span>
                Copy to clipboard
              </button>
              <br /><br /><br /><br />
              <a href=".">Go back</a>
            </div>
          </div>
          {gen_footer()}
          {gen_footer_scripts()}
          <script
            src=
              {"//".jump_config::CDN_HOST.
              "/js/".
              file_get_contents("htdocs/js/clip.js.latest")}>
          </script>
        </body>
      </html>
    </x:doctype>
  ;
}
