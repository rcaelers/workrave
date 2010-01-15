######################################################################
# Defines
######################################################################
BUILD_DATE=`date +"%Y%m%d"`

TINDERBOX_HOME=${HOME}/src/tinderbox
TINDERBOX_SNAPSHOTS_DIR=${TINDERBOX_HOME}/snapshots
TINDERBOX_BIN_DIR=${TINDERBOX_SNAPSHOTS_DIR}/${BUILD_DATE}

WIN32_MAKERUNTIME=${TINDERBOX_HOME}/bin/make-runtime.sh
WIN32_MINGW_ENV=${TINDERBOX_HOME}/bin/mingw32
WIN32_ISCC=${HOME}/.wine/fake_windows/Program Files/Inno Setup 5/ISCC.exe
WIN32_NMAKE=wine ${HOME}/.wine/fake_windows/Program\ Files/Microsoft\ Visual\ C++\ Toolkit\ 2003/bin/nmake.exe

WORKRAVE_GIT_DIR=${TINDERBOX_HOME}/workrave
CONF_FLAGS = --quiet

WORKRAVE_GIT_BRANCH_PREFIX=origin
WORKRAVE_GIT_BRANCH=branch_v1_9
WORKRAVE_TAG=v1_9

#WORKRAVE_GIT_BRANCH_PREFIX=origin
#WORKRAVE_GIT_BRANCH=master
#WORKRAVE_TAG=master

#WORKRAVE_GIT_BRANCH_PREFIX=refs/tags
#WORKRAVE_GIT_BRANCH=v1_9_0-no-networking
#WORKRAVE_TAG=v1_9_0-no-networking
#CONF_FLAGS = --quiet --disable-distribution

#WORKRAVE_GIT_BRANCH_PREFIX=origin
#WORKRAVE_GIT_BRANCH=bug791
#WORKRAVE_TAG=bug791

WORKRAVE_VERSION=`cat ${WORKRAVE_GIT_DIR}/configure.ac | grep AM_INIT_AUTOMAKE | cut -d ','  -f2 | cut -d' ' -f2 | cut -d')' -f1`
WORKRAVE_PREVERSION=`cat ${WORKRAVE_GIT_DIR}/configure.ac | grep AM_INIT_AUTOMAKE | perl -ne ' $$_ =~ /.*([0-9]+).([0-9]+).([0-9]+)/ ; printf "%d.%d.%d.99", $$1, $$2, ($$3 - 1) '`

GIT_URL=git@github.com:rcaelers/workrave.git

unix2dos=perl -e 'while (<>) { s/$$/\r/; print; }'

######################################################################
# Miscellaneous
######################################################################

usage:
	@echo Usage: "make all" to make all files from scratch

all: setup git-setup workrave-win32-all

debug: workrave-win32-debug-build

init: 
	mkdir -p ${TINDERBOX_BIN_DIR}
	( cd `dirname ${WORKRAVE_GIT_DIR}`; git clone ${GIT_URL} ${WORKRAVE_GIT_DIR} )


setup:
	( cd ${WORKRAVE_GIT_DIR}; \
	  mkdir -p ${TINDERBOX_BIN_DIR}; \
          git clean -fdx; \
)

######################################################################
# GIT
######################################################################

git-setup:
	( cd ${WORKRAVE_GIT_DIR}; \
          git clean -fdx; \
          git checkout ${WORKRAVE_GIT_BRANCH}; \
	  git reset --hard HEAD; \
	  git pull; )

git-mkbranch:
	( cd ${WORKRAVE_GIT_DIR}; \
	git branch --track ${WORKRAVE_GIT_BRANCH} ${WORKRAVE_GIT_BRANCH_PREFIX}/${WORKRAVE_GIT_BRANCH}; )


######################################################################
# Windows
######################################################################

workrave-win32-dir=\
	( cd ${1}; \
          git clean -fdx; \
	  cd ${1}/frontend/gtkmm/win32/setup/; \
	  ${WIN32_MAKERUNTIME} ${WORKRAVE_GIT_DIR}; )

workrave-win32-build-original=\
	( . ${HOME}/bin/vcvars32.sh; \
          cd ${1}/common/win32/harpoon/src; \
          mkdir Release; \
	  ${WIN32_NMAKE} /f harpoon.mak CFG="harpoon - Win32 Release"; \
          cd ${1}/frontend/win32/applet/src; \
          mkdir Release; \
          ${WIN32_NMAKE} /f Applet.mak CFG="Applet - Win32 Release" ); \
	( cd ${1}; \
	  . ${WIN32_MINGW_ENV}; \
	  ./autogencross.sh ${CONF_FLAGS} ${2} && make WINDRES=$$WINDRES ; \
	   $$NM -nosC ${1}/frontend/gtkmm/src/workrave.exe >${1}/frontend/gtkmm/src/workrave.sym )

workrave-win32-build=\
	( cd ${1}; \
          . ${HOME}/bin/vcvars32.sh; \
          cd ${1}/common/win32/harpoon/src; \
          mkdir Release; \
	  ${WIN32_NMAKE} /f harpoon.mak CFG="harpoon - Win32 Release"; \
          cd ${1}/frontend/win32/applet/src; \
          mkdir Release; \
	  cp -a ${TINDERBOX_HOME}/prebuilt/Release/workrave-applet.dll Release; \
	  cp -a ${TINDERBOX_HOME}/prebuilt/Release64/workrave-applet.dll Release/workrave-applet64.dll ); \
	( cd ${1}; \
	  . ${WIN32_MINGW_ENV}; \
	  ./autogencross.sh ${CONF_FLAGS} ${2} && make WINDRES=$$WINDRES ; \
	  cp -a ${1}/frontend/gtkmm/src/.libs/workrave.exe ${1}/frontend/gtkmm/src; \
	  $$NM -nosC ${1}/frontend/gtkmm/src/workrave.exe >${1}/frontend/gtkmm/src/workrave.sym )

workrave-win32-strip=\
	(  . ${WIN32_MINGW_ENV}; \
	$$STRIP ${1}/frontend/gtkmm/src/workrave.exe )

workrave-win32-setup=\
	( cd ${1}/frontend/gtkmm/win32/setup/; \
	  unix2dos setup.iss; \
	  wine "${WIN32_ISCC}" setup.iss ); \
	mv ${1}/frontend/gtkmm/win32/setup/Output/setup.exe ${TINDERBOX_BIN_DIR}/workrave-win32-${WORKRAVE_TAG}-${BUILD_DATE}-${2}.exe; \
	gzip -c ${1}/frontend/gtkmm/src/workrave.sym >${TINDERBOX_BIN_DIR}/workrave-win32-${WORKRAVE_TAG}-${BUILD_DATE}-${2}.sym.gz


workrave-win32-dir: 
	$(call workrave-win32-dir,${WORKRAVE_GIT_DIR})

workrave-win32-build: workrave-win32-dir
	$(call workrave-win32-build,${WORKRAVE_GIT_DIR},--disable-debug)
	$(call workrave-win32-strip,${WORKRAVE_GIT_DIR})

workrave-win32-setup: workrave-win32-build
	$(call workrave-win32-setup,${WORKRAVE_GIT_DIR},installer)

workrave-win32-debug-dir: 
	$(call workrave-win32-dir,${WORKRAVE_GIT_DIR})

workrave-win32-debug-build: workrave-win32-debug-dir
	$(call workrave-win32-build,${WORKRAVE_GIT_DIR},--enable-debug)
	( cd ${WORKRAVE_GIT_DIR}/frontend/gtkmm/src; \
	  zip ${TINDERBOX_BIN_DIR}/workrave-win32-${WORKRAVE_TAG}-${BUILD_DATE}-debug.zip workrave.exe )

workrave-win32-all: \
		workrave-win32-setup \
		workrave-win32-debug-build 

######################################################################
# Source
######################################################################

workrave-source: 
	( cd ${WORKRAVE_GIT_DIR}; \
	  ./autogen.sh --disable-gnome \
	  && make dist \
	  && mv ${WORKRAVE_GIT_DIR}/workrave-${WORKRAVE_VERSION}.tar.gz ${TINDERBOX_BIN_DIR}/workrave-src-${WORKRAVE_TAG}-${BUILD_DATE}.tar.gz )


######################################################################
# Debian
######################################################################

workrave-deb: 
	chmod u+x ${WORKRAVE_GIT_DIR}/debian/rules
	( cd ${WORKRAVE_GIT_DIR}; \
	  dch --ignore-dirname -v ${WORKRAVE_VERSION}+git${BUILD_DATE}-1 "Nighly snapshot"; \
	  ./autogen.sh \
          && dpkg-buildpackage -rfakeroot )
	mv ${WORKRAVE_GIT_DIR}/../workrave_*_i386.deb ${TINDERBOX_BIN_DIR}/

## workrave-${BUILD_DATE}_i386.deb

######################################################################
# Upload
######################################################################
upload-purge:
	for i in `find ${TINDERBOX_SNAPSHOTS_DIR} -type d -mtime +5 -print`; \
	do \
		echo Purging old snapshot: $$i; \
		rm -rf $$i; \
	done

upload:
	( cd ${TINDERBOX_SNAPSHOTS_DIR} ; \
          rsync -rv --delete --rsh="ssh -i/home/robc/.ssh/miss_workrave_id_dsa -l miss-workrave,workrave" . miss-workrave,workrave@shell.sourceforge.net:/home/users/m/mi/miss-workrave/public_html/download/snapshots )

shell:
	ssh -i/home/robc/.ssh/miss_workrave_id_dsa -t miss-workrave,workrave@shell.sourceforge.net create < /dev/null

#	sitecopy --update snapshots
	  
