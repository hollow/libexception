#!/bin/bash

source /etc/init.d/functions.sh

PROJECT=libexception
VERSION=$(sed 's/^:Version: \(.*\)/\1/;t;d' README)

mkdir -p ~/public_html/projects/${PROJECT}/dist

disttar=~/public_html/projects/${PROJECT}/dist/${PROJECT}-${VERSION}.tar.bz2

ebegin "Generating project page"
rst2html.py < README > ~/public_html/projects/${PROJECT}/index.html
eend $?

ebegin "Creating release tarball"
if [[ -e ${disttar} ]]; then
	eend 1
	echo "!!! ${disttar} exists."
	exit
else
	git archive --format=tar --prefix=${PROJECT}-${VERSION}/ HEAD | \
	bzip2 > ~/public_html/projects/${PROJECT}/dist/${PROJECT}-${VERSION}.tar.bz2
	eend $?
fi

ebegin "Generating documentation"
./autogen.sh
./configure
doxygen Doxyfile
pushd doc/latex
make
popd
rsync -av doc/ ~/public_html/projects/${PROJECT}/doc/
eend $?
