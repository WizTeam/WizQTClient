PROJECT(clucene-ext)

file(GLOB_RECURSE EXTHEADERS ${clucene-ext_SOURCE_DIR}/boost/*.hpp)

#install public headers.
FOREACH(file ${EXTHEADERS})
	get_filename_component(apath ${file} PATH)
	get_filename_component(aname ${file} NAME)
	file(RELATIVE_PATH relpath ${CMAKE_SOURCE_DIR}/src/ext/boost ${apath})
	
	install(FILES ${file} 
	       DESTINATION "include/CLucene/ext/boost/${relpath}"
               COMPONENT development)
ENDFOREACH(file)

