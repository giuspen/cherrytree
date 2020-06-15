function changeFrame(file_path){
	var iframe = document.getElementById("page_frame");
	iframe.src = file_path;
}

function toggleSubTree(element){
	var nextSibling = element.parentElement.nextElementSibling;
	if(nextSibling.tagName == 'UL'){
		nextSibling.classList.toggle('hide');
		if(element.textContent === '+'){
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


function get_tree() {
	return document.getElementsByClassName("tree")[0]
}


window.onload = function(){ 
	var show_page = window.location.hash.substr(1);
	if (show_page !== '') {
		changeFrame(show_page);
	}

	// Unwrap the header
	var tree = get_tree()
	var para = tree.children[0]

	var index_txt = para.children[0].innerHTML
	var index_title = document.createElement("h1")
	index_title.innerHTML = index_txt
	index_title.id = 'index_header'
	tree.insertBefore(index_title, tree.children[0])


	// Remove unwrapped elements
	para.removeChild(para.children[0])
}
