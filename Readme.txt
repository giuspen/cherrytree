With respect,I forked cherrytree and modified it so that it could work well with 'git diff'.
Now,you can transform cherrytree documents into text files through command line parameters.(more details in 'cherrytree -h')
Follow the steps below,and you can manage cherrytree document with git smoothly.

Linux:
1.Edit file ".gitattributes"
  append "*.ctb diff=cherrytree"
2.Excute  'chmod +777 your_cherrytree_dir/.sh_cherrytree'
          'git config --global diff.cherrytree.textconv your_cherrytree_dir/.sh_cherrytree'
3.Try 'git diff'

Windows:
1.Edit file ".gitattributes"
  append "*.ctb diff=cherrytree"
2.Edit file "diff_cherrytree.py"
  you should change the value of array 'tmp'
3.write your own .sh_cherrytree as I did(sorry,I just use Linux)
4.Try 'git diff'

Tips:
1.if you have installed 'meld',you can append 'meld' at the end of
'python $dir/diff_cherrytree.py $1 '
in file '.sh_cherrytree'
2.make sure that file '.diff' doesn't exist in working directory or its contents are "1"
