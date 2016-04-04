Here is our project folder for our group composed of:
  > Jonathan Rozanes
  > Guillaume Catto
  > Thomas Fossati
  > Edouard Piette

LIST OF FILES  --------------------------------------------------------------------------

	The folder contains all the necessary files for you to test our program(s):
	  > The sws_qX.{cpp,h} files, for each question
	  > inet_socket.{cpp,h} files, from our classes, useful for some programs
	  > utility.{cpp,h} files, containing some functions used for questions {5..7}
	  > The makefile, where you can:
	    >> build all files (make all)
	    >> build a given file (make sws_qX)
	    >> clean the *.o files (make clean)
	    >> clean the *.o files and the built programs (make dist-clean)
	  > The report (report.pdf), written in French, for question 1

LINUX, COMPILER VERSIONS & CODING STYLE -------------------------------------------------

	> LINUX DISTRIBUTION
		>> cat /etc/*-release
			DISTRIB_ID=LinuxMint
			DISTRIB_RELEASE=17.3
			DISTRIB_CODENAME=rosa
			DISTRIB_DESCRIPTION="Linux Mint 17.3 Rosa"
			NAME="Ubuntu"
			VERSION="14.04.4 LTS, Trusty Tahr"
			ID=ubuntu
			ID_LIKE=debian
			PRETTY_NAME="Ubuntu 14.04.4 LTS"
			VERSION_ID="14.04"
			HOME_URL="http://www.ubuntu.com/"
			SUPPORT_URL="http://help.ubuntu.com/"
	> KERNEL VERSION
		>> uname -r
			3.19.0-32-generic
	> COMPILER VERSION
		>> g++ --version:
			g++ (Ubuntu 5.3.0-3ubuntu1~14.04) 5.3.0 20151204
	> CODING STYLE
		Our program(s) are written following the camelCase coding style

NOTES -----------------------------------------------------------------------------------

Please read the following notes:
	> sws_q7.cpp has been entirely refactored, using the 
	functions in inet_socket.cpp and utility.cpp
	> The programs DO NOT seems to work on the 5.3.1 (or 
		higher) version of g++
