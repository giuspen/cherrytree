function changeFrame(file_path){
	console.log(file_path);
	var iframe = document.getElementById("page_frame");
	iframe.src = file_path;
}

window.onload = function(){ 
	document.body.style.overflowY = 'hidden';
}