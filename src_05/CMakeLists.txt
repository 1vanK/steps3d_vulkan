source_group("" FILES ${log_source_files})
source_group("" FILES ${data_source_files})

set(target_name base-8)
set(source_files base-8.cpp)
add_executable(${target_name} ${source_files} ${log_source_files} ${data_source_files})
target_link_libraries(${target_name} PRIVATE glm stb Vulkan::Vulkan)
target_include_directories(${target_name} PRIVATE ../common)
source_group("" FILES ${source_files})
