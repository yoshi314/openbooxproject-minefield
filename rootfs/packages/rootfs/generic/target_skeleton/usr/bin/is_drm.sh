#!/bin/sh

####################################################################
#
# Check if eBook is DRM protected or no, and exit with
# appropriate code
#
####################################################################
# artur.lisiecki@artatech.pl
####################################################################

if [ -z "$1" ];
then
    echo "Usage: $0 eBook_file"
    exit -1
fi

ZIPGREP='/usr/bin/zipgrep'
ZIPINFO='/usr/bin/zipinfo'
CONTENT_OPF='content.opf'

CHECK0=`grep -c META-INF/rights.xml "$1" > /dev/null 2>&1`

if [ $? -eq 0 ];
then
	# PKG is DRM protected!
    # we're running AdobeReader
    exit 1
else
    BASIC_CHECK=`$ZIPINFO -1 "$1" | grep $CONTENT_OPF | wc -l`
    if [ "$BASIC_CHECK" -gt 0 ];
    then
        CHECK1=`$ZIPGREP "font-family:" "$1" *.css | wc -l`
        CHECK2=`$ZIPGREP 'href=".*ttf"' "$1" *.opf | wc -l`
        CHECK3=`$ZIPGREP 'href=".*otf"' "$1" *.opf | wc -l`

        # we should have font inside
        if [ $CHECK1 -gt 0 ];
        then
            if [ $CHECK2 -gt 0 ] || [ $CHECK3 -gt 0 ];
            then
                exit 1
            # broken ttf | otf? - running AdobeReader
            else
                exit 0
            fi
        else
            # we don't have any font inside - running FBReader
            exit 0
        fi
    else
        # we're running FBReader
        exit 0
    fi
fi
