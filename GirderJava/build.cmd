cd java
javac -g nl\girder\api\*.java
jar -cf ..\..\..\GirderJava.jar nl\girder\api\*.class
javah -d ..\jni nl.girder.api.GirderCommand nl.girder.api.GirderLink nl.girder.api.GirderSupport
javadoc -sourcepath . -d ..\javadocs nl.girder.api
cd ..
nmake /f "GirderJava.mak" CFG="GirderJava - Win32 Release"
