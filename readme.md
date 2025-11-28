Примеры из статей про API Vulkan с сайта <https://steps3d.narod.ru/articles.html> с собственными конфигами `CMakeLists.txt`.

Это не мой код, все права принадлежат [автору сайта](https://steps3d.narod.ru/me.html). GitHub автора: <https://github.com/steps3d>.

Скачивание репозитория с подмодулями в папку repo: `git clone --recurse-submodules --shallow-submodules https://github.com/1vanK/steps3d_vulkan repo`.

## Статьи

1. [Начало](https://steps3d.narod.ru/tutorials/vulkan-1-tutorial.html)
    * [base-1.cpp](src_01/base-1.cpp) - выводим список доступных расширений и слоёв валидации; создаём VkInstance
    * [base-2.cpp](src_01/base-2.cpp) - подключаем расширение VK_EXT_debug_utils и слой валидации VK_LAYER_KHRONOS_validation
    * Хабр: [Краткий обзор](https://habr.com/ru/articles/524992/), [Экземпляр (instance)](https://habr.com/ru/articles/531196/),
            [Слои валидации](https://habr.com/ru/articles/535342/)
2. [Устройства](https://steps3d.narod.ru/tutorials/vulkan-2-tutorial.html)
    * [base-3.cpp](src_02/base-3.cpp) - выводим список физических устройств и семейств очередей
    * Хабр: [Устройства и очереди](https://habr.com/ru/articles/537208/)
3. [Работа с памятью и буферами](https://steps3d.narod.ru/tutorials/vulkan-3-tutorial.html)
    * [base-4.cpp](src_03/base-4.cpp) - выбираем физическое устройство; создаём логическое устройство и командный пул (да, именно так примеры распределены по урокам)
4. [Шейдеры, pipeline layout, compute pipeline](https://steps3d.narod.ru/tutorials/vulkan-4-tutorial.html)
    * [base-5.cpp](src_04/base-5.cpp) - выводим информацию о памяти
    * [base-6.cpp](src_04/base-6.cpp) - выделяем память
    * [base-7.cpp](src_04/base-7.cpp) - вычисляем на GPU и читаем результат
5. [Рендеринг треугольника во внеэкранный буфер](https://steps3d.narod.ru/tutorials/vulkan-5-tutorial.html)
    * [base-8.cpp](src_05/base-8.cpp) - рендерим без создания окна (результат сохраняется в screenshot.png)
6. [Особенности систем координат в Vulkan, отличия от OpenGL](https://steps3d.narod.ru/tutorials/vulkan-coordinates-tutorial.html)
7. [Библиотека VMA (Vulkan Memory Allocator)](https://steps3d.narod.ru/tutorials/vulkan-6-tutorial.html)
    * [base-9.cpp](src_07/base-9.cpp) - используем VMA (результат сохраняется в screenshot-2.png)
8. [Vulkan с классами 1](https://steps3d.narod.ru/tutorials/vulkan-with-classes.html) | Все уроки ниже тоже с классами
9. [Vulkan с классами 2](https://steps3d.narod.ru/tutorials/vulkan-with-classes-2.html)
10. [Работа с текстурами](https://steps3d.narod.ru/tutorials/vulkan-with-classes-3.html)
11. [Анимация системы частиц при помощи вычислительных шейдеров](https://steps3d.narod.ru/tutorials/vulkan-with-classes-particles.html)
12. [Deferred renderer](https://steps3d.narod.ru/tutorials/vulkan-deferred.html)
13. [Выравнивание данных (UBO, SSBO)](https://steps3d.narod.ru/tutorials/vulkan-scalar-buffer-layout.html)
14. [Механизм запросов (queries)](https://steps3d.narod.ru/tutorials/vulkan-queries.html)
15. [Расширение VK_KHR_buffer_device_address или для буферов наконец завезли адреса](https://steps3d.narod.ru/tutorials/vulkan-buffer-address.html)
16. [Расширение VK_EXT_dynamic_rendering](https://steps3d.narod.ru/tutorials/vulkan-dynamic-rendering.html)

## Зависимости

В Windows нужно установить [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home).

В Linux Mint:

```
# Обновление списка пакетов
sudo apt update

# Установка Vulkan SDK
sudo apt install vulkan-tools libvulkan-dev vulkan-validationlayers spirv-tools

# Другие зависимости, которые нужны GLFW: https://www.glfw.org/docs/latest/compile.html
sudo apt install libxkbcommon-dev xorg-dev
```

Слои валидации нужно отключать в релизной версии, либо SDK (Windows) или vulkan-validationlayers (Linux) должны быть установлены у конечного пользователя.

## Ещё статьи

* [Vulkan за 30 минут](vulkan_in_30_minutes/ru.md)
* https://gamedev.ru/code/articles/tags/Vulkan
* https://github.com/faserg1/VulkanTest/tree/master/Tutorial
* [Еще один пост о синхронизации в Vulkan](https://habr.com/ru/articles/569318/)
* [Книги](https://gamedev.ru/code/forum/?id=256126)
* [Awesome Vulkan](https://github.com/vinjn/awesome-vulkan)
