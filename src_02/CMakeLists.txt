source_group("" FILES ${log_source_files})

set(target_name base-3)
set(source_files base-3.cpp)
add_executable(${target_name} ${source_files} ${log_source_files})
target_link_libraries(${target_name} PRIVATE glm Vulkan::Vulkan)
target_include_directories(${target_name} PRIVATE ../common)
source_group("" FILES ${source_files})
