This contains a extension file for microlisp developed by David Johnson-Davies (http://www.ulisp.com),
for working with the file system. The file is intended for the Arduino environment for 
ESP32 processors with a connection to an SD card.



    (probe-file pathspec)  
tests whether a file exists. Returns nil if there is no file named pathspec, and otherwise returns pathspec. This is most near to Common Lisp standard command "probe-file" which returns the true name of pathspec. True name can be different from pathspec if it is symbolic link.

    (delete-file pathspec)  
delete specified file. Returns true if success and otherwise returns nil.

    (delete-dir pathspec)
delete specified directory.
Returns true if success and otherwise returns nil.

    (rename-file pathspec newfile)
rename or moving specified file. Returns true if success and otherwise returns nil.

    (copy-file pathspec newfile)   
Сopy specified file. Returns true if success and otherwise returns nil.

    (ensure-directories-exist pathspec)
Tests whether the directories containing the specified file actually exist,
and attempts to create them if they do not. Returns true if success and otherwise returns nil.



  There may be minor inaccuracies in the programs. They can only be found and corrected when used. Therefore, I ask you to report incorrect operation of functions.

Best regards.
Anatoly Shcherbakov.
molnija2@inbox.ru
