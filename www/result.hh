<?hh

require_once ('vendor/facebook/xhp-lib/init.php');

require_once ('html/header.hh');
require_once ('html/footer.hh');

function heading(): string {
  if (empty($_SESSION['success']) || !$_SESSION['success']) {
    return "Error!";
  }

  switch ($_SESSION['action']) {
    case 'delURL':
      return 'Deletion successful';
      break;

    case 'genURL':
      return 'Link generated';
      break;

    case 'genFileURL':
      return 'File uploaded';
      break;

    default:
      return 'Error!';
      break;
  }
}

function message(): mixed {

  if (empty($_SESSION['action'])) {
    return <p>Invalid query.</p>;
  } else if (empty($_SESSION['success']) || !$_SESSION['success']) {
    if (empty($_SESSION['message'])) {
      return <p>Unknown error.</p>;
    } else {
      return <p>{$_SESSION['message']}</p>;
    }
  }

  // past this point, success
  switch ($_SESSION['action']) {
    case 'genURL':
      return
        <p>
          Your link has been generated:
          <a id="newlink" href={$_SESSION['url']} target="_blank">
            {$_SESSION['url']}
            </a>
            <br />
            <br />
            <img src={"https://chart.googleapis.com/chart?chs=300x300&cht=qr&chl=". urlencode($_SESSION['url'])."&choe=UTF-8"} />
        </p>;
      break;

    case 'genFileURL':
      return
        <p>
          Your file has been uploaded:
          <a id="newlink" href={$_SESSION['url']} target="_blank">
            {$_SESSION['url']}
            </a>
            <br />
            <br />
            <img src={"https://chart.googleapis.com/chart?chs=300x300&cht=qr&chl=". urlencode($_SESSION['url'])."&choe=UTF-8"} />
        </p>;
      break;

    case 'delURL':
      if ($_SESSION['was-file']) {
        return <p>Link deleted, file deletion pending.</p>;
      } else {
        return <p>Link deleted.</p>;
      }
      break;

    default:
      return <p>Something weird happened.</p>;
      break;
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

  $container =
    <div class="container centered">
      <h2>{heading()}</h2>
      {message()}<br />
    </div>;

  if (isset($_SESSION['hidden-url'])) {
    $container->appendChild(
      <p>
        Hidden service URL:
        <a id="tor-url" href={$_SESSION['hidden-url']} target="_blank">
          {$_SESSION['hidden-url']}
        </a>
      </p>,
    );
  }

  $buttons = <div id="button-container" class="container centered"></div>;

  if (isset($_SESSION['url'])) {
    $buttons->appendChild(
      <p>
        {copy_button('copy-url', 'Copy to clipboard', $_SESSION['url'])}
      </p>
    );

    if (isset($_SESSION['cdn-url'])) {
      $buttons->appendChild(
        <p>
          {copy_button('copy-cdn', 'Copy direct link', $_SESSION['cdn-url'])}
        </p>,
      );
      // $buttons->appendChild(<br />);
      // $buttons->appendChild(
      //   copy_button('copy-cdn', 'Copy direct link', $_SESSION['cdn-url']),
      // );
      // $buttons->appendChild(<br />);
    }

    if (isset($_SESSION['hidden-url'])) {
      $buttons->appendChild(
        <p>
          {copy_button(
            'copy-tor',
            'Copy hidden service link',
            $_SESSION['hidden-url'],
          )}
        </p>,
      );
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

  if (!isset($_SESSION['action'])) {
    $_SESSION['success'] = false;
    $_SESSION['message'] = 'Invalid request';
  }

  $msg = isset($_SESSION['message']) ? $_SESSION['message'] : "";
  $body =
    <body>
      {gen_nav()}
      <div class="jumbotron">
        {make_container()}
      </div>
      {gen_footer()}
      {gen_footer_scripts()}
    </body>;
  if (isset($_SESSION['url'])) {
    $body->appendChild(
      <script
        src=
          {jump_config::CDN_HOST.
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
