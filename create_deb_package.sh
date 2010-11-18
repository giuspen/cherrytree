#!/bin/sh

mkdir cherrytree_pkg
cd cherrytree_pkg
mkdir DEBIAN
mkdir usr
mkdir usr/bin
mkdir usr/share
mkdir usr/share/application-registry
mkdir usr/share/applications
mkdir usr/share/cherrytree
mkdir usr/share/cherrytree/glade
mkdir usr/share/cherrytree/modules
mkdir -p usr/share/icons/hicolor/scalable/apps
mkdir usr/share/locale
mkdir -p usr/share/mime/packages
mkdir usr/share/mime-info

cp ../deb/control DEBIAN/
cp ../deb/postinst DEBIAN/

cp ../cherrytree usr/bin/

cp ../linux/cherrytree.applications usr/share/application-registry/

cp ../linux/cherrytree.desktop usr/share/applications/

for filename in ../glade/*.*
do
   cp $filename usr/share/cherrytree/glade/
done

for filename in ../modules/*.py
do
   cp $filename usr/share/cherrytree/modules/
done

cp ../linux/cherrytree.svg usr/share/icons/hicolor/scalable/apps/

cd ../locale
python i18n_po_to_mo.py
cd -
for dirname in ../locale/*
do
   if [ -d $dirname ]
   then
      cp -r $dirname usr/share/locale/
   fi
done

cp ../linux/cherrytree.xml usr/share/mime/packages/

cp ../linux/cherrytree.keys usr/share/mime-info/
cp ../linux/cherrytree.mime usr/share/mime-info/

cd ..
dpkg -b cherrytree_pkg
rm -r cherrytree_pkg
