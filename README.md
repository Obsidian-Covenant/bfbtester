# bfbtester

## Modernization Covenant

This is a refactored and modernized edition of [bfbtester](https://sourceforge.net/projects/bfbtester/), a tool to perform checks of single and multiple argument command line overflows and environment variable overflows. 

Key changes:

- ✅ Fixed errors caused by old libraries
- ✅ Fixed warnings caused by old-style coding
- ✅ Cleaned autoreconf rules
- ✅ Refactored code to be used with latest gcc version

### Installation

#### Dependencies

Build dependencies:
```
gcc
```

#### From source

```bash
git clone https://github.com/Obsidian-Covenant/bfbtester.git
cd bfbtester
autoreconf -i
./configure
make
sudo make install
```

## README

README for BFBTester 2.0-BETA
Copyright 2000 Mike Heffner <mheffner@vt.edu>
http://bfbtester.sourceforge.net

-> I N T R O <-

BFBTester is great for doing quick, proactive, security checks of
binary programs. BFBTester will perform checks of single and multiple
argument command line overflows and environment variable
overflows. Versions 2.0-BETA and higher can also watch for tempfile
creation activity to alert the user of any programs using unsafe
tempfile names. While BFBTester can not test all overflows in
software, it is useful for detecting initial mistakes that can red
flag dangerous software.

-> U S A G E <-

The basic syntax for running BFBTester is:

bfbtester [-htv] [-d level ] [-r rejects] [-o out-file]
          [-x max-execs] -asme file [file2 file3 ...]
Must specify one or more of the following tests:
    -s  Single Argument Test
    -m  Multiple Argument Test
    -e  Environment Variable Test
    -a  Selects all tests
Options:
    -h  Print this message
    -t  Enable tempfile monitoring
    -v  Print version string
    -d level     Set debug level (default = 0, max = 2)
    -r rejects   Comma separated list of binaries to skip
    -o out-file  Output to out-file rather than stdout
    -x max-execs Set maximum executables to run in parallel (default = 250)
file    Specific binary or a directory of binaries to test


You must specify at least one test to run and you must specify either
a binary or a directory. 

Executable selection is now done in one of several ways:

If the executable filename is specified with a leading
slash (an absolute path), no selection is used and the
supplied absolute filename is used.

If there is no leading slash in the filename the selection
is made in one of two ways (in this order):
  1) Prepend file name with $PWD and test accesiblity
  2) Search through $PATH and find first accessible executable
The first one to succeed is the executable choosen.

If the filename found is a directory, we walk the directory (one
level deep) looking for executable binaries.

Symbolic links are followed.


You can specify binaries to skip (useful when loading a whole
directory) by using the -r option.

Examples:

bfbtester -s /usr/bin

	Run the single argument test on all binaries in folder 	/usr/bin.

bfbtester -ta patch traceroute

	Run all tests against patch and traceroute and run the tempfile
	monitor.

bfbtester -a ./bfbtester

	Tests bfbtester (provided it's in the same directory). Hope it works :)

bfbtester -r kill /usr/bin/kill

	Does nothing.


-> N O T E S <-

The following is a crash report:

*** Crash </usr/bin/patch> ***
 args:           -D [05120]
 envs:           (null)
 Signal:         11 ( Segmentation fault )
 Core?           Yes

This means "/usr/bin/patch" crashed when fed with an "-D" and a word
5,120 characters long:

$ /usr/bin/patch -D AAA...5,120 characters...AAA

(Numbers in brackets mean replace with a word that many characters
long)

ALSO: BFBTester is very CPU intensive, and will open many files, so
you probably don't want to run it on a production machine during it's
busiest period. Just a warning...


-> K N O W N   B U G S <-

	[6/04/2000] - Been known to blow up on OpenBSD (pthread related).
	Workaround is to define LINUX in src/config.h.

-> T O D O <-



-> T H A N K S <-

To...
	o Thomas Stromberg <tstromberg@rtci.com> for developing
	  SmashWidgets, from where I got the idea for BFBTester
	o Compaq for their great TestDrive servers at:
	  http://www.testdrive.compaq.com/
	o David Wheeler <dwheeler@ida.org> for suggesting I should put
	  together some type of license and for listing BFBTester in the "Secure
	  Programming for Linux and Unix HOWTO" available at:
	  http://www.dwheeler.com/secure-programs/
	o Bill Marquette <wlmarque@hewitt.com> for testing on Solaris and
	  contributing patches
