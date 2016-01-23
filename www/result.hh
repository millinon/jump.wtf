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

function message(string $action, string $message): mixed {
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

function copy_button(string $id, string $label, string $href) {
  return
    <button
      id={$id}
      class="btn btn-default copy-btn"
      data-clipboard-text={$href}>
      <span class="glyphicon glyphicon-share" aria-hidden="true"></span>
      {$label}
    </button>;
}

function make_container() {

  $container = null;

  $msg = "";

  if (isset($_SESSION['url'])) {
    $msg = $_SESSION['url'];
  } else if (isset($_SESSION['message'])) {
    $msg = $_SESSION['message'];
  }

  $container =
    <div class="container centered">
      <h2>{heading($_SESSION['action'])}</h2>
      {message($_SESSION['action'], $msg)}<br />
    </div>;

  $buttons = <div id="button-container" class="container centered"></div>;

  if ($_SESSION['action'] === 'url_success' ||
      $_SESSION['action'] === 'file_success') {
    $buttons->appendChild(
      copy_button('copy-url', 'Copy to clipboard', $_SESSION['url']),
    );

    if (isset($_SESSION['cdn-url'])) {
      $buttons->appendChild(<br />);
      $buttons->appendChild(
        copy_button('copy-cdn', 'Copy direct link', $_SESSION['cdn-url']),
      );
      $buttons->appendChild(<br />);
    }
  }

  $buttons->appendChild(
    <div id="copy-success" class="alert alert-success copy-message">
      Copied to clipboard.
    </div>,
  );
  $buttons->appendChild(<br />);
  $buttons->appendChild(
    <div id="copy-failure" class="alert alert-warning copy-message">
      {"Couldn't copy to clipboard."}
    </div>,
  ); // this line messes up vim, so I wrapped it in {}s, sorry

  $container->appendChild($buttons);
  $container->appendChild(<p class="back"><a href=".">Go back</a></p>);

  return $container;
}

function r_main(): void {

  session_start();

  $msg = isset($_SESSION['message']) ? $_SESSION['message'] : "";
  $body =
    <body>
      {gen_nav()}
      <div class="jumbotron">
        {make_container()}
      </div>
      {gen_footer()}
      {gen_footer_scripts()}
      <script
        src=
          {"//".
          jump_config::CDN_HOST.
          "/js/".
          file_get_contents("htdocs/js/clip.js.latest")}>
      </script>
    </body>;
  if ($_SESSION['action'] === 'url_success' ||
      $_SESSION['action'] === 'file_success') {
    $body->appendChild(
      <script
        src=
          {'//'.
          jump_config::CDN_HOST.
          '/js/'.
          file_get_contents('htdocs/js/clip.js.latest')}>
      </script>,
    );
  }

  echo
    <x:doctype>
      <html lang="en">
        {gen_head()}
        {$body}
      </html>
    </x:doctype>
  ;
}
