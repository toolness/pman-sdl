SUBDIRS = docs resources src VisualC
EXTRA_DIST = BUGS auto-rebuild.sh auto-package.sh

# install and uninstall have been completely replaced so that the
# installer copies all essential program files into the /release
# directory, and the uninstaller deletes the /release directory.

install:
	test -d release \
	|| mkdir release
	cp -p resources/bmp/*.bmp release
	cp -p src/pman${EXEEXT} release

uninstall:
	rm -r -f release

