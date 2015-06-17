<?hh
/*
include('b/blackhole.hh');
include('p/log.hh');

require('p/header.hh');
require('p/forms.hh');
require('p/footer.hh');

require('p/aws.hh');
require('p/aws_constants.hh');
*/

function get_num_links(): string{
	$dyclient = mk_aws()->get('DynamoDb');

	$result = $dyclient->describeTable(
		array(
			'TableName' => aws_config::LINK_TABLE));
	return (string) $result['Table']['ItemCount'];
}

function is_meta(): string{
	if(
	( isset($_SERVER['HTTP_REFERER']) && preg_match("/(https?:\/\/)?(cdn\.)?jump\.wtf(\/.*)?/", $_SERVER['HTTP_REFERER']))
	||
	( isset($_ENV['HTTP_REFERER']) && preg_match("/(https?:\/\/)?(cdn\.)?jump\.wtf(\/.*)?/", $_ENV['HTTP_REFERER']))
	) return "This is so meta.";
	else return "";
}

function i_main(): void{

echo gen_html_tag();
echo gen_head();

$body = <body></body>;
$body->appendChild(gen_nav());
$body->appendChild(
<div class="jumbotron">
	<div class="container centered">
		<h1>Link Shortening / File Hosting Service</h1>
		<p>
			An experiment with AWS resources.<br/>
			By submitting a link or file, you are agreeing to this site's <a href="t.txt">Terms of Service and Privacy Policy</a>.
			<br /><br />
			A total of {get_num_links()} links have been generated.
			<br /><br />
			{is_meta()}
		</p>
	</div>
</div>
);

$body->appendChild(gen_form());

$body->appendChild(gen_footer());
foreach( gen_footer_scripts() as $script )
	$body->appendChild($script);

echo $body;


echo "</html>";
}

//i_main();
