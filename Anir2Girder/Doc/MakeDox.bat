@echo off

@echo *** Removing old docs ***
rd /S /Q html

@echo ***Copying some files ***

SET VERSION=0.3.0

md ..\bin\html
copy ..\ReadMe.txt ..\bin\html
copy ..\NEWS.txt ..\bin\html
copy ..\TODO.txt ..\bin\html
copy ..\Changelog.txt ..\bin\html


@echo *** Generating docs ***
doxygen doxygen.cfg


@echo *** Done ***
@echo Warnings: (see Doxygen.log)
@echo Run "ReadDocs" in bin directory to view the docs
