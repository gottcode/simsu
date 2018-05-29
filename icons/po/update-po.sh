#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f simsu.desktop.in
cp simsu.desktop simsu.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e 's/^Name/_Name/' \
	-e 's/^GenericName/_GenericName/' \
	-e 's/^Comment/_Comment/' \
	-i simsu.desktop.in

rm -f simsu.appdata.xml.in
cp simsu.appdata.xml simsu.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e 's/<p>/<_p>/' \
	-e 's/<\/p>/<\/_p>/' \
	-e 's/<summary>/<_summary>/' \
	-e 's/<\/summary>/<\/_summary>/' \
	-e 's/<name>/<_name>/' \
	-e 's/<\/name>/<\/_name>/' \
	-i simsu.appdata.xml.in

cd po
echo ' DONE'


echo -n 'Updating translations...'
for POFILE in *.po;
do
	echo -n " $POFILE"
	msgmerge --quiet --update --backup=none $POFILE description.pot
done
echo ' DONE'


echo -n 'Merging translations...'
cd ..

intltool-merge --quiet --desktop-style po simsu.desktop.in simsu.desktop
rm -f simsu.desktop.in

intltool-merge --quiet --xml-style po simsu.appdata.xml.in simsu.appdata.xml
echo >> simsu.appdata.xml
rm -f simsu.appdata.xml.in

echo ' DONE'
