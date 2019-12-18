## tabstops = 4 ##
##############################################
# For WXWIDGETS SUPPORT, uncomment the following line:
#
#	WXWIDGETS := y
#
# Nothing more is required.
# You are allowed to activate Java and WXWidgets at the same time.
###############################################

##############################################
# For PLATFORM INDEPENDENT GUI SUPPORT uncomment ONE of these:
# (* not yet implemented *)
#	GUI := java
#	GUI := wxwidgets
#############################################

#############################################
# For ALTERNATIVE CONTEXTS SEMANTICS uncomment ONE of these (default is 1):
#	CTX := 1
#	CTX := 2
#	CTX := 3
#	CTX := 4
# Some of these alternatives correspond to internal projects and, therefore,
# are not available in the public distribution of CxProlog.
##############################################

###################################################
# For VALGRIND SUPPORT, uncomment the following line:
#	VALGRIND=y
# The end-used should not use this option. This is only for
# debugging CxProlog itself.
#####################################################

###############################################
# For ADDITIONAL object files, libraries and options concerned with
# YOUR PROLOG EXTENSIONS, define your own stuff here:
#
EXT_OBJ_DIR := src/ext
EXT_CFLAGS :=
EXT_DEFINES :=
EXT_LIBS :=
EXT_NAMES :=

#
# Example:
#	EXT_OBJ_DIR := src/obj/ext
#	EXT_CFLAGS := -I/usr/include
#	EXT_DEFINES := -DUSE_X=2 -DCOMPASS=1
#	EXT_LIBS := -L/usr/lib -lcrypt
#	EXT_NAMES := ext/file1 ext/file2
##########################################

#########################################
# The DIRECTORY where CxProlog is INSTALLED by "make install":
#
PREFIX := /usr/local
##########################################



##########################################
  ############                                                 #############
############   RESERVED AREA -  DON'T CHANGE ANYTHING BELOW!!!   #############
  ############                                                 #############
##################################################

##################################################
# Base configuration
###################################################

APP := cxprolog
VERSION := 0.98.2
CC := gcc
CPP := g++
LD := ld
CFLAGS := -Wall -Wextra -O1
ONLYCFLAGS := # -Wmissing-prototypes
DEFINES :=
LIBS := -lm
OBJ_DIR := src/obj
TMP_DIR := /var/tmp/$(APP)
DEBUG :=
NAMES:=	Alias Array Arith Atom Attention Boot Buffer CallProlog		\
		Character Clause Clock CmdLine CodeGen Compiler				\
		Contexts1 Contexts2 Contexts3 Contexts4 Consult CxProlog	\
		Debug Dict Disassembler Exception Extra File FileSys Flags	\
		ForeignEvent GCollection Gui IVar Index Instructions		\
		InterLine Java JavaAddOns Locale Machine Memory Mesg Net	\
		Nil Null Number Operator OSUnix OSWin OSUnknown Predicate	\
		PredicateProperty Process Queue Scratch Scribbling Stack	\
		Stream StreamProperty String SysTrace Table TestLib Term	\
		TermRead TermWrite Thread Ucs2 Unify Unit Util Utf8 Utf16	\
		Utf32 VarDictionary Version WxWidgets WxWidgetsAddOns		\
		WxWidgetsDemo YourExtensions
OS := $(shell uname | cut -c 1-7)
EXTRA := -fPIC # -m32
SHELL := /bin/bash

###################################################
# Computed configuration
##################################################

define OBJS
	$(addprefix $(OBJ_DIR)/,												\
		$(addsuffix .o, $(foreach file, $(NAMES) $(EXT_NAMES), $(file)))	\
	)
endef

ifneq ($(OS),MINGW32)
	ZPREFIX := -DPREFIX=\"$(PREFIX)\"
endif

ifeq ($(VERBOSE),y)
	CFLAGS := -v $(CFLAGS)
	LIBS := -v $(LIBS)
endif

ZFLAGS := $(DEBUG) $(EXT_CFLAGS)
ZDEFINES := $(EXTRA) $(ZPREFIX) -DOS=$(OS) $(DEFINES) $(EXT_DEFINES)
ZLIBS := $(EXTRA) $(LIBS) $(EXT_LIBS)
ZPRE :=

ifeq ($(READLINE),n)
else
  ifneq ($(OS),MINGW32)
	ifeq ($(shell if test -e "/usr/local/opt/readline"; then echo "y" ; fi),y)
		ZDEFINES += -DUSE_READLINE=4 -I/usr/local/opt/readline/include
		ZLIBS += -L/usr/local/opt/readline/lib -lreadline -lncurses
	else ifeq ($(shell if test -e "/opt/local/include/readline"; then echo "y" ; fi),y)
		ZDEFINES += -DUSE_READLINE=3 -I/opt/local/include
		ZLIBS += -L/opt/local/lib -lreadline -lncurses
	else ifeq ($(shell if test -e "/usr/local/include/readline"; then echo "y" ; fi),y)
		ZDEFINES += -DUSE_READLINE=2
		ZLIBS += -L/usr/local/lib -lreadline -lncurses
	else ifeq ($(shell if test -e "/usr/include/readline"; then echo "y" ; fi),y)
		ZDEFINES += -DUSE_READLINE=1
		ZLIBS += -lreadline -lncurses
	else
		ZPRE +=																\
		 echo "**********************************************************";	\
		 echo " Missing READLINE support! Install the libraries yourself!";	\
		 echo "  Ubuntu ->   sudo apt-get install libreadline-dev libncurses-dev";	\
		 echo "  MacPorts -> sudo port install readline";					\
		 echo "  Homebrew -> sudo brew install readline";					\
		 echo "**********************************************************";
	endif
# Include search path: -Idirs, /usr/local/include, /usr/include
# Library search path: -Ldirs, standard
# Check libs used: /sbin/ldconfig -p | grep readline
# Check libs used: ldd cxprolog / otool -L cxprolog
# http://bhami.com/rosetta.html
  endif
endif

ifeq ($(JAVA),y)
  ifeq ($(OS),MINGW32)
	ZDEFINES += -DUSE_JAVA $(shell src/java-config-mingw --cxxflags)
	ZLIBS += $(shell src/java-config-mingw --libs)

  else # Assume Unix
	ZDEFINES += -DUSE_JAVA $(shell src/java-config --cxxflags)
	ZLIBS += $(shell src/java-config --libs)
  endif
	
	ZPRE +=																	\
	 if ! type javac >/dev/null 2>&1 ; then									\
	  echo "\n";															\
	  echo "*************************************************************";	\
	  echo "Missing JAVA dependency:";										\
	  echo "  The Java Development Toolkit of Sun is missing!!!";			\
	  echo "  The JDK is required to compile CxProlog with JAVA support.";	\
	  echo "  On Debian-based systems is enough to issue a command like:";	\
	  echo "        sudo apt-get install sun-java5-jdk";					\
	  echo "  On other sys you must downld and install a package from:";	\
	  echo "        http://www.java.com/en/download/";						\
	  echo "*************************************************************";	\
	  echo "\n";															\
	  exit 1;																\
	 fi;
	ZPOST_disable +=														\
	  echo "\n";															\
	  echo "*************************************************************";	\
	  echo "Mandatory manual post-installation for JAVA:";					\
	  echo "    - Run 'make install' to ensure that the Java classes of";	\
	  echo "      CxProlog library are where CxProlog expects them to be.";	\
	  echo "*************************************************************";	\
	  echo "\n";
endif

ifeq ($(WXWIDGETS),y)
  ifeq ($(OS),MINGW32)
	ZDEFINES += -DUSE_WXWIDGETS												\
				-mthreads -pipe -fmessage-length=0							\
				-DHAVE_W32API_H -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE	\
				-IC:/SourceCode/Libraries/wxWidgets2.8/lib/gcc_dll/mswu		\
				-IC:/SourceCode/Libraries/wxWidgets2.8/include
	ZLIBS += -enable-auto-import -export-all-symbols -mthreads				\
				-LC:/SourceCode/Libraries/wxWidgets2.8/lib/gcc_dll -lwxmsw28u

  else # Assume Unix
	ZDEFINES += -DUSE_WXWIDGETS $(shell wx-config --unicode --cxxflags)
	ZLIBS += $(shell wx-config --unicode --libs)
	ZPRE +=																	\
	 if ! type wx-config >/dev/null 2>&1									\
	 		|| [[ $(shell wx-config --release) < "2.8" ]]  ; then			\
	  echo "\n";															\
	  echo "*************************************************************"; \
	  echo "Missing WXWIDGETS dependency:";									\
	  echo "  The developement library \"libwxgtk2.8-dev\" is missing!!!";	\
	  echo "  For WXWIDGETS support, you need to install it yourself.";		\
	  echo "  In Debian-based systems it is enough to issue the command:";	\
	  echo "        sudo apt-get install libwxgtk2.8-dev libjpeg62-dev";	\
	  echo "*************************************************************";	\
	  echo "\n";															\
	  exit 1;																\
	 fi;
  endif
endif

ifeq ($(STATIC),y)
	ZLIBS := -static $(ZLIBS) -ltermcap
endif

ifeq ($(GUI),java)
	ZDEFINES += -DGUI_JAVA
endif
ifeq ($(GUI),wxwidgets)
	ZDEFINES += -DGUI_WXWIDGETS
endif

ifdef CTX
	ZDEFINES += -DCONTEXTS=$(CTX)
endif

ifdef COMPASS
	ZDEFINES += -DCOMPASS=$(COMPASS)
endif

ifeq ($(VALGRIND),y)
	ZDEFINES += -DUSE_VALGRIND
	ZFLAGS += -O0 -g
else
	ZFLAGS += $(CFLAGS)
endif


##############################################################################
# Build CxProlog as an application
##############################################################################

$(APP): $(OBJS)
	$(CPP) -o $(TMP_DIR)/$(APP) $(OBJS) $(ZLIBS)
	ln -sf $(TMP_DIR)/$(APP) $(APP)
	@$(ZPOST)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(ZFLAGS) $(ZDEFINES) $(ONLYCFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: src/%.cpp
	$(CPP) $(ZFLAGS) $(ZDEFINES) -c $< -o $@

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR):
	@$(ZPRE)
	mkdir -p $(TMP_DIR)/$(OBJ_DIR) $(TMP_DIR)/$(EXT_OBJ_DIR)
	ln -sf $(TMP_DIR)/$(OBJ_DIR) $(OBJ_DIR)
	ln -sf $(TMP_DIR)/$(EXT_OBJ_DIR) $(EXT_OBJ_DIR)


##############################################################################
# Build CxProlog as a shared library
##############################################################################

ifeq ($(OS),MINGW32)
  libcxprolog.a cxprolog.dll: $(OBJ_DIR) $(OBJS)
	$(CPP) -shared -o cxprolog.dll $(OBJS) -Wl,--out-implib,libcxprolog.a $(ZLIBS)
	
  cxprolog_shared.exe: libcxprolog.a cxprolog.dll
  ifeq ($(WXWIDGETS),y)
	$(CPP) -o cxprolog_shared.exe -lmingw32 -L./ -lcxprolog
  else
	$(CPP) -o cxprolog_shared.exe -L./ -lcxprolog
  endif
	
  cxprolog_testlib.exe: libcxprolog.a cxprolog.dll
	gcc -DTESTLIB=1 -o src/TestLib.o -c src/TestLib.c
	$(CPP) -enable-auto-import -o cxprolog_testlib src/TestLib.o -L./ -lcxprolog
	rm src/TestLib.o
	
lib: libcxprolog.a cxprolog_shared.exe cxprolog_testlib.exe

else # Assume Unix
  libcxprolog.so: $(OBJ_DIR) $(OBJS)
	$(CPP) -shared -o libcxprolog.so $(OBJS) -lc $(ZLIBS)

  cxprolog_shared: libcxprolog.so
	$(CPP) $(ZDEFINES) -o cxprolog_shared -L./ -lcxprolog

  cxprolog_testlib: libcxprolog.so
	gcc $(ZDEFINES) -DTESTLIB=1 -o src/TestLib.o -c src/TestLib.c
	$(CPP) $(ZDEFINES) -o cxprolog_testlib src/TestLib.o -L./ -lcxprolog
	rm src/TestLib.o

lib: libcxprolog.so cxprolog_shared cxprolog_testlib

run_lib: lib
	LD_LIBRARY_PATH=. cxprolog_shared

test_lib: lib
	LD_LIBRARY_PATH=. cxprolog_testlib
	
test_java_call: lib
	LD_LIBRARY_PATH=. java -classpath lib/cxprolog/java/prolog.jar prolog.Prolog
# -Djava.library.path=.

endif


##############################################################################
# Utilities
##############################################################################

all: $(APP) lib java

.PHONY : clean
clean:
	rm -f $(APP)
	rm -rf $(TMP_DIR) $(OBJ_DIR) $(EXT_OBJ_DIR) src/*.o core pl/CxCore.pl
	rm -f acconfig.h acinclude.m4 autogen.sh setup-gettext stamp-h.in
	find . -type f \( -name '*.bak' -o -name '*.class' -o -size 0 \) -exec rm -f {} \;
 
.PHONY : run
run: $(APP)
	./$(APP)

.PHONY : test
test: $(APP)
	./$(APP) --test

.PHONY : check
check: $(APP)
	./$(APP) --test

.PHONY : install
install: $(APP)
	rm -rf $(PREFIX)/lib/$(APP) $(PREFIX)/share/$(APP) $(PREFIX)/bin/$(APP)
	mkdir -p $(PREFIX)/bin/ $(PREFIX)/lib/$(APP) $(PREFIX)/share/$(APP)
	cp $(APP) $(PREFIX)/bin/
	cp -pR lib/$(APP)/* $(PREFIX)/lib/$(APP)
	cp -pR examples pl ChangeLog.txt COPYING.txt INSTALL.txt \
          INSTALL-windows.txt MANUAL.txt README.txt $(PREFIX)/share/$(APP)
ifeq ($(OS),MINGW32)
	cp -a wxmsw28u_gcc.dll $(PREFIX)/bin/
endif

.PHONY : uninstall
uninstall:
	rm -rf $(PREFIX)/lib/cxprolog $(PREFIX)/share/cxprolog $(PREFIX)/bin/$(APP)
  ifeq ($(OS),MINGW32)
	rm -f $(PREFIX)/bin/$(APP).exe $(PREFIX)/bin/wxmsw28u_gcc.dll
  endif

.PHONY : installlinks
installlinks: $(APP)
	rm -rf $(PREFIX)/lib/$(APP) $(PREFIX)/share/$(APP) $(PREFIX)/bin/$(APP)
	ln -sT $(CURDIR)/$(APP) $(PREFIX)/bin/$(APP)
	ln -sT $(CURDIR)/lib/$(APP) $(PREFIX)/lib/$(APP)
	ln -sT $(CURDIR) $(PREFIX)/share/$(APP)

.PHONY : dist
DIST := $(APP)-$(VERSION)
dist: clean
	rm -rf $(DIST)
	mkdir -p $(DIST)
	cp -aL ChangeLog.txt COPYING.txt MANUAL.txt README.txt $(DIST)
	cp -aL lib examples pl $(DIST)
  ifeq ($(OS),MINGW32)
		echo "MINGW32"
  else
		cp -aL INSTALL.txt INSTALL-windows.txt Makefile src $(DIST)
		rm -rf $(DIST)/src/.[a-z]* $(DIST)/src/CVS $(DIST)/src/obj
		cp -a zobfusc/* $(DIST)/src
		tar cz $(DIST) > $(DIST).src.tgz
		rm -rf $(DIST)
		cp -a $(DIST).src.tgz ~/www/cxprolog/cxunix
		mkdir -p ../CxVersions ; mv $(DIST).src.tgz ../CxVersions
		cp -aL ChangeLog.txt MANUAL.txt INSTALL.txt INSTALL-windows.txt ~/www/cxprolog
  endif

.PHONY : java
java:
	rm -rf $(PREFIX)/lib/cxprolog/java/prolog/*.class
	javac -Xlint:unchecked $(PREFIX)/lib/cxprolog/java/prolog/*.java
	cd $(PREFIX)/lib/cxprolog/java; jar cf prolog.jar prolog/*.class
	rm -rf $(PREFIX)/lib/cxprolog/java/prolog/*.class

.PHONY : distzip
distzip: clean
	cd ..; zip -r $(APP)-$(VERSION).zip $(APP)-$(VERSION) > /dev/null

.PHONY : distdll
distdll:
	rm -rf cx_headers ; mkdir cx_headers
	cp -a src/*.h cx_headers

.PHONY : wx
wx:
	curl http://apt.wxwidgets.org/key.asc | sudo apt-key add -
	#deb http://apt.wxwidgets.org/ DIST-wx main
	#deb-src http://apt.wxwidgets.org/ DIST-wx main
	sudo apt-get update
	sudo apt-get install wx2.8-i18n wx2.8-i18n libwxgtk2.8-dev libgtk2.0-dev

.PHONY : full
full:
	$(MAKE) WXWIDGETS=y JAVA=y $(APP)

.PHONY : base
base:
	make all

.PHONY : link
link: $(APP) uninstall
	cd $(PREFIX)/bin ; ln -s /home/amd/development/CxProlog/$(APP)
	cd $(PREFIX)/lib ; ln -s /home/amd/development/CxProlog/lib/cxprolog

.PHONY : version
version:
	vi +/version src/CallProlog.c Makefile MANUAL.txt

.PHONY : newyear
newyear:
	sed -i -e "s/1990-20../1990-2016/" src/*.[hc] src/*.cpp src/java-config \
				pl/*.pl lib/cxprolog/java/prolog/*.java README.txt MANUAL.txt

.PHONY : gdb
gdb:
	gdb -batch -ex 'run bt' cxprolog


##############################################################################
# THE END
##############################################################################
