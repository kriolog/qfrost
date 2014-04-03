#!/bin/bash
rm -rf icons/*

ORIGIN="/usr/share/icons/oxygen/"
DEST="./icons"
line_length=0
grep -oPhs '(?<=QIcon::fromTheme\(").*?(?="\))' `find ../src -name "*.cpp"` | sort | uniq | while read i; do
    if [ $line_length -gt 40 ]; then
       line_length=0
       echo
    fi
    line=`echo "$i "`
    line_length=$((`echo ${#line}` + ${line_length}))
    echo -n "${line}"
    find "$ORIGIN" -name "$i.png" | while read j; do
	ICON_PATH=`echo $j | sed 's#'"$ORIGIN"'##'`
	ICON_DIR=`dirname $ICON_PATH`
	ICON_DEST=`echo $DEST/$ICON_DIR`
	if [ ! -d "$ICON_DEST" ]; then
	    mkdir -p "$ICON_DEST"
	fi
	cp "$j" "$ICON_DEST"
    done	
done
echo

cp "$ORIGIN/index.theme" "$DEST"

(
echo '<!DOCTYPE RCC><RCC version="1.0">'
echo '<qresource>'

find icons -type f | sort | while read i; do
    echo '    <file>'$i'</file>'

done
echo '</qresource>'
echo '</RCC>'
) > icons.qrc
