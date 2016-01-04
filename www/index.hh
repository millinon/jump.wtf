<?hh

/*class :file_span extends :span {
  attribute bool disabled;
}*/

function input_label($label){
    return <span class="input-group-addon">&nbsp;{$label}</span>;
}

function url_input() {
    return <div class="input-group">
        {input_label("URL:")}
                    <input type="url" class="form-control" name="input-url" id="input-url" placeholder="http://www.example.com" maxlength={jump_config::MAX_URL_LEN} autofocus={true} required={true} autocomplete="off" />
                    <input style="display:none" />
                </div>;
}

function file_input() {
    return <div class="input-group" id="file-input-group">
        {input_label("File:")}
        <span id="file-group" class="form-control">
            <input type="hidden" name="MAX_FILE_SIZE" value={(string) jump_config::MAX_LOCAL_FILE_SIZE} />
            <span class="btn btn-default btn-file" id="file-button">
            Browse
            <input type="file" class="form-control" name="input-file" id="input-file" />
                </span>
                <span id="file-label" style="margin-left: 5px"></span>
        </span>
    </div>;
}

function pass_input(string $id_prefix) {
    return <div class="input-group">
        {input_label("Password for deletion:")}
        <input type="password" class="form-control" name="password" id={$id_prefix . '-pass'} maxlength={jump_config::MAX_PASS_LEN} autocomplete="off" placeholder="(optional)" />
        </div>;
}

function expire_input(string $id_prefix) {
    return <div class="input-group">
        <span class="input-group-addon">
        <input type="checkbox" name="expires" id={$id_prefix . '-expires'} />&nbsp;Expire after clicks:
        </span>
            <input type="number" class="form-control" name="clicks" id={$id_prefix . '-clicks'} value="" min={(string) 1} max={(string) jump_config::MAX_CLICKS} step={1.0} />
        </div>;
}

function action_input(string $id_prefix, string $action) {
    return <input type="hidden" name="action" id={$id_prefix . '-action'} value={$action} />;
}

function submit_input(string $id_prefix, string $icon) {
    return <button name="submit-button" id={$id_prefix . '-submit'} class="btn btn-primary submit-btn">
                <span id={$id_prefix . '-glyph'} class={'glyphicon ' . $icon} aria-hidden="true"></span>&nbsp;Submit
        </button>;
}

function del_key_input() {
    return <div class="input-group">
        {input_label('ID:')}
        <input type="text" class="form-control" name="del-url" id="del-url" pattern={'(https?://jump.wtf/)?' . key_config::regex} placeholder="https://jump.wtf/fooBar" required={true} autocomplete="off" maxlength={key_config::MAX_LENGTH} />
    </div>;
}

function del_pass_input() {
    return <div class="input-group">
        {input_label('Password:')}
        <input type="password" class="form-control" name="password" id="del-pass" maxlength={jump_config::MAX_PASS_LEN} autocomplete="off" />
        </div>;
}

function none_input() {
    return <input style="display:none" />;
}

function i_main(): void {
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
                <a target="_blank" href="https://f.jump.wtf/wZw.txt">
                  Terms of Service and Privacy Policy
                </a>. <br />
                This site's source code is available on <a href="https://github.com/millinon/jump.wtf">GitHub</a>.
              </p>
            </div>
          </div>
    
          <div class="tabs container-fluid" id="form_container">
          
          <input type="radio" name="tabs" id="tab_url" checked={true} />
            <label for="tab_url">
                <i class="fa fa-html5"></i><span>Shorten a URL</span>
            </label>
            
            <input type="radio" name="tabs" id="tab_file" />
            <label for="tab_file">
                <i class="fa fa-html5"></i><span>Upload a file</span>
            </label>
            
                <input type="radio" name="tabs" id="tab_del"/>
            <label for="tab_del">
                <i class="fa fa-html5"></i><span>Delete a link</span>
            </label>
            
            <div id="tab-url-form" class="tab-content">
            <form id="new-url" action="s" method="post" enctype="multipart/form-data" autocomplete="off">
                {url_input()}
                {none_input()}
                {pass_input('url')}
                {expire_input('url')}
                {action_input('url', 'new-url')}
                {submit_input('url', 'glyphicon-link')}
            </form>
            </div>

            <div id="tab-file-form" class="tab-content">
            <form id="new-file" action="s" method="post" enctype="multipart/form-data" autocomplete="off">
                {file_input()}
                {none_input()}
                {pass_input('file')}
                {expire_input('file')}
                {action_input('file', 'new-file')}
                {submit_input('file', 'glyphicon-cloud-upload')}
            </form>
            </div>

            <div id="tab-del-form" class="tab-content">
            <form id="delete" action="s" method="post" enctype="multipart/form-data" autocomplete="off">
                {del_key_input()}
                {none_input()}
                {del_pass_input()}
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
