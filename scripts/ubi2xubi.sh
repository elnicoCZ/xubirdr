#!/bin/bash
################################################################################
# Calling sequence ./ubi2xubi.sh UBIIMG > XUBIIMG
################################################################################

# Check the number of arguments
if [ $# -ne 1 ];
then
	echo "Usage: $0 UBIIMG" >&2
	echo "The script prints to its standard output."
	exit 1
fi
UBIFILENAME=$1

# Check the input is a UBI image
UBIHEADER=$(head -c 4 $UBIFILENAME)
if [ $? -ne 0 ] || [ $UBIHEADER != "UBI#" ];
then
	echo "$UBIFILENAME is not a valid UBI image" >&2
	exit 2
fi

# Get the UBI file size
FILESIZE=$(stat -c%s "$UBIFILENAME")

### Print the header
# 4 bytes file type
printf "XUBI"
# 4 bytes size of the UBI image
for i in {0..3};
do
	BYTE=$(($FILESIZE%256))
	FILESIZE=$(($FILESIZE/256))
	printf \\$(printf '%03o' $BYTE)
done
# 4 bytes magical number for endianess checking
printf \\$(printf '%03o' 3)
printf \\$(printf '%03o' 2)
printf \\$(printf '%03o' 1)
printf \\$(printf '%03o' 0)

### Print the body
cat $UBIFILENAME
