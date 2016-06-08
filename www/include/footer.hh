<?hh

require_once ('vendor/facebook/xhp-lib/init.php');

require_once ('config/jump_config.hh');

function gen_footer(): mixed {
  return
    <footer class="footer">
      <div class="container text-center">
        <p class="text-muted">&copy; Phillip Goldfarb {date("Y")}</p>
      </div>
    </footer>;
}

function gen_footer_scripts(): mixed {
  $cdn_host = jump_config::cdn_host();
  $base = jump_config::base_url();

  if (substr($_SERVER['SERVER_NAME'], -strlen('.onion')) === '.onion') {
    $cdn_host = jump_config::H_CDN_HOST;
    $base = jump_config::H_BASEURL;
  }

  return
    <p>
      <script src={$cdn_host."/vendor/jquery-1.11.1.min.js"}></script>
      <script src={$cdn_host."/vendor/clipboard.js/dist/clipboard.min.js"}>
      </script>
      <script src={$cdn_host."/vendor/bootstrap/dist/js/bootstrap.min.js"}>
      </script>
      <script src={$cdn_host."/vendor/clippy.js/build/clippy.min.js"}>
      </script>
      <script src={$cdn_host."/vendor/dropzone/dist/dropzone.js"}></script>
      <script
        async={true}
        src={$cdn_host."/js/".file_get_contents("htdocs/js/main.js.latest")}>
      </script>
    </p>;
}
