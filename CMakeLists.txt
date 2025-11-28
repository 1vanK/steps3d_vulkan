cmake_minimum_required(VERSION 3.16)

project(steps3d_vulkan)

# Если используется одноконфигурационный генератор
# и конфигурация не указана
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    # то конфигурацией по умолчанию будет Release
    set(CMAKE_BUILD_TYPE Release)
    # Нельзя оставлять переменную CMAKE_BUILD_TYPE пустой,
    # так как при этом не будут заданы флаги GCC и MinGW
endif()

# Предупреждаем об in-source build
if(CMAKE_BINARY_DIR MATCHES "^${CMAKE_SOURCE_DIR}")
    message(WARNING "Генерировать проекты в папке с иходниками - плохая идея")
endif()

# --------- Сторонние библиотеки ---------

find_package(Vulkan REQUIRED)
#if(WIN32)
#    target_compile_definitions(Vulkan::Vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
#else() # Linux
#    target_compile_definitions(Vulkan::Vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
#    # Может быть ещё VK_USE_PLATFORM_WAYLAND_KHR или VK_USE_PLATFORM_XLIB_KHR
#endif()

add_subdirectory(third_party/external/glfw)
add_subdirectory(third_party/external/glm)
add_subdirectory(third_party/external/stb)
add_subdirectory(third_party/external/vulkan_memory_allocator)

# --------- Примеры ---------

# Версия стандарта C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Указываем Студии на то, что исходники в кодировке UTF-8.
# Это позволяет писать U'🍌'. У других компиляторов, похоже, нет с этим проблем.
# https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2295r6.pdf
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Создаёт ссылку для папки. Если ссылку создать не удалось, то копирует папку
function(create_dir_link from to)
    if(EXISTS ${to})
        return()
    endif()

    if(NOT CMAKE_HOST_WIN32)
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${from} ${to}
                        OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE RESULT)
    else()
        # Не используем create_symlink в Windows, так как создание symbolic links
        # [требует админских прав](https://ss64.com/nt/mklink.html),
        # а поддержка junctions из CMake
        # [была удалена](https://gitlab.kitware.com/cmake/cmake/-/merge_requests/7530)
        string(REPLACE / \\ from ${from})
        string(REPLACE / \\ to ${to})
        execute_process(COMMAND cmd /C mklink /J ${to} ${from}
                        OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE RESULT)
    endif()

    if(NOT RESULT EQUAL 0)
        # Причиной неудачи может быть перезаписанная переменная PATH, в которой нет %SystemRoot%\system32
        message("Не удалось создать ссылку для папки, поэтому копируем папку")
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory  ${from} ${to})
    endif()
endfunction()

# Куда будут помещены следующие скомпилированные экзешники
set(bin_dir ${CMAKE_BINARY_DIR}/result)

# Создаём папку
file(MAKE_DIRECTORY ${bin_dir})

# Для одноконфигурационных генераторов (MinGW)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bin_dir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${bin_dir})

# Для многоконфигурационных генераторов (Visual Studio)
foreach(config_name ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config_name} config_name)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_name} ${bin_dir})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_name} ${bin_dir})
endforeach()

# Чтобы программы при отладке находили ресурсы
set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY ${bin_dir})
# Переменная CMAKE_DEBUGGER_WORKING_DIRECTORY добавлена в CMake 4.0

# Копируем папки с ресурсами в result
set(dir_names shaders)
foreach(dir_name ${dir_names})
    create_dir_link(${CMAKE_CURRENT_SOURCE_DIR}/${dir_name} ${bin_dir}/${dir_name})
endforeach()

# Копируем dll-ки MinGW 
if(MINGW)
    # dll-ки MinGW находятся рядом с компилятором, поэтому определяем путь к компилятору
    execute_process(COMMAND where x86_64-w64-mingw32-gcc.exe
                    OUTPUT_VARIABLE mingw_fullpath)

    # Команда where может вернуть несколько строк, если MinGW установлен в разные места.
    # Преобразуем вывод команды в список и получаем нулевой элемент
    string(REPLACE "\n" ";" mingw_fullpath ${mingw_fullpath})
    list(GET mingw_fullpath 0 mingw_fullpath)

    cmake_path(GET mingw_fullpath PARENT_PATH mingw_dir)

    set(dlls ${mingw_dir}/libgcc_s_seh-1.dll
             ${mingw_dir}/libstdc++-6.dll
             ${mingw_dir}/libwinpthread-1.dll)

    foreach(dll ${dlls})
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll} ${bin_dir})
    endforeach()
endif()

set(log_source_files ${CMAKE_CURRENT_SOURCE_DIR}/common/Log.cpp ${CMAKE_CURRENT_SOURCE_DIR}/common/Log.h)
set(data_source_files ${CMAKE_CURRENT_SOURCE_DIR}/common/Data.cpp ${CMAKE_CURRENT_SOURCE_DIR}/common/Data.h)

add_subdirectory(src_01)
add_subdirectory(src_02)
add_subdirectory(src_03)
add_subdirectory(src_04)
add_subdirectory(src_05)
add_subdirectory(src_07)
