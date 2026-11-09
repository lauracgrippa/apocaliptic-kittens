# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS

Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle:

https://moodle.ufrgs.br/mod/assign/view.php?id=6302370

## Protótipo Apocaliptic Kittens

O protótipo (antes em `legacy/apocaliptic_kittens_prototipo_opengl.zip`) foi integrado como um segundo executável, `apocaliptic_preview`, compilado junto com `main` pelo mesmo `CMakeLists.txt`.

- Código: `src/preview.cpp`
- Shaders: `src/preview_vertex.glsl`, `src/preview_fragment.glsl`
- Modelos/texturas: `data/cube.obj`, `data/sphere.obj`, `data/red_brick_diff_1k.jpg`, `data/rocky_terrain_02_diff_1k.jpg`
- Captura de tela: `images/preview.png`

Assim como `main`, deve ser executado de dentro de `bin/Debug` (ou `bin/Release`, `bin/Linux`), pois os caminhos são relativos (`../../data/...`). No Linux: `cmake --build --preset default-build --target run_preview`.

| Ação | Comando |
| --- | --- |
| Câmera fixa: andar / ré / girar | W / S / A D (Shift corre) |
| Órbita: andar relativo à câmera | W A S D (Shift corre) |
| Alternar câmera fixa / órbita | C |
| Girar a câmera | Botão direito + arrastar |
| Zoom | Roda do mouse |
| Reiniciar Teodora | R |
| Sair | Esc |

Opções: `--demo` (Teodora caminha sozinha), `--frames N --capture arquivo.ppm` (renderiza N quadros e salva uma captura).

Origem dos assets: as texturas são da Poly Haven (`red_brick`, `rocky_terrain_02`); `cube.obj` vem dos laboratórios da disciplina.
