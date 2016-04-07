$(document).ready(function(){

    $("#file-expires").change(function(){
        c = $("#file-clicks");
        if($(this).is(":checked")){
            c.prop("disabled", false);
            c.prop("value",1);
        } else {
            c.prop("disabled", true);
            c.prop("value", "");
        }
    });

    $("#file-expires").change();
    
    $("#new-expires").change(function(){
        c = $("#new-clicks");
        if($(this).is(":checked")){
            c.prop("disabled", false);
            c.prop("value",1);
        } else {
            c.prop("disabled", true);
            c.prop("value", "");
        }
    });

    $("#new-expires").change();
    
    $("#new-promo").on('change textInput input', function(){
        keyfield = $("#new-key");
        hint = $("#new-wrap-expires")

        if(this.value){
            keyfield.prop("disabled", false);
            hint.off(".tooltip");
        } else {
            keyfield.prop("value", "");
            keyfield.prop("disabled", true);
            hint.tooltip("enable");
        }
    });

    $("#new-promo").change();

    
    $("#file-promo").on('change textInput input', function(){
        keyfield = $("#file-key");
        hint = $("#file-wrap-expires")
        
        if(this.value){
            keyfield.prop("disabled", false);
            hint.off(".tooltip");
        } else {
            keyfield.prop("value", "");
            keyfield.prop("disabled", true);
            hint.tooltip("enable");
        }
        });

    $("#file-promo").change();

    $("#input-file").change(function(){
        $("#file-label").text($(this).val().replace(/^.*\\/, "").replace(/^.*\//, ""));
    });

    $("#file-label").text("No file selected");

    

    $(document).ready(function(){
            $('[data-toggle="tooltip"]').tooltip(); 
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
        "In this moment, I am euphoric. Not because of any phony god’s blessing. But because, I am enlightened by my intelligence.",
        "you're a dirty memer, aren't you",
        "omg, gmos!",
        "loading unloader",
        "memeing maymays",
        "from whence comes the realization that not only do we live, but we live as individuals?",
        "lol philosomemes",
        "yes",
        "aoh no, we're all out of bacon!",
        "if things get bad, always remember someone, somewhere loves you.",
        "gods fall and thrones fall, with laws of nature and man crumble at the world's end",
        "if memes can be dreams, so can spleens (and dreams)",
        "reticulating spleens",
        "three times a day, you realize you're breathing manually. this is one of those times",
        "if you sneeze hard enough, can you turn yourself inside out?,"

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
