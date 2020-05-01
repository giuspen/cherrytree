function changeFrame(file_path){
	var iframe = document.getElementById("page_frame");
	iframe.src = file_path;
}

function toggleSubTree(element){
	var nextSibling = element.parentElement.nextElementSibling;
	if(nextSibling.tagName == 'UL'){
		nextSibling.classList.toggle('hide');
		if(element.textContent == '+'){
			element.textContent = '-'
		}else{
			element.textContent = '+'
		}
	}
}

function expandAllSubtrees(element){
	var subtrees = document.getElementsByClassName("subtree");
	for(var i=0; i<subtrees.length; i++){
		var subtree = subtrees[i];
		subtree.classList.remove('hide');
		subtree.previousElementSibling.firstChild.textContent = '-';
	}
}

function collapseAllSubtrees(element){
	var subtrees = document.getElementsByClassName("subtree");
	for(var i=0; i<subtrees.length; i++){
		var subtree = subtrees[i];
		subtree.classList.add('hide');
		subtree.previousElementSibling.firstChild.textContent = '+';
	}
}

window.onload = function(){ 
	var show_page = window.location.hash.substr(1);
	if (show_page !== '') {
		changeFrame(show_page);
	}
}
