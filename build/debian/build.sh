#!/bin/bash

email=albert748@gmail.com

pkg_name=wiznote
version=1.0

source_root=../..
build_dir=tmp/$pkg_name-$version

rm -r tmp

# copy all source
mkdir -p $build_dir
cp -r $source_root/src $build_dir
cp -r $source_root/share $build_dir
cp -r debian $build_dir/src

# files need install
echo "WizNote /usr/bin" >> ${build_dir}/src/debian/install
echo "../share/* /usr/share" >> ${build_dir}/src/debian/install

cd $build_dir/src

# generate makefile
qmake-qt4 wiznote.pro -r -spec linux-g++

# build
dh_make -p ${pkg_name}_${version} --email ${email} --createorig
dpkg-buildpackage -v${version}

cp ../*.deb ../../..
