#!/bin/bash
#This script is by NO MEANS SAFE!

DIRECTORY=${1%/}
HEADERFILE=header.txt

function update_header() {
	echo "Modifying: $1"
	cp "$1" "$1".tmp
	cat "$1" | grep -n -e '*\{76\}' > /dev/null
	if [ "$?" -eq 0 ];
	then
		cat "$1" | grep -n -e '\*\{76\}'|\
		awk -F':' '{print$1}' | tail -1 |\
	        xargs -I % expr 1 + % |\
	       	xargs -I % tail -n +% "$1" > "$1".tmp
	fi
	cat "$HEADERFILE" "$1".tmp > "$1"
	rm "$1".tmp
}

shopt -s globstar
for file in $DIRECTORY/**/*.cpp; do
  [ -f "$file" ] || continue
  update_header "$file"
done
for file in $DIRECTORY/**/*.h; do
  [ -f "$file" ] || continue
  update_header "$file"
done

