# Registro de prompts de IA - prototipo V2

**Codigo V2 gerado com assistencia de IA.** Ao incorporar estas alteracoes num
repositorio Git da disciplina, o commit correspondente deve iniciar por `IA: `,
conforme as regras do professor. Nao fazer um unico commit para simular um
historico de desenvolvimento incremental; o processo e as contribuicoes da dupla
precisam ser documentados fielmente.

## Solicitações do usuario que definiram este prototipo

- "Chat. O jogo precisa parecer doom 😅"
- "Vamos gerar essas versão então"

Contexto explicitado na conversa anterior: manter o jogo Apocaliptic Kittens na
arena inspirada no Instituto de Informatica (UFRGS), com Teodora uma gata laranja,
primeira pessoa com orelhas e patas visiveis, camera secundaria em terceira pessoa
orbital, HUD no estilo Doom, corredores mais fechados, visual pixelado PS1/PS2,
Chads em patrulha/perseguicao, Betinhas resgataveis, e drone seguindo Bezier.

## Partes criadas/alteradas por IA nesta entrega

- `src/preview.cpp`: implementacao do FPS, gameplay, arena, entidades, camara e framebuffer.
- `src/overlay.cpp` e `include/overlay.h`: HUD e fonte pixelada procedural.
- `src/collisions.cpp` e `include/collisions.h`: AABB/circulo/deslizamento no mapa.
- `include/cg_math.h`: matrizes Model, View, Projection e rotacoes criadas manualmente.
- `assets/shaders/preview.frag`: iluminacao, nevoa e quantizacao de cores.
- `assets/textures/metal.png`: imagem procedural gerada por script Python.
- `CMakeLists.txt`: inclusao dos novos arquivos-fonte.
- `GUIA_EXECUCAO_V2.txt` e este registro de prompts: documentacao tecnica,
  nao o README final nem o SPEC.md da dupla.

## Exemplo de mensagem de commit (quando for verdadeiro)

`IA: prototipo FPS retro, HUD, cameras, colisoes e resgates`

Atenção: o README final e o SPEC.md devem respeitar as restricoes de autoria
explicitadas no enunciado e no template oficial da disciplina.


## Ajuste V2.1 — ponto de vista do DOOM (2026-11-11)

Prompt do usuario: "Melhorou mto, mas não queremos ver a gata no centro, vamos ver apenas a visão da cabeça e orelhas, acima do avatar do hud… doom não aparece o personagem no centro da tela" seguido de "Pode fazer isso".

Implementacao gerada por IA: em `src/preview.cpp`, o viewmodel FPS
foi isolado na funcao `drawFirstPersonCat()`. A silhueta anteriormente
exagerada foi substituida por duas orelhas pequenas e uma faixa do topo
da cabeca, todas sobre o retrato central TEO do HUD, sem ocupar a mira.
Os ataques ainda mostram a pata apenas durante a animacao. A camera
em terceira pessoa continua a renderizar o corpo inteiro.

Commit sugerido para essa alteracao, quando corresponder ao codigo aplicado:
`IA: reduzir viewmodel FPS para orelhas acima do retrato TEO`

## Integracao no repositorio (2026-11-25)

Prompt do usuario: "Avalia esse zip, vê o que ele mudou e atualiza o repo se já está mais doom anos 90".

Integracao feita por IA (Claude Code) na branch `doom-v2`, a partir da `knorst`:
- a V2.1 do ZIP foi trazida para a estrutura do repositorio, sem a pasta `assets/`:
  shaders em `src/preview_vertex.glsl` e `src/preview_fragment.glsl`, modelos e texturas
  em `data/`, caminhos `../../` como no `main` (executar de `bin/Debug`);
- `CMakeLists.txt`: `collisions.cpp` e `overlay.cpp` no alvo `apocaliptic_preview`;
- README, `data/FONTES.txt` e capturas em `images/preview_v2_*.png`;
- bibliotecas, GLAD, GLM, stb e tinyobj do ZIP conferidas: identicas as do repositorio;
- validado no Windows (MinGW64): compila sem avisos, `--self-test` OK e captura renderizada.

## Correcoes: Blinn-Phong, texturas e SPEC (2026-11-25)

Prompt do usuario: "depois disso vamos resolver teus achados, atualizando a spec conforme tua sugestão. Essas correções mantem na Knorst."

Gerado por IA (Claude Code), na `knorst`:
- `src/preview_fragment.glsl`: volta o termo especular do Blinn-Phong (half-vector,
  expoente 24), que a V2 tinha removido; mantidos nevoa e quantizacao;
- `src/preview_vertex.glsl`, `src/preview_fragment.glsl` e `src/preview.cpp`: nas caixas
  texturizadas o UV passa a ser a coordenada do objeto escalada pelo tamanho real da face,
  e o `tiles` vira repeticoes por metro. Antes, uma unica contagem por objeto esticava a
  imagem nas faces compridas (chao 23x73, paredes 72 m);
- `SPEC.md`: secoes de cameras (primeira pessoa livre + look-at orbital), transformacoes
  controladas pelo usuario (mouse + WASD) e funcionalidade extra (barra de status);
- capturas `images/preview_v2_*.png` refeitas.
