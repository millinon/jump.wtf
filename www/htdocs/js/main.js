$(document).ready(function(){
	
	$("#new_file").change(function(){
		$("#file-label").text($("#new_file").val().replace(/^.*\\/, "").replace(/^.*\//, ""));});

	$("#expires").change(function(){
		if($(this).is(":checked")){
						$("#clicks").prop("disabled",false);
						$("#clicks").prop("value",1);
		} else {
				$("#clicks").prop("disabled",true);
				$("#clicks").prop("value","");
		}});


	$("#file-group").attr("disabled", true);
	$("#file-button").attr("disabled", true);

	$("input[name='sub_type']").change(function (){
		radioValue = $(this).val();

		if(radioValue == "file"){
				$("#new_url").prop("value","");
				$("#new_url").prop("disabled",true);
				$("#new_url").prop("placeholder","");
				
				$("#new_file").prop("disabled",false);
				$("#file-group").attr("disabled", false);
				$("#file-button").attr("disabled", false);
				$("#file-label").text("No file selected");

				$("#newsub_glyph").removeClass("glyphicon-link");
				$("#newsub_glyph").addClass("glyphicon-cloud-upload");
		} else {
				$("#new_url").prop("value","");
				$("#new_url").prop("disabled",false);
				$("#new_url").prop("placeholder","http://www.example.com/");
				
				$("#new_file").prop("disabled",true);
				$("#file-group").attr("disabled", true);
				$("#file-button").attr("disabled", true);
				$("#file-label").text("")

				e = $("#new_file");
				e.wrap('<form>').closest('form').get(0).reset();
				e.unwrap();
				
				$("#newsub_glyph").removeClass("glyphicon-cloud-upload");
				$("#newsub_glyph").addClass("glyphicon-link");
		}
});

    $("#new_submit").submit(function(){
		type = $("input[name='sub_type']:checked").val();
		if(type == "file"){
				$("#newsub").html($("#newsub").html().replace("Submit","Uploading..."));
				$("#newsub").prop("disabled",true);
		} else if(type == "url"){
		
		}	

    });
});

var currentMousePos = { x: -1, y: -1 };
$(document).mousemove(function(event) {
		currentMousePos.x = event.pageX;
		currentMousePos.y = event.pageY;
});


        
function registerClippy(){
    mouseDist = 0;
    lastPos = [0, 0];

    return function(){
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
            clippy.load('Clippy', function(agent){
                agent.show();

                (function randText(){
                    text = [ "Do you need help?", "This should be simple.", "Please hurry up.", "Make up your mind, already.", "I must not fear. Fear is the mind-killer. Fear is the little-death that brings total obliteration. I will face my fear. I will permit it to pass over me and through me. And when it has gone past I will turn the inner eye to see its path. Where the fear has gone there will be nothing. Only I will remain."];

                    min = 10000;
                    max = 100000;

                    setTimeout(function(){
                        agent.animate();
                        agent.speak(text[Math.floor(Math.random() * text.length)]);
                        randText();
                    }, Math.round(Math.random() * (max - min)) + min);
                })();
            });
            window.clearInterval(handle);
        }
    };
}

handle = setInterval(registerClippy(), 200);
