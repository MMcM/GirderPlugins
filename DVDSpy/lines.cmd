@rem $Header$
@echo off
setlocal
set CLASSPATH=d:\xalan-j_2_5_1\bin\xalan.jar;d:\xalan-j_2_5_1\bin\xercesImpl.jar
set XSLT=%~dp0%lines.xsl
java org.apache.xalan.xslt.Process -XSL "%XSLT%" -IN %1 -OUT %2 -PARAM lines %3
endlocal
