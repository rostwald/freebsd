#!/bin/sh

set -e
PATH=$HOME/bin:$PATH

PRISTINE_BASE=/scratch/tmp/kib/obj/scratch/tmp/kib/ino64_master/
CHANGED_BASE=/scratch/tmp/kib/obj/scratch/tmp/kib/ino64/
objs=$(find $CHANGED_BASE \! -regex '.*test.*' \! -regex '.*private.*' \
    -type f -name '*.so.*.full' | sed 's@^'${CHANGED_BASE}'@@' )

exclude_list="libhx509.so.11 libgssapi_ntlm.so.10"

mkdir -p tmp
rm -rf compat_reports tmp/*
troubles=
for obj in $objs; do
#	    -public-headers ${PRISTINE_BASE}/tmp/usr/include
    if echo $obj | grep world32 ; then
	suffix=32
    else
	suffix=""
    fi
    libname=$(basename ${obj}${suffix} | sed 's/\.full//')
    for x in $exclude_list; do
	if [ $x \= $libname ] ; then
		continue 2
	fi
    done
    changed_obj=${CHANGED_BASE}/${obj}
    pristine_obj=$(echo ${changed_obj} | sed s/ino64/ino64_master/g)
    abi-dumper.pl ${pristine_obj} \
      -lver pristine -o tmp/${libname}-pristine.dump
    abi-dumper.pl ${changed_obj} \
      -lver ino64 -o tmp/${libname}-ino64.dump
    if abi-compliance-checker.pl -l ${libname} \
      -old tmp/${libname}-pristine.dump \
      -new tmp/${libname}-ino64.dump ; then : ; else
	troubles="${troubles} ${libname}"
    fi
done

echo "Troublesome libraries: " ${troubles}
