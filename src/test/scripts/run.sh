#!/bin/bash

CMDLINE_ARGUMENTS=$@
CXXFILE=$1
OUTPUT_FILE="/tmp/.tmp_run"
CXXSTANDARD='-std=c++20'
OPTIONS='-Wall -Werror'
OPT_LEVEL='-O2'

FEATURE_FLAGS='-march=native'
INCLUDE_ALT_PATH='/usr/local/include'
LIB_ALT_PATH='/usr/local/lib'

LIB='-lbenchmark -lpthread'

DEFINE='-DBENCHMARK_STATIC_DEFINE'

if [[ -z $CXXFILE ]]; then
    if [[ ! -f $OUTPUT_FILE ]]; then
        echo -e "missing filename. Expected \`$0 CXXFILENAME -O{OPT_LEVEL} {-OPTIONS...}\`"
        exit -1
    fi
    $OUTPUT_FILE && exit 0
elif [[ ! $CXXFILE =~ *.(c(c|p|(pp)|(xx)){1})|(C(PP)?)$ ]]; then
    echo -e "Invalid C++ file extension. required file must be *.c(c|p|pp|xx) or *.C(PP)"
    exit -1
fi

if [[ $(uname --operating-system) =~ ((MINGW)|(Ms)|(WIN))+. ]]; then
    LIB="$LIB -lshlwapi"
fi

MACHINE=$(uname --machine)
if [[ -n $MACHINE ]]; then
    if [[ $MACHINE =~ arm\s+7* ]]; then
        FEATURE_FLAGS+=" -mfpu=neon -mfloat-abi=hard -munaligned-access";
    elif [[ $MACHINE == 'x86_64' || $MACHINE == 'amd64' ]]; then
        if [[ ! -f /proc/cpuinfo ]]; then
            echo -e "unable to determine feature flags"
            exit -1
        fi
        FLAGS=$(cat /proc/cpuinfo | grep -Eio '((avx)|(sse\d*)|(bmi\d*))[0-9]?*')
        if [[ -n $FLAGS ]]; then
            if   [[ $FLAGS =~ (avx2)+ ]];   then FEATURE_FLAGS+=" -mavx2"
            elif [[ $FLAGS =~ (sse4_2)+ ]]; then FEATURE_FLAGS+=" -msse4_2"
            elif [[ $FLAGS =~ (sse2)+ ]];   then FEATURE_FLAGS+=" -msse2"
            fi
            if [[ $FLAGS =~ (bmi1)+ ]]; then FEATURE_FLAGS+=" -mbmi1"; fi
            if [[ $FLAGS =~ (bmi2)+ ]]; then FEATURE_FLAGS+=" -mbmi2"; fi
        fi
    fi
fi

for OPT in $CMDLINE_ARGUMENTS; do
    if   [[ $OPT =~ \-{1}O{1}([0-3]|[a-zA-Z]*)$ ]]; then
	      OPT_LEVEL=$OPT;
    elif [[ $OPT =~ \-{1}[^O][[:alpha:]]+ ]]; then
        OPTIONS+=" $OPT"
    else
	      echo -e "Invalid option '$OPT'\nExpected \`$0 CXXFILENAME -O{OPT_LEVEL} {-OPTIONS...}\`"
	      exit -1
    fi
done

echo "running g++ $CXXSTANDARD $OPTIONS $OPT_LEVEL $FEATURE_FLAGS $CXXFILE -o .run $DEFINE -I$INCLUDE_ALT_PATH -L$LIB_ALT_PATH $LIB"

if g++ $CXXSTANDARD $OPTIONS $OPT_LEVEL $FEATURE_FLAG $CXXFILE -o $OUTPUT_FILE $DEFINE -I$INCLUDE_ALT_PATH -L$LIB_ALT_PATH $LIB; then
    $OUTPUT_FILE && exit 0
fi
