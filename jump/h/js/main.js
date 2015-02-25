function t1(){
		if($("#expires").is(":checked")){
						$("#clicks").prop("disabled",false);
						$("#clicks").prop("value",1);
		} else {
				$("#clicks").prop("disabled",true);
				$("#clicks").prop("value","");
		}
}

jQuery(document).ready(function(){$("input[name='sub_type']").change(radioChanged);});

function radioChanged(){
		radioValue = $(this).val();

		if(radioValue == "file"){
				$("#new_url").prop("value","");
				$("#new_url").prop("disabled",true);
				$("#new_url").prop("placeholder","");
				
				//$("#new_url").removeAttr('required');

				//$("#new_file").addAttr('required');
				$("#new_file").prop("disabled",false);

				$("#newsub_glyph").removeClass("glyphicon-link");
				$("#newsub_glyph").addClass("glyphicon-cloud-upload");
		} else if(radioValue == "url"){
				$("#new_url").prop("disabled",false);
				$("#new_url").prop("placeholder","http://www.example.com/");
				
				//$("#new_url").addAttr('required');
				//$("#new_file").removeAttr('required');

				$("#new_file").prop("disabled",true);
				e = $("#new_file");
				e.wrap('<form>').closest('form').get(0).reset();
				e.unwrap();
				
				$("#newsub_glyph").removeClass("glyphicon-cloud-upload");
				$("#newsub_glyph").addClass("glyphicon-link");
		}
}

function subbutton(){
		type = $("input[name='sub_type']:checked").val();
		if(type == "file"){
				$("#newsub").html($("#newsub").html().replace("Submit","Uploading..."));
				$("#newsub").prop("disabled",true);
		} else if(type == "url"){
		
		}	
}

var currentMousePos = { x: -1, y: -1 };
$(document).mousemove(function(event) {
		currentMousePos.x = event.pageX;
		currentMousePos.y = event.pageY;
});

mouseDist = 0;
lastPos = [0, 0];
handle = setInterval(function(){
	mouseDist += Math.sqrt(
			Math.pow(
					currentMousePos.x - lastPos[0],
					2.0
			) + 
			Math.pow(
					currentMousePos.y - lastPos[1],
					2.0
			)
		);
	lastPos[0] = currentMousePos.x;
	lastPos[1] = currentMousePos.y;

	if(mouseDist >= 6000){
		initClippy();
		window.clearInterval(handle);
	}
},200);

function initClippy(){
		clippy.load('Clippy', function(agent){
		agent.show();

		(function randAnim(){
				setTimeout(function(){
						agent.animate();
						randAnim();
				},
				Math.round(Math.random() * (100000 - 10000)) + 10000);
				})();
		});
}

