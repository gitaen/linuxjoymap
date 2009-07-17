#!/bin/sh
echo "struct keymap_struct {" > validkeys.h
echo -e "\tchar key[32];" >> validkeys.h
echo -e "\tint value;" >> validkeys.h
echo "};" >> validkeys.h
echo "extern struct keymap_struct keymap[];" >> validkeys.h
echo "#include \"validkeys.h\"" > validkeys.c
echo "struct keymap_struct keymap[]={" >> validkeys.c
cat keys.txt |sed 's/\\/\\\\/'|sed 's/"/\\"/'| sed 's/\t[\t]*/", /' |sed 's/^/\t{"/'|sed 's/$/},/' >> validkeys.c
echo -e " \t{\"\", -1}," >> validkeys.c
echo "};" >> validkeys.c
