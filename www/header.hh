<?hh

function gen_head($title="JUMP.WTF"){
return <head>
        <meta charset="utf-8" />
        <meta http-equiv="X-UA-Compatible" content="IE=edge" />
        <title>{$title}</title>
        <meta name="description" content="" />
        <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no" />
        <link rel="shortcut icon" type="image/x-icon" href="https://f.jump.wtf/favicon.ico" />
        <link rel="stylesheet" href={jump_config::CDN_HOST . "/vendor/bootstrap/dist/css/bootstrap.min.css"} />
        <link rel="stylesheet" href={jump_config::CDN_HOST . "/vendor/bootstrap/dist/css/bootstrap-theme.min.css"} />
        <link rel="stylesheet" href={jump_config::CDN_HOST . "/vendor/clippy.js/build/clippy.css"} media="all" />
        <link rel="stylesheet" href="//fonts.googleapis.com/css?family=Montserrat:700" type="text/css" data-noprefix="" />
        <link rel="stylesheet" href={jump_config::CDN_HOST . "/css/" . file_get_contents("htdocs/css/main.css.latest")} />
    </head>;
}

function gen_nav(){

//echo "<body>";
return <nav class="navbar navbar-inverse navbar-fixed-top">
		<div class="container">
				<div class="navbar-header">
<a class="navbar-brand" href="https://jump.wtf">JUMP.WTF</a>
				</div>
				<div id="navbar" class="navbar-collapse collapse">
				</div>
		</div>
</nav>;
}
