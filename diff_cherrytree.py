import os,sys,random,re

done=False
tmp=['/tmp/doc0','/tmp/doc1']

if not os.path.isdir(tmp[0]):
    os.mkdir(tmp[0])
if not os.path.isdir(tmp[1]):
    os.mkdir(tmp[1])

if len(sys.argv)>1:
    if os.path.exists(".diff"):
        fin=open(".diff","rt")
        a=fin.read(1)
        fin.close()
        if a=="1":
            a="0"
        else:
            a="1"
    else:
        a="0"
    fout=open(".diff","wt")
    fout.write(a)
    fout.close()
    print("python "+sys.path[0]+"/cherrytree -t "+tmp[int(a)]+ " " +sys.argv[1])
    os.system("python "+sys.path[0]+"/cherrytree -t "+tmp[int(a)]+ " " +sys.argv[1])

if(a=="1"):
    diff="git diff"
    if len(sys.argv)>2:
        diff=sys.argv[2]
    print(diff+" "+tmp[0]+" "+tmp[1])
    os.system(diff+" "+tmp[0]+" "+tmp[1])
