ZeroClipboard.config({swfPath: "https://cdnjs.cloudflare.com/ajax/libs/zeroclipboard/2.2.0/ZeroClipboard.swf" });
var client = new ZeroClipboard(document.getElementById("copybutton"));
client.on("ready",function(readyEvent){
		client.on("aftercopy",function(event){
			btn = document.getElementById("copybutton");
			par = btn.parentNode;
			el = document.createElement("div");
			el.className = "alert alert-success";
			el.setAttribute("style","text-align:center;margin:0 auto;width:20%;");
			el.style.height = btn.clientHeight;
			el.innerHTML = "Copied to clipboard.";
			par.replaceChild(el,btn);

			//										parent.replaceChild(
			//						        event.target.style.display="none";
		});
			});
