/*
 User Extensions
*/

// Definitions




/*
 *   File search using '*'-type patterns
*/
int fillpattern(char *mask, char *pattern)
{
    int i = 0 ;
    if(*mask==0) return -1 ;
    while((*mask!=0)&&(*mask!='*'))
    {
        *pattern++ = *mask++ ;
        i++ ;
    }
    *pattern = 0 ;

    return i ;
}

int findpattern(char *pattern, char *name)
{
    int i = 0 , lenp, lenn ;
    if(*pattern==0) return -1 ;
    lenp = strlen(pattern) ;
    lenn = strlen(name) ;
    while(lenp<=lenn)
    {
        if(strncmp(name,pattern,lenp)==0) return i;
        name++ ;
        lenn-- ;
        i++ ;
    }
    return -1 ;
}




int selection(char *name, char *filemask )
{
    char file_pattern[256] ;
    int i;
    int imaskpos = 0, inamepos = 0 ;
    i = fillpattern(filemask, file_pattern);
    if(i==-1) return -1 ;

    if(i>0)
    {
        if(strncmp(file_pattern,name,i)!=0) return 0 ;
    }

    imaskpos += i+1 ; // next position after '*'
    inamepos += i ;
    do{
        // take mask next fragment between '*' symbols
        i = fillpattern(&filemask[imaskpos], file_pattern);  
        if(i == -1 ) return 1 ;
     
        int k = findpattern(file_pattern, &name[inamepos]);
        if(k==-1) return 0 ;
        imaskpos += i ;
        inamepos += k ;
        if(filemask[imaskpos] == '*') imaskpos++ ;
        else
            if(name[inamepos+i] != 0) return 0 ;  
            // the end of pattern but not end of name
        
    }while(1) ;
return 0;
}


/*
  (directory [pattern])
  Returns a list of the filenames of the files on the SD card.
  Pattern is string which contains '*' symbols.
*/
//  (directory "/home/*/")  - search directories 
//  (directory "/home/*")  ("/home/*.*") ("/home/*.txt") search files 

object *fn_directory (object *args, object *env) {
  (void) env;
#if defined(sdcardsupport)
  int type = 0x4 | 0x8 ;  // Files and directories
  char pattern_string[256] = "*" ;
  char dirname_string[256] = "/";

  if (args != NULL)
  {   //  Directory name
      if(stringp(car(args)))
      {
        cstring(car(args), dirname_string, 256) ;
        if(dirname_string[strlen(dirname_string)-1] == '/')
        {
            dirname_string[strlen(dirname_string)-1] = 0x0 ;
            type = 0x4 ;
        }
        else type = 0x8 ;

        char *pattern_bgn = strchr(dirname_string,'*') ;
        if(!pattern_bgn)
           pattern_bgn = &dirname_string[strlen(dirname_string)-1] ;
       
        while((pattern_bgn!=dirname_string)&&(*pattern_bgn!='/')) pattern_bgn -- ;
        if(*pattern_bgn=='/')
        {
            pattern_bgn ++ ;
            strcpy(pattern_string, pattern_bgn);
            *pattern_bgn = 0x0 ; // set 0x00 into dirname_string 
        }
        else
        {
            strcpy(pattern_string, pattern_bgn);
            strcpy(dirname_string, "/"); 
        }
            
        if(!(*dirname_string))
                strcpy(dirname_string, "/"); // Dir name "/" restore
      }
      else {
        error("argument must be string",car(args));
        return nil;
      }
  }

  object *result = cons(NULL, NULL);
  object *ptr = result;

  SDBegin();
  File root = SD.open(dirname_string);
  if (!root){  error("cannot open directory", car(args)); return nil; }

  while (true) {
      File entry = root.openNextFile();
      if(!entry) break;

      if( (entry.isDirectory() && (type&0x4)) || (!entry.isDirectory() && (type&0x8)) )
         if(selection((char*)entry.name(), pattern_string ))
      {
        object *filename = lispstring((char*)entry.name());
        cdr(ptr) = cons(filename, NULL);
        ptr = cdr(ptr);
      }
  };

  root.close();

  return cdr(result);
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}


/* Insert '/' symbol into begin of filename if it is absent.
"name" => "/name",  "home/name" => "/home/name"
*/
void test_filename(char *name)
{
  int len, i ;
  char *cPtr ;
  if(name[0] == '/' ) return ;
  len = strlen(name) ;
  cPtr = &name[len] ;
  *(cPtr+1) = 0 ;

  for(i=0;i<len;i++) {
      *cPtr= *(cPtr-1) ;
      cPtr -- ;  
  }

  name[0] = '/' ;
}



/*(probe-file pathspec)  tests whether a file exists.
Returns nil if there is no file named pathspec,
and otherwise returns the truename of pathspec.
*/
object *fn_probefile (object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char pattern_string[256] ;
  int findDir = 0 ;

  if(stringp(car(args))) cstring(car(args), pattern_string, 256) ;
  else {  error("argument must be string", car(args)); return nil; }
 
  if(pattern_string[strlen(pattern_string)-1] == '/') {
    pattern_string[strlen(pattern_string)-1] = 0x0 ;
    findDir = 1 ;
  }

  test_filename(pattern_string) ;

  SDBegin();
  if(SD.exists(pattern_string)) {
    File entry = SD.open(pattern_string) ;
    if( (entry.isDirectory()) && (findDir)) {
      entry.close();
      return car(args);
    }
    else if( (!entry.isDirectory() )&& (!findDir)) {
      entry.close();
      return car(args);
    }
  }

  return nil;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}


/* (delete-file pathspec)   delete specified file.
Returns true if success and otherwise returns nil.
*/
object *fn_deletefile (object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char pattern_string[256] ;

  if(stringp(car(args))) cstring(car(args), pattern_string, 256) ;
  else {  error("argument must be string", car(args)); return nil; }

  test_filename(pattern_string) ;

  SDBegin();
  if(SD.exists(pattern_string))
  {
    if(SD.remove(pattern_string)) return tee;
    else return nil;
  }
 
  return tee;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}


/* (delete-dir pathspec)   delete specified directory.
Returns true if success and otherwise returns nil.
*/
object *fn_deletedir (object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char pattern_string[256] ;

  if(stringp(car(args))) cstring(car(args), pattern_string, 256) ;
  else {  error("argument must be string", car(args)); return nil; }

  test_filename(pattern_string) ;

  SDBegin();
  if(SD.exists(pattern_string))
  {
     if(SD.rmdir(pattern_string)) return tee;
     else return nil;
  }
  
  return tee;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}


/* (rename-file pathspec newfile)  rename or moving specified file.
Returns true if success and otherwise returns nil.
*/
object *fn_renamefile (object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char filename_string[256] ;
  char newname_string[256] ;
  object *firstarg = car(args);

  if(stringp(car(args))) cstring(car(args), filename_string, 256) ;
  else  {  error("first argument must be string", car(args)); return nil; }

  args = cdr(args);
  
  if(stringp(car(args)))
    cstring(car(args), newname_string, 256) ;
  else  {  error("second argument must be string", car(args)); return nil; }

  test_filename(filename_string) ;
  test_filename(newname_string) ;

  SDBegin();
  if (!SD.exists(filename_string)) {  error("File not exists", firstarg); return nil; }
  

  File fp_source = SD.open(filename_string, FILE_READ);
  if (fp_source.isDirectory()) { 
      fp_source.close() ;
      error("argument must be a file", firstarg); return nil; }

  if (SD.exists(newname_string)) SD.remove(newname_string) ;
  File fp_dest = SD.open(newname_string, FILE_WRITE);
  if (!fp_dest) {  error("cannot open destination file", car(args)); return nil; }

  uint32_t i, sz ;
  sz = fp_source.size();

  for(i=0; i<sz;i++) fp_dest.write(fp_source.read()) ;

  fp_source.close();
  fp_dest.close();
  SD.remove(filename_string) ;

  return tee;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}


/* (copy-file pathspec newfile)  copy specified file.
Returns true if success and otherwise returns nil.
*/
object *fn_copyfile (object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char filename_string[256] ;
  char newname_string[256] ;
  object *firstarg = car(args);

  if(stringp(car(args))) cstring(car(args), filename_string, 256) ;
  else  {  error("first argument must be string", car(args)); return nil; }

  args = cdr(args);

  if(stringp(car(args)))
    cstring(car(args), newname_string, 256) ;
  else  {  error("second argument must be string", car(args)); return nil; }

  test_filename(filename_string) ;
  test_filename(newname_string) ;

  SDBegin();
  if (!SD.exists(filename_string)) {  error("File not exists", firstarg); return nil; }

  File fp_source = SD.open(filename_string, FILE_READ);
  if (fp_source.isDirectory()) { 
      fp_source.close() ;
      error("argument must be a file", firstarg); return nil; }

  if (SD.exists(newname_string)) SD.remove(newname_string) ;
  File fp_dest = SD.open(newname_string,FILE_WRITE);
  if (!fp_dest) {  error("cannot open destination file", car(args)); return nil; }

  uint16_t i, sz ;
  sz = fp_source.size();

  for(i=0; i<sz;i++) fp_dest.write(fp_source.read()) ;

  fp_source.close();
  fp_dest.close();

  return tee;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}



/* (ensure-directories-exist pathspec)   Tests whether the specified
directories actually exist, and attempts to create them if they do not.
Returns true if success and otherwise returns nil.
*/
object *fn_ensuredirectoriesexist(object *args, object *env) {
#if defined(sdcardsupport)
  (void) env;
  char pattern_string[256] ;
 
  if(stringp(car(args))) cstring(car(args), pattern_string, 256) ;
  else  {  error("argument must be string", car(args)); return nil; }

  test_filename(pattern_string) ;

  SDBegin();
  if(!SD.exists(pattern_string))
  {
    if(SD.mkdir(pattern_string)) return tee;
  }
  else return tee;

  return nil;
#else
  (void) args, (void) env;
  error2("not supported");
  return nil;
#endif
}





// Symbol names

const char string_probefile[] PROGMEM = "probe-file";
const char string_deletefile[] PROGMEM = "delete-file";
const char string_deletedir[] PROGMEM = "delete-dir";
const char string_renamefile[] PROGMEM = "rename-file";
const char string_copyfile[] PROGMEM = "copy-file";
const char string_ensuredirectoriesexist[] PROGMEM = "ensure-directories-exist";



// Documentation strings

const char doc_probefile[] PROGMEM = "(probe-file pathspec)\n"
"tests whether a file exists.\n"
" Returns nil if there is no file named pathspec,"
" and otherwise returns the truename of pathspec.";

const char doc_deletefile[] PROGMEM = "(delete-file pathspec)\n"
"delete specified file.\n"
" Returns true if success and otherwise returns nil.";

const char doc_deletedir[] PROGMEM = "(delete-dir pathspec)\n"
"delete specified directory.\n"
" Returns true if success and otherwise returns nil.";

const char doc_renamefile[] PROGMEM = "(rename-file pathspec newfile)\n"
"rename or moving specified file.\n"
" Returns true if success and otherwise returns nil.";

const char doc_copyfile[] PROGMEM = "(copy-file pathspec newfile)\n"
"copy specified file.\n"
" Returns true if success and otherwise returns nil.";

const char doc_ensuredirectoriesexist[] PROGMEM = "(ensure-directories-exist pathspec)\n"
"Tests whether the specified directories actually exist,"
" and attempts to create them if they do not.\n"
" Returns true if success and otherwise returns nil.";



// Symbol lookup table
const tbl_entry_t lookup_table2[] PROGMEM  = {
    { string_probefile, fn_probefile, 0211, doc_probefile },
    { string_renamefile, fn_renamefile, 0222, doc_renamefile },
    { string_copyfile, fn_copyfile, 0222, doc_copyfile },
    { string_deletefile, fn_deletefile, 0211, doc_deletefile },
    { string_ensuredirectoriesexist, fn_ensuredirectoriesexist, 0211, doc_ensuredirectoriesexist },
    { string_deletedir, fn_deletedir, 0211, doc_deletedir },
};




// Table cross-reference functions

tbl_entry_t *tables[] = {lookup_table, lookup_table2};
const unsigned int tablesizes[] PROGMEM = { arraysize(lookup_table), arraysize(lookup_table2) };

const tbl_entry_t *table (int n) {
  return tables[n];
}

unsigned int tablesize (int n) {
  return tablesizes[n];
}
