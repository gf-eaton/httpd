
cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("httpd")
add_executable (httpd "httpd.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET httpd PROPERTY CXX_STANDARD 20)
endif()

#set_property(TARGET httpd PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#target_compile_options(httpd PRIVATE /MTd) #/MT release
#target_link_options(httpd PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)

find_package(libhv REQUIRED)
find_package(libpqxx REQUIRED)
target_link_libraries(httpd PRIVATE hv libpqxx::pqxx)
