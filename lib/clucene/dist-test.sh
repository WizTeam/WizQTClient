#!/bin/bash
#Check compliance with Coding standards...

#where to keep the temp files...
TMP=disttest

function usage {
    echo "usage: ../dist-test.sh [all | "
    echo "    <env - creates environment>"
    echo "    <c_all - compile all headers together>"
    echo "    <compile - compile and test>"
    echo "    <inline - test for inline using doxygen documents>]"
    echo "    <c_header - test that each header compiles independently of each other>]"
    echo "    <license - test that each header has a valid license>]"
    echo "    <ifdefs - test that each header doesn't have invalid ifdefs>]"
    echo "    <exports - test that each header exports all its classes>]"
    exit 1;
}
t_all=0
t_env=0
t_c_all=0
t_inline=0
t_compile=0
t_c_h=0
t_license=0
t_ifdefs=0
t_exports=0
FAIL=0

if [ $# -eq 0 ]; then
    usage
else
    while [ "$1" != "" ]; do
        if [ "$1" == "all" ]; then
            t_all=1
        elif [ "$1" == "env" ]; then
            t_env=1
        elif [ "$1" == "c_all" ]; then
            t_c_all=1
        elif [ "$1" == "inline" ]; then
            t_inline=1
        elif [ "$1" == "compile" ]; then
            t_compile=1
        elif [ "$1" == "c_header" ]; then
            t_c_h=1
        elif [ "$1" == "license" ]; then
            t_license=1
        elif [ "$1" == "ifdefs" ]; then
            t_ifdefs=1
        elif [ "$1" == "exports" ]; then
            t_exports=1
        else
            usage
        fi
        shift
    done
fi

if [ $t_all -eq 1 ]; then
    t_env=1
    t_c_all=1
    t_c_h=1
    t_inline=1
    t_compile=1
    t_license=1
    t_ifdefs=1
    t_exports=1
fi


#check to see that no #ifdefs exist in headers that don't belong
function checkForIfdefs {
    I=0
    grep "#if" $1| grep -v "_UCS2" |grep -v "_CL_HAVE_" |grep -v "_ASCII" |grep -v "_WIN32"|grep -v "_MSC_"|grep -v "__MINGW32__" |grep -v "_WIN64" | while read line; do
        I=`expr $I + 1`
        if [ $I -gt 1 ]; then
            echo $1 might have invalid ifdef: $line
        fi
    done
}


if [ $t_env -eq 1 ]; then
    rm -fdr $TMP 2>/dev/null
    mkdir $TMP
    
    #create header file for testing of symbols in headers.
    echo "#include \"CLucene/StdHeader.h"\" >$TMP/pub-headers.cpp

		#iterate all headers
    for H in `find ../src/shared/CLucene| grep "\.h$"` `find ../src/core/CLucene| grep "\.h$"`; do
        BH=`basename "$H"`
        DN=`dirname "$H"`
        if [ "${BH:0:1}" != "_" ]; then
            DH=`dirname "${H:3}"`
        
            if [ "${H:7}" != "core/CLucene/util/Reader.h" ]; then
	            #move headers somewhere to compile
	            mkdir -p "$TMP/$DH" 2>/dev/null
	            ln -s "`cd "$DN" && pwd`/$BH" "$TMP/${H:3}" 2>/dev/null
	            
	            #create pub-headers.cpp
              echo "#include \"${H:7}\"" >>$TMP/pub-headers.cpp
            fi
        fi
    done
    
    echo "int main(){return 0;}"  >>$TMP/pub-headers.cpp
fi


################################################
#now the environment is finished being setup...
################################################
echo "Starting tests..."

if [ $t_c_h -eq 1 ] || [ $t_ifdefs -eq 1 ] || [ $t_exports -eq 1 ]; then
    for H in `find $TMP/src | grep "\.h$"`; do
        BH=`basename "$H"`
        DH=`dirname "${H:3}"`
        
        if [ $t_ifdefs -eq 1 ]; then
            checkForIfdefs $H
        fi
    
        #check that all classes are exported
        if [ $t_exports -eq 1 ]; then
      		if [ "${H:0:1}" == "_" ]; then
      			#internal headers... none must be exported
	          XX=`awk '/^[ \t]*(class|struct)/ { print $line }' $H| grep -v ";$"| grep -v CLUCENE_EXPORT| grep -v CLUCENE_INLINE_EXPORT| grep -v CLUCENE_SHARED_EXPORT| grep -v CLUCENE_SHARED_INLINE_EXPORT`
	          if [ "$XX" == "" ]; then
	              echo "$H is internal but has exported class: $XX"
	              echo ""
	              FAIL=1
	          fi
          else
          	#external headers... all must be exported
	          XX=`awk '/^[ \t]*(class|struct)/ { print $line }' $H| grep -v ";$"| grep -v CLUCENE_EXPORT| grep -v CLUCENE_INLINE_EXPORT| grep -v CLUCENE_SHARED_EXPORT| grep -v CLUCENE_SHARED_INLINE_EXPORT`
	          if [ "$XX" != "" ]; then
	              echo "$H has unexported class: $XX"
	              echo ""
	              FAIL=1
	          fi
          fi
        fi
        
        #test that each header compiles independently...
        if [ $t_c_h -eq 1 ]; then
            echo "#include \"CLucene/StdHeader.h"\" >$TMP/pub-header.cpp
            echo "#include \"$H"\" >>$TMP/pub-header.cpp
            echo "int main(){ return 0; }" >>"$TMP/pub-header.cpp"
            ERROR=`g++ -I. -I$TMP/src/shared -I./src/shared -I../src/ext -I$TMP/src/core $TMP/pub-header.cpp`
            if [ $? -ne 0 ]; then 
              echo ""
            	echo "$H doesn't compile seperately..."
            	echo $ERROR
            	FAIL=1; 
            fi
        fi
    done
    
    
    if [ $t_ifdefs -eq 1 ]; then
      echo "Not all ifdefs are invalid, you have to figure it out for yourself :-)"
      echo "If defs in classes which change depending on a user setting can cause big problems due to offset changes"
      echo "for example:"
      echo "class X {"
      echo " #ifdef _DEBUG"
      echo "  int x;"
      echo " #endif"
      echo " int y;"
      echo "}"
      echo "If the library is compiled with _DEBUG, and then a user calls y without _DEBUG defined, unexpected behaviour will occur"
    fi
fi

#iterate all our code...
if [ $t_license -eq 1 ]; then
    for H in `find ../src`; do
        BH=`basename "$H"`
        BH_len=${#BH}
        
        if [ "${BH:BH_len-2}" == ".h" ] || [ "${BH:BH_len-2}" == ".c" ] || [ "${BH:BH_len-4}" == ".cpp" ]; then
		        
		        #snowball has its own license...
		        if [ "echo $H|grep 'snowball/src_c'" != "" ]; then
		        	continue
		        fi
		        #snowball has its own license...
		        if [ "echo $H|grep 'libstemmer'" != "" ]; then
		        	continue
		        fi
		        #zlib has its own license...
		        if [ "echo $H|grep 'CLucene/util/zlib'" != "" ]; then
		        	continue
		        fi
		        
            if [ "`awk '/\* Copyright \(C\) [0-9]*-[0-9]* .*$/ { print $line }' $H`" == "" ]; then
                if [ "`awk '/\* Copyright [0-9]*-[0-9]* .*$/ { print $line }' $H`" == "" ]; then
                    echo "$H ($BH) has invalid license"
                    FAIL=1
                fi
            fi
        fi
    done
fi


#test if headers can compile together by themselves:
if [ $t_c_all -eq 1 ]; then
    g++ -I$TMP/src -I../src/ext -I$TMP/src/shared -I$TMP/src/core $TMP/pub-headers.cpp -I./src/shared
fi

if [ $t_inline -eq 1 ]; then
		if [ ! -d "./doc" ]; then
			echo "Couldn't find docs, run:"
		  echo "# cmake -DENABLE_CLDOCS:BOOLEAN=TRUE ."
		  echo "# make doc"
		  echo "and then try again"
		  exit 1
		fi
	
    INLINES=0
    grep -c "\[inline" doc/html/*.html|grep -v ":0$"|grep -v "util"|grep -v "jstreams" | while read line; do
    
        #ignore some actual inlines...
        if [ "doc/html/classlucene_1_1index_1_1Term.html:1" == $line ]; then
            continue;
        fi
        if [ "doc/html/classlucene_1_1search_1_1Similarity.html:1" == $line ]; then
            continue;
        fi
        if [ "doc/html/classlucene_1_1store_1_1BufferedIndexInput.html:1" == $line ]; then
            continue;
        fi
        
        if [ $INLINES -eq 0 ]; then
            echo "These files report inline code:"
            INLINES=1
            FAIL=1
        fi
        echo $line
    done
fi

if [ $t_compile -eq 1 ]; then
    #compile serperately
    make cl_test
    if [ $? -ne 0 ]; then 
        FAIL=1; 
    fi
    
    echo "Undefines for shared lib:"
    nm -u --demangle bin/libclucene-shared.so |grep -E "lucene::"
    echo "Undefines for core lib:"
    nm -u --demangle bin/libclucene-core.so |grep -E "lucene::"|grep -v "lucene::util::Misc" |grep -v "lucene::util::mutex" |grep -v "lucene::util::StringBuffer"|grep -v "lucene::util::shared_condition"

    #compile together
    make test-all
    if [ $? -ne 0 ]; then 
        FAIL=1; 
    fi
    
    
fi


if [ $FAIL == 1 ]; then
    echo "There were errors, please correct them and re-run"
    exit 1
fi
exit 0
