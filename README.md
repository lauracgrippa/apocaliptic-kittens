# Computação Gráfica e Visualização I (INF01047) - INF/UFRGS

Este repositório contém o código base para o trabalho final. O enunciado completo do trabalho final está no Moodle:

https://moodle.ufrgs.br/mod/assign/view.php?id=6302370

## Protótipo Apocaliptic Kittens (V2.1, visual retrô)

O protótipo é um segundo executável, `apocaliptic_preview`, compilado junto com `main` pelo mesmo `CMakeLists.txt`. A V2.1 troca a caminhada em terceira pessoa por um visual de FPS dos anos 90: render em 640×360 ampliado sem filtro, texturas `GL_NEAREST`, cores quantizadas, névoa escura e barra de status no rodapé.

- Código: `src/preview.cpp` (jogo), `src/collisions.cpp` (círculo × AABB com deslizamento), `src/overlay.cpp` (HUD e fonte pixelada), `include/cg_math.h` (matrizes Model, View e Projection escritas à mão)
- Shaders: `src/preview_vertex.glsl`, `src/preview_fragment.glsl`
- Modelos e texturas: `data/cube.obj`, `data/sphere.obj`, `data/red_brick_diff_1k.jpg`, `data/rocky_terrain_02_diff_1k.jpg`, `data/metal.png` (gerada por `scripts/gerar_textura_metal.py`)
- Capturas: `images/preview_v2_fps.png`, `images/preview_v2_third.png` (a V1 continua em `images/preview.png`)
- Código gerado com IA: ver `PROMPTS.md`

Assim como `main`, deve ser executado de dentro de `bin/Debug` (ou `bin/Release`, `bin/Linux`), pois os caminhos são relativos (`../../data/...`). No Linux: `cmake --build --preset default-build --target run_preview`.

| Ação | Comando |
| --- | --- |
| Mover / correr | W A S D / Shift |
| Olhar (FPS) ou orbitar (terceira pessoa) | Mouse |
| Patada (curto alcance) | Botão esquerdo |
| Miau sônico (projétil) | Botão direito |
| Resgatar Betinha próxima | E |
| Alternar FPS / terceira pessoa orbital | Tab |
| Zoom (terceira pessoa) | Roda do mouse |
| Reiniciar | R |
| Soltar / capturar o cursor | Esc |
| Sair | Q |

Missão demonstrativa: resgatar 2 das 3 Betinhas antes que os Chads matem duas. Teodora tem 9 vidas; o drone patrulha numa curva de Bézier cúbica.

Opções: `--self-test` (colisões, resgates, vitória e Bézier, sem abrir janela), `--demo` (Teodora anda sozinha), `--third-person`, `--frames N --capture arquivo.ppm` (renderiza N quadros e salva uma captura).

Origem dos assets: as texturas de tijolo e terreno são da Poly Haven (`red_brick`, `rocky_terrain_02`); `metal.png` é procedural; `cube.obj` e `sphere.obj` vêm dos laboratórios da disciplina.
