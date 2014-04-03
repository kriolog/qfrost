#!/bin/bash
#Don't forget to use cmake-bulk-decrufter also!

echo --------------- Styling sources ---------------
# astyle-kdelibs + --add-brackets --preserve-date 
#		   --formatted --suffix=none --align-pointer=name
astyle --indent=spaces=4 --brackets=linux \
       --indent-labels --pad-oper --unpad-paren --pad-header \
       --keep-one-line-statements --convert-tabs --indent-preprocessor \
       --preserve-date --formatted --suffix=none --add-brackets --align-pointer=name \
       `find src -type f -name '*.cpp' -or -name '*.cc' -or -name '*.h' -or -name '*.h.in'`

find -type f -name '*~' | while read i; do
  rm "$i"
done

echo --------- Sanitizing trailing newlines --------
newline='
'
find src -type f -name '*.cpp' -or -name '*.cc' -or -name '*.h' -or -name '*.h.in' | while read file; do
  lastline=$(tail -n 1 $file; echo x); lastline=${lastline%x}
  [ "${lastline#"${lastline%?}"}" != "$newline" ] && echo >> $file
done

echo ---------------- Copying icons ----------------
cd ./res
./copy_used_icons.sh
cd ..

TARBALL="../qfrost.tar.xz"
echo ----------- Saving $TARBALL ------------
rm "$TARBALL"
XZ_OPT=-9 tar cJf "$TARBALL" -C".." $(basename "`pwd`")
