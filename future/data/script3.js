right_arrow_heavy = '&#129090;'
left_arrow_heavy = '&#129088;'


function changeFrame(file_path){
	var iframe = document.getElementById("page_frame");
	iframe.src = file_path;
}

function toggleSubTree(element){
	var nextSibling = element.parentElement.nextElementSibling;
	if(nextSibling.tagName === 'UL'){
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
function toggle_tree_panel() {
	var tree = get_tree_panel();
	var toggle_btn = document.getElementById('tree_panel_toggle_btn')
	if (tree.style.display === 'none') {
		tree.style.display = 'inline'
		toggle_btn.innerHTML = left_arrow_heavy
		toggle_btn.onmouseenter = null
	} else {
		tree.style.display = 'none'
		toggle_btn.innerHTML = right_arrow_heavy
		toggle_btn.onmouseenter = toggle_tree_panel
	}
}

function get_tree() {
	return document.getElementsByClassName("tree")[0]
}
function get_tree_panel() {
	return document.getElementsByClassName("tree-panel")[0]
}


window.onload = function() {
	var show_page = window.location.hash.substr(1);
	if (show_page !== '') {
		changeFrame(show_page);
	}

	// Unwrap the header
	let tree = get_tree()
	let para = tree.children[0]

	let index_txt = para.children[0].innerHTML
	let index_title = document.createElement("h1")
	index_title.innerHTML = index_txt
	index_title.id = 'index_header'
	tree.insertBefore(index_title, tree.children[0])


	// Remove unwrapped elements
	para.removeChild(para.children[0])

	// Keybinds
	document.onkeydown = handle_keypress

	// Toggle tree panel button
	let panels = document.getElementsByClassName("two-panels")[0]
	let toggle_btn = document.createElement("button")
	toggle_btn.onclick = toggle_tree_panel
	toggle_btn.innerHTML = left_arrow_heavy
	toggle_btn.id = 'tree_panel_toggle_btn'
	panels.insertBefore(toggle_btn, panels.children[1])

}

function handle_keypress(ev) {
	switch(ev.key) {
		case "Escape":
			toggle_tree_panel()
			break
	}
}