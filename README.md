# Software Renderer

https://github.com/user-attachments/assets/6701d146-6460-4131-9a6a-c8930ce3d4fa

## Features
### Line drawing (DDA, z-buffer)
![](docs/screens/line_dda_z_buffer.png)

### Line clipping (Cohen–Sutherland)
![](docs/screens/line_clipping_cohen_sutherland.png)

### Triangle filling (top-left strategy, screen space clipping)
![](docs/screens/triangle_filling_top_left.png)

### Z-buffering
![](docs/screens/z_buffering.png)

### Color interpolation
![](docs/screens/color_interpolation.png)

### Frustum culling
![](docs/screens/frustum_culling.png)

### Near plane clipping
![](docs/screens/near_plane_clipping.png)

### Texture mapping (affine)
![](docs/screens/texture_mapping.png)

## Documentation
The /docs directory contains GeoGebra, Mathcad and other files that contain visualization and formal derivation of all mathematical constructions used in the renderer: rotation matrices, clipping algorithms etc.

## Notes
- Вырожденные треугольники: проверить/убедиться/доказать, что для вырожденного треугольника (который является смежным с, например, двумя соседними "нормальными") справедливо следующее: если такой треугольник имеет хотя бы один фрагмент, который растеризуется, то этот фрагмент принадлежит только этому треугольнику и, соответственно, растеризуется только в контексте этого треугольники, то есть, смежные треугольники данный фрагмент не растеризуют, а значит, при реализации, например, прозрачности, данный фрагмент не будет закрашен дважды.
- В данной реализации возможны "дырки" на границах двух смежных треугольников из-за инкрементного вычисления начала и конца каждой скан-линии. Например, при отрисовке левого треугольника мы можем пропустить пиксел на правом ребре из-за накопленных ошибок округления при вычислении конца скан-линии: допустим, истинное значение конца скан-линии для текущей координаты Y равно 399.000123, но инкрементом мы попали в 399.0, поэтому ceil(399.0)-1 даст 398. Но при отрисовке правого треугольника, на этом же уровне Y мы можем получить истинное начало скан-линии равное 399.000123, что даст после применения правила top-left ceil(399.000123) = 400.0, поэтому пиксел с координатой 399 также не будет отрисован. Самое простое решение проблемы: для каждой скан-линии вычислять начало и конец как текущая дельта по y, умноженная на slope ребра.

> Этот рендерер используется в других моих проектах и там я его немного отрефакторил, в т.ч. исправил "дырки" на границах смежных треугольников.
> https://github.com/igor-240340/GeoDistance
> https://github.com/igor-240340/AntennaDemo

## Model
https://skfb.ly/6V6GL
