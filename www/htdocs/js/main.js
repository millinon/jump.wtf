
// add event handlers when the page has loaded
$(document).ready(function(){
    
    new_file = $("#new_file");

    // remove filesystem path from selected file name
	new_file.change(function(){
		$("#file-label").text(new_file.val().replace(/^.*\\/, "").replace(/^.*\//, ""));});

    // respond to expires checkbox
	$("#expires").change(function(){
        c = $("#clicks");
		if($(this).is(":checked")){
						c.prop("disabled",false);
						c.prop("value",1);
		} else {
				c.prop("disabled",true);
				c.prop("value","");
		}});

    // URL mode is default
	$("#file-group").attr("disabled", true);
	$("#file-button").attr("disabled", true);

    // respond to URL vs file mode
	$("input[name='sub_type']").change(function (){
		radioValue = $(this).val();
        url = $("#new_url");
        glyph = $("#newsub_glyph");

        // disable URL input, enable file input
		if(radioValue === "file"){
				url.prop("value","");
				url.prop("disabled",true);
				url.prop("placeholder","");
				
				new_file.prop("disabled",false);
				$("#file-group").attr("disabled", false);
				$("#file-button").attr("disabled", false);
				$("#file-label").text("No file selected");

				glyph.removeClass("glyphicon-link");
				glyph.addClass("glyphicon-cloud-upload");
		} else {
        // disable file input, enable URL input
				url.prop("value","");
				url.prop("disabled",false);
				url.prop("placeholder","http://www.example.com/");
				
				new_file.prop("disabled",true);
				$("#file-group").attr("disabled", true);
				$("#file-button").attr("disabled", true);
				$("#file-label").text("")

				new_file.wrap('<form>').closest('form').get(0).reset();
				new_file.unwrap();
				
			    glyph.removeClass("glyphicon-cloud-upload");
				glyph.addClass("glyphicon-link");
		}
    });

    // prevent duplicate file submissions
    $("#new_submit").submit(function(){
		type = $("input[name='sub_type']:checked").val();
		if(type == "file"){
            newsub = $("#newsub");

				newsub.html(newsub.html().replace("Submit","Uploading..."));
				newsub.prop("disabled",true);
		} else if(type == "url"){
		
		}	

    });

    // super secret easter egg
    var mouseDist = 0;
    var lastPos = [0, 0];
    var text = [
        "I must not fear. Fear is the mind-killer. Fear is the little-death that brings total obliteration. I will face my fear. I will permit it to pass over me and through me. And when it has gone past I will turn the inner eye to see its path. Where the fear has gone there will be nothing. Only I will remain.",
        "Once men turned their thinking over to machines in the hope that this would set them free. But that only permitted other men with machines to enslave them.",
        "If you ask 'Should we be in space?' you ask a nonsense question. We are in space. We will be in space.",
        "Those who believe in telekinetics, raise my hand.",
        "Advertising is legalized lying.",
        "The path of least resistance is the path of the loser.",
        "A learning experience is one of those things that say, 'You know that thing you just did? Don't do that'.",
        "Time is an illusion. Lunchtime doubly so.",
        "I do not fear computers. I fear the lack of them.",
        "Violence is the last refuge of the incompetent.",
        "Insanity is relative. It depends on who has who locked in what cage.",
        "The 'Net' is a waste of time, and that's exactly what's right about it.",
        "The future is not google-able.",
        "Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn.",
        "Fool me once, shame on you. Fool me twice, shame on me.",
        "You see, in this world there's two kinds of people, my friend: Those with loaded guns and those who dig. You dig.",
        "I'm from buenos aires, and I say kill them all!",
        "There are no dangerous weapons; there are only dangerous men.",
        "Bazinga",
        "Smee! Do something intelligent!",
        "That's a paddlin'",
        "Just because I don't care doesn't mean I don't understand.", 
        "We will forsake our countries. " + 
            "We will leave our motherland behind us " +
            "and become one with this earth. We have " +
            "no nation, no philosophy, no ideology. " + 
            "We go where we're needed, fighting, not " + 
            "for government, but for ourselves. " + 
            "We need no reason to fight. " + 
            "We fight because we are needed. ",
        "Si vis pacem, para bellum.",
        "Get busy living, or get busy dying.",
        "By failing to prepare, you are preparing to fail.",
        "Life is either a great adventure or nothing.",
        "Electricity is really just organized lightning.",
        "The only thing necessary for the triumph of evil is that good men do nothing.",
        "There are countless ingredients that make up the human body and mind, like all the components that make up me as an individual with my own personality. Sure, I have a face and voice to distinguish myself from others, but my thoughts and memories are unique only to me, and I carry a sense of my own destiny. Each of those things are just a small part of it. I collect information to use in my own way. All of that blends to create a mixture that forms me and gives rise to my conscience.",
        "I reject your reality and substitute my own.",
        "In this moment, I am euphoric. Not because of any phony god’s blessing. But because, I am enlightened by my intelligence."
            ];

    var speech_anims = [
        //"Alert",
        "CheckingSomething",
        "Explain",
        "GetArtsy",
        "GetAttention",
        "GetTechy",
        "GetWizardy",
        "Thinking",
        "Wave"
            ];
    
    $(document).mousemove(function(event) {
        if(lastPos[0] > 0){
            mouseDist += Math.sqrt(
                Math.pow(
                    event.pageX - lastPos[0],
                    2.0
                    ) + 
                Math.pow(
                    event.pageY - lastPos[1],
                    2.0
                )
            );
        }
        lastPos[0] = event.pageX;
        lastPos[1] = event.pageY;

        if(mouseDist >= 6000){
            $(document).unbind("mousemove");
            clippy.load('Clippy', function(agent){
            
                c = $("#form_container");
                p = c.position();
                a = $(".clippy");

                agent.moveTo(p.left + c.width()/2 - a.width()/2, p.top + c.height()/2 - a.height()/2);
                
                a.click(function(event){
                            agent.stop();
                            agent.play(speech_anims[Math.floor(Math.random() * speech_anims.length)]);
                            agent.speak(text[Math.floor(Math.random() * text.length)]);
                });

                agent.play('Greeting');
            });
        }
    });
});

// callbacks are fantastic
