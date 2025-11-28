# Перезаписываем опции
# https://stackoverflow.com/questions/39588836/why-do-i-need-force-to-override-a-cmake-option
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(repo)
