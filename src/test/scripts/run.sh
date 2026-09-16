#!/bin/bash

CXXSTANDARD="-std=c++20"
OPTION="-Wall -Werror"

FEATURE_FLAG='-march=native'
INCLUDE_ALT_PATH='/usr/local/include'
LIB_ALT_PATH='/usr/local/lib'

LIB='-lbenchmark -lpthread'

DEFINE='-DBENCHMARK_STATIC_DEFINE'

if [[ $(uname --operating-system) =~ ((MINGW)|(Ms)|(WIN))+. ]]; then
    LIB="$LIB -lshlwapi"
fi

MACHINE=$(uname --machine)
if [[ -n $MACHINE ]]; then
    if [[ $MACHINE == arm\s+[7-9]* ]]; then
        FEATURE_FLAG="$FEATURE_FLAG -mfpu=neon -mfloat-abi=hard -munaligned-access";
    elif [[ $MACHINE == 'x86_64' || $MACHINE == 'amd64' ]]; then
        if [[ ! -f /proc/cpuinfo ]]; then
            echo -e "unable to determine feature flags";
            exit -1;
        fi
        FLAGS=$(cat /proc/cpuinfo | grep -Eio '((avx)|(sse\d*)|(bmi\d*))[0-9]?*')
        if [[ -n $FLAGS ]]; then
            if   [[ $FLAGS =~ (avx2)+ ]];   then FEATURE_FLAG="$FEATURE_FLAG -mavx2"
            elif [[ $FLAGS =~ (sse4_2)+ ]]; then FEATURE_FLAG="$FEATURE_FLAG -msse4_2"
            elif [[ $FLAGS =~ (sse2)+ ]];   then FEATURE_FLAG="$FEATURE_FLAG -msse2"
            fi
            if [[ $FLAGS =~ (bmi1)+ ]]; then FEATURE_FLAG="$FEATURE_FLAG -mbmi1"; fi
            if [[ $FLAGS =~ (bmi2)+ ]]; then FEATURE_FLAG="$FEATURE_FLAG -mbmi2"; fi
        fi
    fi
fi

OPT_LEVEL='-O2'
if [[ -n $2 ]]; then
    if [[ ! $2 =~ \-{1}O{1}[0-3]{1} ]]; then
        echo -e "Invalid option at arg 2\nExpected \`$0 CXXFILENAME -O{OPT_LEVEL} {-EXCLUDE, ...}\`";
        exit -1;
    fi
    OPT_LEVEL=$2;
fi

EXCLUDE=''
if [[ -n $3 ]]; then
    if [[ ! $3 =~ \-{1}[a-zA-Z]+ ]]; then
        echo -e "Invalid option at arg 3\nExpected \`$0 CXXFILENAME -O{OPT_LEVEL} {-EXCLUDE, ...}\`";
        exit -1;
    fi
    EXClUDE=$3;
fi
echo "running g++ $CXXSTANDARD $OPTION $OPT_LEVEL $FEATURE_FLAG $1 -o .run $DEFINE -I$INCLUDE_ALT_PATH -L$LIB_ALT_PATH $LIB"

if [[ -z $1 ]]; then
    if [[ ! -f "./.run" ]]; then
        echo -e "missing filename. Expected \`$0 CXXFILENAME -O{OPT_LEVEL} {-EXCLUDE, ...}\`";
        exit -1;
    fi
    ./.run && exit 0
fi

echo "running g++ $CXXSTANDARD $OPTION $OPT_LEVEL $FEATURE_FLAG $1 -o .run $DEFINE -I$INCLUDE_ALT_PATH -L$LIB_ALT_PATH $LIB"

if g++ $CXXSTANDARD $OPTION $OPT_LEVEL $FEATURE_FLAG $1 -o .run $DEFINE -I$INCLUDE_ALT_PATH -L$LIB_ALT_PATH $LIB; then
    ./.run && exit 0
fi
