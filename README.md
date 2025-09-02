# Software Renderer

https://github.com/user-attachments/assets/aa9ad717-bde6-44f8-8207-60f736e0d112

## Documentation
The /docs directory contains GeoGebra, Mathcad and other files that contain visualization and formal derivation of all mathematical constructions used in the renderer: rotation matrices, clipping algorithms etc.

## Notes
- Вырожденные треугольники: проверить/убедиться/доказать, что для вырожденного треугольника (который является смежным с, например, двумя соседними "нормальными") справедливо следующее: если такой треугольник имеет хотя бы один фрагмент, который растеризуется, то этот фрагмент принадлежит только этому треугольнику и, соответственно, растеризуется только в контексте этого треугольники, то есть, смежные треугольники данный фрагмент не растеризуют, а значит, при реализации, например, прозрачности, данный фрагмент не будет закрашен дважды.
