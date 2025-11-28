Основной способ передачи ресурсов (буферов и текстур) в шейдеры - наборы дескрипторов (descriptor sets).
В шейдерах ресурсы адресуются двумерной координатой: (`set`, `binding`) = (`номер набора`, `номер привязки внутри набора`).

```
layout(std430, set = 0, binding = 0) buffer InData // set = 0 можно не писать
{
    float values[];
};
```

DescriptorSetLayout (макет набора дескрипторов) - макет, который описывает один набор.
Несколько наборов полезны для ресурсов, которые меняются с разной частотой: <https://developer.android.com/ndk/guides/graphics/design-notes#group>.
Причём наборы надо сортировать в порядке увеличения частоты смены: Vulkan 1.0 spec > 14.2.2. Pipeline Layouts > Pipeline Layout Compatibility

> Place the least frequently changing descriptor sets near the start of the pipeline layout, and place the descriptor sets representing
> the most frequently changing resources near the end. When pipelines are switched, only the descriptor set bindings that have been
> invalidated will need to be updated and the remainder of the descriptor set bindings will remain in place.

## Примерная иерархия структур пайплайна

Тут не все поля структур.

### Вычислительный пайплайн

```text
Pipeline pipeline ≈ device.createComputePipeline(PipelineCache, ComputePipelineCreateInfo)
                                                                ├── PipelineShaderStageCreateInfo stage // Стадия конвейера
                                                                │   ├── ShaderStageFlagBits stage = ShaderStageFlagBits::eCompute
                                                                │   ├── ShaderModule module // Конкретный шейдер
                                                                │   └── const char* pName = "main"
                                                                └── PipelineLayout layout ≈ device.createPipelineLayout(PipelineLayoutCreateInfo) // Макет конвейера. Описывает все ресурсы в шейдерах (Uniforms, Samplers, Push Constants)
                                                                                                                        ├── uint32_t setLayoutCount = 1 // Число макетов наборов
                                                                                                                        └── DescriptorSetLayout* pSetLayouts ≈ device.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo) // Массив макетов наборов
                                                                                                                                                                                                ├── uint32_t bindingCount = 1 // Число привязок в макете набора
                                                                                                                                                                                                └── DescriptorSetLayoutBinding* pBindings // Массив привязок
                                                                                                                                                                                                    ├── uint32_t binding = 0 // Номер привязки
                                                                                                                                                                                                    ├── DescriptorType descriptorType = DescriptorType::eStorageBuffer
                                                                                                                                                                                                    ├── uint32_t descriptorCount = 1 // Привязка может быть массивом дескрипторов
                                                                                                                                                                                                    └── ShaderStageFlags stageFlags = ShaderStageFlagBits::eCompute
```

### Графический пайплайн

```text
Pipeline pipeline ≈ device.createGraphicsPipeline(PipelineCache, GraphicsPipelineCreateInfo)
                                                                 ├── uint32_t stageCount = 2
                                                                 ├── const PipelineShaderStageCreateInfo* pStages
                                                                 │                                        ├── [0] // Вершинный шейдер
                                                                 │                                        │   ├── ShaderStageFlagBits stage = ShaderStageFlagBits::eVertex
                                                                 │                                        │   ├── ShaderModule module // Конкретный шейдер
                                                                 │                                        │   └── const char* pName = "main"
                                                                 │                                        └── [1] // Фрагментный шейдер
                                                                 │                                            ├── ShaderStageFlagBits stage = ShaderStageFlagBits::eFragment
                                                                 │                                            ├── ShaderModule module // Конкретный шейдер
                                                                 │                                            └── const char* pName = "main"
                                                                 ├── const PipelineVertexInputStateCreateInfo* pVertexInputState // Описывает входные данные из буфера вершин = атрибуты вершин = как интерпретировать сырые байты из vk::Buffer
                                                                 ├── const PipelineInputAssemblyStateCreateInfo* pInputAssemblyState
                                                                 │                                               └── PrimitiveTopology topology = PrimitiveTopology::eTriangleList
                                                                 ├── const PipelineViewportStateCreateInfo* pViewportState
                                                                 │                                          ├── uint32_t viewportCount = 1
                                                                 │                                          ├── const Viewport* pViewports
                                                                 │                                          │                   └── [0]
                                                                 │                                          │                       └── float x=0.f, y=0.f, 640.f, 480.f, minDepth=0.f, maxDepth=1.f
                                                                 │                                          ├── uint32_t scissorCount = 1
                                                                 │                                          └── const Rect2D* pScissors
                                                                 │                                                            └── [0]
                                                                 │                                                                ├── Offset2D offset
                                                                 │                                                                │            └── int32_t x = 0, y = 0
                                                                 │                                                                └── Extent2D extent
                                                                 │                                                                             └── uint32_t width = 640, height = 480
                                                                 ├── const PipelineRasterizationStateCreateInfo* pRasterizationState
                                                                 │                                               ├── PolygonMode polygonMode = PolygonMode::eFill
                                                                 │                                               ├── CullModeFlags cullMode = CullModeFlagBits::eBack
                                                                 │                                               ├── FrontFace frontFace = FrontFace::eClockwise
                                                                 │                                               └── float lineWidth = 1.f
                                                                 ├── const PipelineMultisampleStateCreateInfo* pMultisampleState
                                                                 │                                             ├── SampleCountFlagBits rasterizationSamples = SampleCountFlagBits::e1 // Должно совпадать со значением в AttachmentDescription
                                                                 │                                             └── float minSampleShading = 1.f
                                                                 ├── const PipelineColorBlendStateCreateInfo* pColorBlendState
                                                                 │                                            ├── uint32_t attachmentCount = 1
                                                                 │                                            └── const PipelineColorBlendAttachmentState* pAttachments
                                                                 │                                                                                         └── [0]
                                                                 │                                                                                             ├── Bool32 blendEnable = false
                                                                 │                                                                                             ├── BlendFactor srcColorBlendFactor = BlendFactor::eOne
                                                                 │                                                                                             ├── BlendFactor dstColorBlendFactor = BlendFactor::eZero
                                                                 │                                                                                             ├── BlendOp colorBlendOp = BlendOp::eAdd
                                                                 │                                                                                             ├── BlendFactor srcAlphaBlendFactor = BlendFactor::eOne
                                                                 │                                                                                             ├── BlendFactor dstAlphaBlendFactor = BlendFactor::eZero
                                                                 │                                                                                             ├── BlendOp alphaBlendOp = BlendOp::eAdd
                                                                 │                                                                                             └── ColorComponentFlags colorWriteMask = ColorComponentFlagBits::eR | ColorComponentFlagBits::eG | ColorComponentFlagBits::eB | ColorComponentFlagBits::eA
                                                                 ├── PipelineLayout layout ≈ device.createPipelineLayout(PipelineLayoutCreateInfo) // Описано выше в вычислительном конвейере
                                                                 ├── RenderPass renderPass ≈ device.createRenderPass(RenderPassCreateInfo) // Проход состоит из подпроходов
                                                                 │                                                   ├── uint32_t attachmentCount = 1
                                                                 │                                                   ├── const AttachmentDescription* pAttachments
                                                                 │                                                   │                                └── [0]
                                                                 │                                                   │                                    ├── Format format = Format::eR8G8B8A8Unorm
                                                                 │                                                   │                                    ├── SampleCountFlagBits samples = SampleCountFlagBits::e1
                                                                 │                                                   │                                    ├── AttachmentLoadOp loadOp = AttachmentLoadOp::eClear
                                                                 │                                                   │                                    ├── AttachmentStoreOp storeOp = AttachmentStoreOp::eStore
                                                                 │                                                   │                                    ├── AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::eDontCare
                                                                 │                                                   │                                    ├── AttachmentStoreOp stencilStoreOp = AttachmentStoreOp::eDontCare
                                                                 │                                                   │                                    ├── ImageLayout initialLayout = ImageLayout::eUndefined
                                                                 │                                                   │                                    └── ImageLayout finalLayout = ImageLayout::ePresentSrcKHR
                                                                 │                                                   ├── uint32_t subpassCount = 1
                                                                 │                                                   └── const SubpassDescription* pSubpasses
                                                                 │                                                                                 └── [0]
                                                                 │                                                                                     ├── PipelineBindPoint pipelineBindPoint = PipelineBindPoint::eGraphics
                                                                 │                                                                                     ├── uint32_t colorAttachmentCount = 1
                                                                 │                                                                                     └── const AttachmentReference* pColorAttachments
                                                                 │                                                                                                                    └── [0]
                                                                 │                                                                                                                        ├── uint32_t attachment = 0
                                                                 │                                                                                                                        └── ImageLayout layout = ImageLayout::eColorAttachmentOptimal
                                                                 └── uint32_t subpass = 0
```
