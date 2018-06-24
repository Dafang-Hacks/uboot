#
# usage:
#	$0 <mbr_file> <xboot_file> <gpt_file> <partition_file> <out_file>
#
# 1. get gpt_start_size from <partition_file>
# 2. out file by this layout
#   --------------------------------------------
#   |  mbr  |  xboot  |  fill zero  |   gpt    |
#   |       |         |             ^          |
#   |       |         |      gpt_start_size    |
#   --------------------------------------------
#

#! /bin/sh

set -u
set -e

if [ $# -ne 5 ]; then
	echo ""
	echo "usage:"
	echo "  $0 <mbr_file> <xboot_file> <gpt_file> <partition_file> <out_file>"
	echo ""
	exit 1
fi

file1=$1
file2=$2
file3=$3
file4=$5
partition_file=$4

# read gpt_header_lba from <partition_file>
gpt_header_lba=$(sed -n -e 's/^[ \t]*//' -e 's/#.*$//' \
                        -e 's/[ \t]*$//' -e 's/gpt_header_lba[ \t]*=[ \t]*//p' $partition_file)
# parse gpt_header_lba size expression
number=$(echo $gpt_header_lba | sed 's/[mMkKgG]//')
ext=$(echo $gpt_header_lba | sed "s/$number//")

if [ "$ext" = "k" -o "$ext" = "K" ]; then
	gpt_header_lba=$(($number * 1024))
elif [ "$ext" = "m" -o "$ext" = "M" ]; then
	gpt_header_lba=$(($number * 1024 * 1024))
elif [ "$ext" = "g" -o "$ext" = "G" ]; then
	gpt_header_lba=$(($number * 1024 * 1024 * 1024))
else
	gpt_header_lba=$number
fi

echo gpt_header_lba = $gpt_header_lba

file1_size=`du -sb $1 | tail -n1 | awk '{print $1;}'`
file2_size=`du -sb $2 | tail -n1 | awk '{print $1;}'`
file1_file2_size=`expr $file1_size + $file2_size`

if [ $file1_file2_size -gt $gpt_header_lba ]; then
	echo "error: $file1 size + $file2 size > gpt_header_lba"
	exit 1
fi

fill_size=`expr $gpt_header_lba - $file1_file2_size`
fill_file=zero_`date +%H%M%S%N`.bin
dd if=/dev/zero of=$fill_file bs=1 count=$fill_size

cat $file1 $file2 $fill_file $file3 > $file4

rm -f $fill_file
