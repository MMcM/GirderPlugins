@rem $Header$
@echo off
setlocal
set XSLT=%~dp0%lines.xsl
java org.apache.xalan.xslt.Process -XSL "%XSLT%" -IN %1 -OUT %2 -PARAM lines %3
endlocal
