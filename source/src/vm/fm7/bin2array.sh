#!/bin/sh
mainstr=`xargs -0 | sed s/"."/"&\n"/g | nkf -os -x`
#echo $mainstr;
printf "/* converted by bin2array.sh */\n"
printf "const static uint8 hidden_message_77av_1[] = {\n\t"
i=0;
for Ch in $mainstr ; do
    echo $(printf "%3d" \"$Ch) | \
    awk '{printf "0x%02x,",$1}';
    i=`expr $i + 1`;
    if [ $i -gt 7 ]; then
      printf "\n\t"
      i=0;
    fi
done
printf "0x00\n"
printf "};\n"