# Especificação da Implementação

## Integrantes da dupla

1) Henrique Anderson Knorst (173942)
2) Laura Cárdenas Grippa (219351)

## Detalhes do que será implementado

### Apocaliptic Kittens

Em _**Apocaliptic Kittens**_, as IAs finalmente se cansaram da humanidade, e a culpa é toda dos _Chads_. Neste jogo 3D de ação e sobrevivência satírico, o jogador controla _Teodora_, uma gata laranja implacável que explora as ruínas do Campus do Vale da UFRGS. 
A missão de Teodora é proteger os _Betinhas_ (humanos decentes que restaram) e exterminar os _Chads_, a escória cujas ideias tóxicas propagadas na internet dos anos 2000 enlouqueceram os algoritmos e causaram o apocalipse.

## Especificação visual

### Vídeo - Link

> [!IMPORTANT]
> - Coloque aqui um link para um vídeo que mostre a aplicação gráfica
>   de referência que você vai implementar. **Sua implementação deverá
>   ser o mais parecido possível com o que é mostrado no vídeo (mais
>   detalhes abaixo).**
> - **Você não pode escolher como referência: (1) algum trabalho realizado
>   por outros alunos desta disciplina, em semestres anteriores. (2) Minecraft.**
> - Por exemplo, você pode colocar um vídeo de um jogo que você gosta,
>   e seu trabalho final será uma re-implementação do jogo.
> - O vídeo pode ser um link para YouTube, Google Drive, ou arquivo mp4 dentro
>   do próprio repositório. Mas, garanta que qualquer um tenha
>   permissão de acesso ao vídeo através deste link.

<mark>`https://www.youtube.com/watch?v=MnqLJpgq7jc`</mark>

### Vídeo - Timestamp

> [!IMPORTANT]
> - Coloque aqui um **intervalo de ~30 segundos** do vídeo acima, que
>   será a base de comparação para avaliar se o seu trabalho final
>   conseguiu ou não reproduzir a referência.

- **Timestamp inicial**: <mark>`https://youtu.be/MnqLJpgq7jc?si=diFH3LuA-xRiJF0c&t=9`</mark>
- **Timestamp final**: <mark>`https://youtu.be/MnqLJpgq7jc?si=diFH3LuA-xRiJF0c&t=39`</mark>

### Imagens

> [!IMPORTANT]
> - Coloque aqui **três imagens** capturadas do vídeo acima, que você
>   irá usar como ilustração para as explicações que vêm abaixo.
> - As imagens devem estar armazenadas neste repositório, no diretório
>   `images/spec/`, com os nomes `image1`, `image2` e `image3`.
> - Cada imagem deve usar o formato `.jpg` ou `.png`. Ajuste a extensão
>   nos vínculos abaixo para que corresponda ao arquivo armazenado.
> - Escolha imagens que correspondam a momentos do intervalo indicado
>   acima ou que sejam relevantes para a comparação com a implementação.

#### Imagem 1

- **Descrição**: <mark>`<preencher>`</mark>

![Imagem 1](images/spec/image1.jpg)

#### Imagem 2

- **Descrição**: <mark>`<preencher>`</mark>

![Imagem 2](images/spec/image2.jpg)

#### Imagem 3

- **Descrição**: <mark>`<preencher>`</mark>

![Imagem 3](images/spec/image3.jpg)

## Especificação textual

### Malhas poligonais complexas
Os personagens Teodora (gata), Chads (figuras humanas masculinas) e Betinhas (figuras humanas variadas), o cenário do Campus do Vale e objetos menores, como drones e churus, serão representados por modelos low-poly obtidos em repositórios permitidos pelas diretrizes do trabalho.

### Transformações geométricas controladas pelo usuário
As transformações geométricas são calculadas para cada elemento do jogo. O cumprimento da exigência de transformações controladas pelo usuário ocorre por meio da protagonista Teodora (instância única): o jogador modifica os parâmetros matriciais da gata, aplicando transformações de translação (caminhar pelo mapa), rotação (virar para a direção do movimento) e escala (como em animações de dano ou interação) por meio das teclas W, A, S, D.

Além da personagem controlada pelo jogador, o jogo aplica outras transformações nos demais elementos da cena:

- Betinhas sofrem transformações de posição, rotação e escala.
- Chads e Drones: sofrem transformações de posição e rotação.
- Churus: sofrem transformações de posição e de rotação contínua (girando sobre o próprio eixo para chamar a atenção do jogador).

### Diferentes tipos de câmeras
O jogo implementa duas câmeras:

1) **Câmera em Terceira Pessoa Livre**, posicionada atrás e ligeiramente acima da personagem, acompanhando os movimentos dela no espaço;
2) **Câmera de Órbita em Terceira Pessoa**, em que a câmera gira em torno do próprio personagem, mantendo uma distância constante.

### Instâncias de objetos
- **Teodora:** 1 instância. Muda de posição, rotação e escala.
- **Betinhas:** ver _Cálculo de spawns por fase_. Mudam de posição e rotação.
- **Chads:** ver _Cálculo de spawns por fase_. Mudam de posição e rotação.
- **Drone:** ver _Cálculo de spawns por fase_. Mudam de posição e rotação.
- **Churus:** mesmo número de Betinhas no mapa. Mudam de posição e rotação.

#### Cálculo de spawns por fase
A quantidade de personagens será definida _progressivamente_ de acordo com o número da fase (F), **iniciando em 1**.

A meta de Betinhas a serem resgatadas será calculada por R(F) = min(2 + ⌊F/2⌋, 5), enquanto o total disponível será B(F) = R(F) + 1, permitindo a perda de uma sobrevivente. A quantidade total de Chads será C(F) = 3 + 2(F − 1), com limite de inimigos simultâneos definido por Cmax(F) = min(2 + ⌊(F − 1)/2⌋, 5). Os drones serão limitados a uma unidade nas três primeiras fases e duas unidades a partir da quarta.

O sistema de spawns respeitará limites de personagens ativos, intervalos de geração e distâncias mínimas entre personagens, garantindo uma progressão de dificuldade controlada.

##### Legenda das fórmulas

- **F:** número da fase atual, começando em 1.
- **R(F):** quantidade mínima de Betinhas que Teodora precisa resgatar para vencer a fase.
- **B(F):** quantidade total de Betinhas disponíveis na fase.
- **C(F):** quantidade total de Chads que poderão surgir durante a fase.
- **Cmax(F):** quantidade máxima de Chads ativos simultaneamente.
- **min(a, b):** retorna o menor valor entre *a* e *b*, estabelecendo um limite máximo.
- **⌊x⌋ (floor):** arredonda o valor para o inteiro imediatamente inferior.
- **Drones:** uma unidade nas fases 1 a 3 e duas unidades a partir da fase 4.

### Testes de intersecção
Os testes de intersecção são realizados a partir dos cenários em que os elementos do jogo podem colidir. Dividiu-se os cenários de colisão de acordo com cada personagem e objeto do jogo:

#### Personagem 01: Teodora
- **Teodora x paredes:** Teodora pode colidir com as paredes do Campus do Vale enquanto anda pelo Campus do Vale. Teodora não pode “entrar” nas paredes. Se Teodora encostar na parede, nada acontece.
- **Teodora x Chad:** Teodora pode colidir com Chad enquanto anda pelo Campus do Vale. Teodora não pode “entrar” na figura humana do Chad e não pode encostar no Chad. Se Teodora encostar no Chad, ela perde 1 vida.
- **Teodora x Aura Chad:** Teodora pode colidir com a Aura Chad enquanto anda pelo Campus do Vale. Teodora pode “entrar” no raio da Aura do Chad. Se Teodora entra no raio da Aura Chad, Teodora entra no modo de ataque (suas unhas afiadas são expostas).
- **Teodora x Redpill:** Teodora pode colidir com Redpills enquanto é atacada pelo Chad. Se Teodora encostar na Redpill, ela perde 1 vida.
- **Teodora x Betinha:** Teodora pode colidir com Betinha enquanto anda pelo Campus do Vale. Teodora não pode “entrar” nas figuras humanas Betinhas, mas pode encostar nelas. Se Teodora encosta no Betinha, Teodora salva o Betinha.
- **Teodora x Churu:** Teodora pode colidir com Churus enquanto anda pelo Campus do Vale. Teodora não pode “entrar” nos elementos Churus, mas pode encostar neles. Se Teodora encostar no Churu, ela ganha 1 vida.

#### Personagem 02: Chad
- **Chad x paredes:** Chad pode colidir com as paredes do Campus do Vale enquanto anda pelo Campus do Vale. Chad não pode “entrar” nas paredes. Se Chad encostar na parede, nada acontece.
- **Chad x Betinha:** Chad pode colidir com Betinha enquanto anda pelo Campus do Vale. Chad não pode “entrar” nas figuras humanas Betinhas, mas pode encostar nelas. Se Chad encostar no Betinha, morre um Betinha.

##### Atributo de Personagem (Chad): Aura Chad
- **Aura Chad x paredes:** A Aura Chad pode colidir com as paredes do Campus do Vale enquanto anda pelo Campus do Vale. A Aura Chad pode “entrar” nas paredes.
- **Aura Chad x Teodora:** explorado em tópico anterior.

##### Atributo de Personagem (Chad): Redpill
- **Redpill x paredes:** Redpill nunca colide com as paredes do Campus do Vale.
- **Redpill x Teodora:** explorado em tópico anterior.

#### Personagem 03: Betinha
- **Betinha x paredes:** Betinha pode colidir com as paredes do Campus do Vale enquanto anda pelo Campus do Vale. Betinha não pode “entrar” nas paredes. Se Betinha encostar na parede, nada acontece.
- **Betinha x Teodora:** Betinha pode colidir com Teodora enquanto anda pelo Campus do Vale. Betinha não pode “entrar” na gata Teodora, mas pode encostar nela. Se Betinha encosta na Teodora, Betinha é salva pela Teodora (missão principal do jogo).

#### Objeto 01: Drone
- **Drone x paredes:** Drone pode colidir com as paredes do Campus do Vale enquanto sobrevoa pelo Campus do Vale. Drone não pode “entrar” nas paredes. Se Drone encostar na parede, nada acontece.
- **Drone x Teodora:** Drone nunca colide com Teodora enquanto sobrevoa pelo Campus do Vale, pois ele está acima dela, sobrevoando.

#### Objeto 02: Churu
- **Churu x paredes:** Churu nunca colide com as paredes do Campus do Vale. Porém, Churu não pode ser criado “dentro” nas paredes.
- **Churu x Teodora:** explorado em tópico anterior.

### Modelos de Iluminação em todos os objetos
Será implementado o modelo matemático de iluminação de Blinn-Phong (com componentes ambiente, difusa e especular) nos *shaders* da aplicação, garantindo que todos os objetos 3D no campus reajam de forma não-trivial a uma luz direcional representando o sol.

### Mapeamento de texturas em todos os objetos
Todos os objetos terão mapeamento de coordenadas UV (texturas 2D representadas por no mínimo três imagens distintas). Para evitar perda de nota, os cálculos de mapeamento e a construção dos modelos 3D garantirão que as imagens não fiquem esticadas de forma antinatural.

### Movimentação com curva Bézier cúbica
Um drone de vigilância controlado pelas Inteligências Artificiais sobrevoa o Campus do Vale, realizando uma patrulha aérea por uma trajetória suave e curvilínea no espaço tridimensional, definida matematicamente por uma curva de Bézier cúbica. Se o drone avistar a Teodora, Teodora perde 1 vida.

### Movimentação dos inimigos
- **Chads:** os chads utilizam uma máquina de estados para controlar seu comportamento, alternando entre patrulha, perseguição e ataque. Durante a patrulha, percorrerão pontos predefinidos do cenário (waypoints). Ao detectar Teodora ou uma Betinha, passarão a perseguir o alvo, respeitando as colisões com obstáculos do mapa.
- **Drone:** já explorado em tópico anterior.

### Animações baseadas no tempo ($\Delta t$)
Todas as movimentações e rotações dos objetos dinâmicos do jogo, incluindo Teodora, Chads, drones, projéteis e câmeras, serão calculadas utilizando o tempo decorrido entre atualizações (*deltaTime*). As velocidades lineares e angulares serão multiplicadas por esse fator, garantindo que os movimentos ocorram de forma consistente, independentemente da taxa de quadros por segundo (FPS) do computador. O deslocamento dos drones ao longo das curvas de Bézier também será baseado no tempo.

### Funcionalidade extra obrigatória
A dupla implementará outros elementos gráficos do jogo, a barra de hidratação e o contador de vidas da gata Teodora.

## Limitações esperadas

> [!IMPORTANT]
> - Coloque aqui uma lista de detalhes visuais ou de interação que
>   aparecem no vídeo e/ou imagens acima, mas que você **não pretende
>   implementar** ou que você **irá implementar parcialmente**.
> - Para cada item, **explique por que** não será implementado ou por
>   que será implementado parcialmente.

<mark>`<preencher>`</mark>
