<?hh

require_once ('vendor/facebook/xhp-lib/init.php');

if (!include_once('config/jump_config.hh')) {
  require_once ('config/jump_config.hh.example');
}

function gen_head($title = "JUMP.WTF") {

  $cdn_host = jump_config::cdn_host();
  $base = jump_config::base_url();

  return
    <head>
      <meta charset="utf-8" />
      <title>{$title}</title>
      <meta http-equiv="X-UA-Compatible" content="IE=edge" />
      <meta
        name="description"
        content="Link shortening and file hosting service"
      />
      <meta name="keywords" content="URL,link,shorten,upload,web" />
      <meta name="author" content="Phillip Goldfarb" />
      <meta property="og:title" content="JUMP.WTF" />
      <meta property="og:type" content="website" />
      <meta
        property="og:description"
        content="Link shortening and file hosting service"
      />
      <meta property="og:locale" content="en_US" />
      <meta
        name="viewport"
        content="width=device-width, initial-scale=1, user-scalable=no"
      />
      <link
        rel="shortcut icon"
        type="image/x-icon"
        href="https://f.jump.wtf/favicon.ico"
      />
      <link
        rel="stylesheet"
        href={$cdn_host."/vendor/bootstrap/dist/css/bootstrap.min.css"}
      />
      <link
        rel="stylesheet"
        href={$cdn_host."/vendor/bootstrap/dist/css/bootstrap-theme.min.css"}
      />
      <link
        rel="stylesheet"
        href={$cdn_host."/vendor/clippy.js/build/clippy.css"}
        media="all"
      />
      <link
        rel="stylesheet"
        href="https://fonts.googleapis.com/css?family=Montserrat:700"
        type="text/css"
        data-noprefix=""
      />
      <link
        rel="stylesheet"
        href=
          {$cdn_host."/css/".file_get_contents("htdocs/css/main.css.latest")}
      />
    </head>;
}

function gen_nav() {

  $base = jump_config::base_url();

  //echo "<body>";
  return
    <nav class="navbar navbar-inverse navbar-fixed-top">
      <div class="container">
        <div class="navbar-header">
          <a class="navbar-brand" href={$base}>JUMP.WTF</a>
        </div>
        <div id="navbar" class="navbar-collapse collapse"></div>
      </div>
    </nav>;
}
