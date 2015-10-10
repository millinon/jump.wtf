<?hh

function get_num_links(): string{
    $dyclient = mk_aws()->get('DynamoDb');

    $result = $dyclient->describeTable(
        array(
            'TableName' => aws_config::LINK_TABLE));
    return (string) $result['Table']['ItemCount'];
}

function is_meta(): string{
/*  if(
    ( isset($_SERVER['HTTP_REFERER']) && preg_match("/(https?:\/\/)?(cdn\.)?jump\.wtf(\/.*)?/", $_SERVER['HTTP_REFERER']))
    ||
    ( isset($_ENV['HTTP_REFERER']) && preg_match("/(https?:\/\/)?(cdn\.)?jump\.wtf(\/.*)?/", $_ENV['HTTP_REFERER']))
    ) return "This is so meta.";
    else return "";
*/
    return "";
}

function i_main(): void
{

    echo <x:doctype>
        <html lang="en">
            {gen_head()}
            <body>
                {gen_nav()}
                <div class="jumbotron">
                    <div class="container centered">
                        <h1>Link Shortening / File Hosting Service</h1>
                        <p>
                            An experiment with AWS resources.<br/>
                            By submitting a link or file, you are agreeing to this site's <a target="_blank" href="https://f.jump.wtf/wZw.txt">Terms of Service and Privacy Policy</a>.
                            <br /><br />
                            This site's source code is available on <a href="https://github.com/millinon/jump.wtf">GitHub</a>.
                        </p>
                    </div>
                </div>
                {gen_form()}
                {gen_footer()}
                {gen_footer_scripts()}
            </body>
        </html>
    </x:doctype>;
}
