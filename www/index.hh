<?hh

require_once ('html/header.hh');
require_once ('html/footer.hh');

function input_none() {
  return <div class="input-group"><input class="input-none" /></div>;
}

function input_label(string $label) {
  return <span class="input-group-addon">&nbsp;{$label}</span>;
}

function file_input() {
  return
    <div class="input-group" id="file-input-group">
      {input_label("File:")}
      <span id="file-group" class="form-control">
        <span class="btn btn-default btn-file" id="file-button">
          Browse
          <input
            type="file"
            class="form-control"
            name="input-file"
            id="input-file"
          />
        </span>
        <span id="file-label" class="file-label"></span>
      </span>
    </div>;
}

function expire_input(string $id_prefix) {
  return
    <div class="input-group">
      <span class="input-group-addon">
        <input type="checkbox" name="expires" id={$id_prefix.'-expires'} />
        &nbsp;Expire after clicks:
      </span>
      <input
        type="number"
        class="form-control"
        name="clicks"
        id={$id_prefix.'-clicks'}
        value=""
        min={(string) 1}
        max={(string) jump_config::MAX_CLICKS}
        step={1.0}
      />
    </div>;
}

function action_input(string $id_prefix, string $action) {
  return
    <input
      type="hidden"
      name="action"
      id={$id_prefix.'-action'}
      value={$action}
    />;
}

function submit_input(string $id_prefix, string $icon) {
  return
    <button
      name="submit-button"
      id={$id_prefix.'-submit'}
      class="btn btn-primary submit-btn">
      <span
        id={$id_prefix.'-glyph'}
        class={'glyphicon '.$icon}
        aria-hidden="true">
      </span>&nbsp;Submit
    </button>;
}

function url_input(
  string $id_prefix,
  string $title,
  string $placeholder,
  int $maxlength = jump_config::MAX_URL_LEN,
  bool $autofocus = false,
) {

  $id = $id_prefix.'-url';

  return
    <div class="input-group" id={$id_prefix.'-url-input-group'}>
      {input_label($title)}
      <input
        type="url"
        class="form-control"
        name={$id}
        id={$id}
        placeholder={$placeholder}
        maxlength={$maxlength}
        autofocus={$autofocus}
        required={true}
        autocomplete="off"
      />
    </div>;
}

function pass_input(
  string $id_prefix,
  string $title = 'Password:',
  bool $required = false,
) {

  $id = $id_prefix.'-pass';

  $input = null;

  if ($required) {
    $input =
      <input
        type="password"
        class="form-control"
        name="password"
        id={$id}
        maxlength={jump_config::MAX_PASS_LEN}
        autocomplete="on"
        required={true}
      />;
    //$input->setAttribute('required', 'true');
  } else {
    $input =
      <input
        type="password"
        class="form-control"
        name="password"
        id={$id}
        maxlength={jump_config::MAX_PASS_LEN}
        autocomplete="on"
        placeholder="(Optional)"
      />;
    //$input->setAttribute('placeholder', '(Optional)');
  }

  return
    <div class="input-group">
      {input_label($title)}
      {$input}
    </div>;
}

function promo_input(string $id_prefix) {
  $id = $id_prefix.'-promo';

  return
    <div class="input-group" id={$id_prefix.'-promo-input-group'}>
      {input_label('Promo code:')}
      <input
        type="text"
        class="form-control"
        name="promo-code"
        id={$id}
        autocomplete="off"
      />
    </div>;
}

function key_input(string $id_prefix) {

  $id = $id_prefix.'-key';

  return
    <div class="input-group" id={$id_prefix.'-key-input-group'}>
      {input_label('Custom key:')}
      <div
        id={$id_prefix.'-wrap-expires'}
        data-toggle="tooltip"
        title="Promo code required">
        <input
          type="text"
          class="form-control"
          name="custom-url"
          id={$id}
          autocomplete="off"
          data-toggle="tooltip"
          title="Promo code required"
        />
      </div>
    </div>;

}

function collapse_input(string $id_prefix) {
  return
    <div class="input-group">
      <button
        id={$id_prefix."-col-button"}
        type="button"
        class="btn showdetails"
        data-toggle="collapse"
        data-target={'#'.$id_prefix.'-opt'}>
        <span class="glyphicon glyphicon-collapse-down">&nbsp;</span>More
        Options
      </button>
      <div
        id={$id_prefix.'-opt'}
        class="collapse container-fluid form-container">
        {pass_input($id_prefix, 'Password for deletion:')}
        {expire_input($id_prefix)}
        {key_input($id_prefix)}
        {promo_input($id_prefix)}
      </div>
    </div>;
}

function i_main(): void {

  $tab_classes =
    "container-fluid col-xs-12 col-sm-12 col-md-6 col-lg-6 col-md-offset-3 col-lg-offset-3 tab-content";

  echo
    <x:doctype>
      <html lang="en">
        {gen_head()}
        <body>
          {gen_nav()}
          <div class="jumbotron">
            <div class="container centered">
              <h1>Link Shortening / File Hosting Service</h1>
              <p>
                By submitting a link or file, you are agreeing to this site's
                <a target="_blank" href="https://jump.wtf/tos">
                  Terms of Service and Privacy Policy
                </a>. <br />
                This site's source code is available on
                <a href="https://jump.wtf/github">GitHub</a>. <br /> <br />
                Help support the operation of this site:
                <a
                  class="coinbase-button"
                  href=
                    "https://www.coinbase.com/checkouts/bd79a891a81b4acd1cdf9ec96db2cf20">
                  donate bitcoins
                </a>
              </p>
            </div>
          </div>
          <div class="tabs container-fluid" id="form_container">
            <input type="radio" name="tabs" id="tab_url" checked={true} />
            <label for="tab_url">
              <i></i><span>Shorten a&nbsp;</span>URL
            </label>
            <input type="radio" name="tabs" id="tab_file" />
            <label for="tab_file">
              <i></i><span>Upload a&nbsp;</span>File
            </label>
            <input type="radio" name="tabs" id="tab_del" />
            <label for="tab_del">
              <i></i>Delete<span>&nbsp;a Link</span>
            </label>
            <div id="tab-url-form" class={$tab_classes}>
              <form
                id="new-url"
                action="s"
                method="post"
                enctype="multipart/form-data">
                {url_input(
                  'input',
                  'URL:',
                  'http://example.com',
                  jump_config::MAX_URL_LEN,
                  true,
                )}
                {input_none()}
                {collapse_input('new')}
                {action_input('new', 'new-url')}
                {submit_input('new', 'glyphicon-link')}
              </form>
            </div>
            <div id="tab-file-form" class={$tab_classes}>
              <form
                id="new-file"
                action="s"
                method="post"
                enctype="multipart/form-data">
                {file_input()}
                {collapse_input('file')}
                {action_input('file', 'new-file')}
                {submit_input('file', 'glyphicon-cloud-upload')}
              </form>
            </div>
            <div id="tab-del-form" class={$tab_classes}>
              <form
                id="delete"
                action="s"
                method="post"
                enctype="multipart/form-data">
                {url_input(
                  'del',
                  'URL:',
                  'https://jump.wtf/fooBar',
                  jump_config::MAX_URL_LEN,
                )}
                {input_none()}
                {pass_input('del', 'Password:', true)}
                {action_input('del', 'del-url')}
                {submit_input('del', 'glyphicon-trash')}
              </form>
            </div>
          </div>
          {gen_footer()}
          {gen_footer_scripts()}
        </body>
      </html>
    </x:doctype>
  ;
}
