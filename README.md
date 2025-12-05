# Trabalho Final - Fundamentos de Computação Gráfica

**Disciplina:** INF01047 - Fundamentos de Computação Gráfica
**Professor:** Eduardo Gastal
**Instituição:** UFRGS - Instituto de Informática

## Descrição do Jogo

Jogo de ação 3D em arena onde o jogador controla uma arqueira/maga que deve derrotar inimigos e um boss dragão. O jogador vence ao derrotar o dragão e perde se sua vida chegar a zero.

---

## Contribuições

### Emanuel Rauter:
- Efeitos Sonoros (integração com miniaudio)
- Gouraud Shading (iluminação por vértice)
- Phong Shading (iluminação por pixel)
- Iluminação Lambert (Difusa)
- Escolha e implementação de Texturas
- Escolha e implementação de Modelos 3D
- Spawn de Inimigos (uso de uma mesma matriz modelo para várias instâncias)
- Colisão AABB x AABB
- Câmera Look-At

### Gabriel Stoffel:
- Câmera primeira pessoa (livre)
- Projéteis e lógica de tiro
- Construção do mapa/arena
- Iluminação Blinn-Phong (especular)
- Iluminação considerando tochas como fonte de luz (múltiplas fontes pontuais)
- Curva de Bézier para movimentação de inimigos
- Colisão AABB x Plano
- Colisão Ponto-Esfera
- Lógica do jogador e inimigos
- Animações baseadas no tempo (deltaTime)
- Modo Pausa com câmera esférica

---

## Uso de Inteligência Artificial

O ChatGPT foi utilizado no trabalho, principalmente na resolução de dúvidas sobre colisões e para correções de bugs no código. A consulta com IA se mostrou mais efetiva de forma inversamente proporcional à quantidade de contexto do programa necessário para resolver a tarefa.

**Onde a IA auxiliou adequadamente:**
- Funções pontuais como testes de colisões (implementação de AABB, ponto-esfera)
- Verificação de erros no shader de fragmentos
- Questões teóricas sobre como implementar funções específicas (ex: cálculo de diferença de ângulo entre dois vetores para rotacionar o boss e inimigos)

**Onde a IA não auxiliou adequadamente:**
- Problemas que envolviam relações entre classes separadas geraram respostas não satisfatórias
- A IA frequentemente perdia o contexto e respondia de forma inválida
- Exemplo específico: bug onde o print do texto do crosshair não permitia atualização da câmera primeira pessoa - ChatGPT foi incapaz de ajudar na resolução

**Conclusão:** A IA é útil para tarefas isoladas e bem definidas, mas perde eficácia quando o problema requer entendimento de múltiplas partes do código simultaneamente.

---

## Requisitos Técnicos Implementados

### 1. Malhas Poligonais Complexas

**Onde:** Modelos carregados via tinyobjloader em `src/Renderer.cpp`

| Modelo | Arquivo | Complexidade |
|--------|---------|--------------|
| Arqueira (jogador) | `modelos/arqueira.obj` | 957 KB |
| Dragão (boss) | `modelos/dragon.obj` | 2.99 MB |
| Monstro (inimigo) | `modelos/monstro.obj` | 1.18 MB |
| Varinha | `modelos/Varinha.obj` | 63 KB |
| Projétil | `modelos/fireball.obj` | 62 KB |
| Vida (pickup) | `modelos/vida.obj` | 32 KB |

### 2. Transformações Geométricas Controladas pelo Usuário

**Onde:** `src/Player.cpp`, `src/Input.cpp`

- **Translação:** Movimento do jogador com teclas W/A/S/D
- **Rotação:** Rotação da câmera com movimento do mouse
- **Modelo composto:** Player + Varinha utilizam matrizes empilhadas (`src/Renderer.cpp:renderPlayer()`) - a varinha acompanha a posição e rotação do jogador através de operações sucessivas em matrizes modelo

### 3. Câmera Livre e Câmera Look-At

**Onde:** `src/Player.cpp`, `src/Game.cpp`

- **Câmera Livre (Primeira Pessoa):** Implementada em `Player::updateFirstPersonCamera()` - movimento relativo à direção do olhar, rotação com mouse
- **Câmera Look-At (Terceira Pessoa):** Implementada em `Player::updateLookAtCamera()` - câmera orbita ao redor do jogador
- **Alternância:** Tecla F alterna entre os modos
- **Câmera do Modo Pausa:** Câmera esférica adicional para observação (`Game::updatePausedCamera()`)

### 4. Instâncias de Objetos

**Onde:** `src/Enemy.cpp`, `src/Renderer.cpp`

- **Múltiplos inimigos:** Classe `EnemyManager` gerencia N instâncias de inimigos usando o mesmo VAO/VBO mas diferentes Model matrices
- **Pilares:** 10 pilares na arena usando mesmo modelo com translações diferentes
- **Tochas:** 8 tochas posicionadas ao redor da arena
- **Projéteis:** Pool de projéteis reutilizados com diferentes posições

### 5. Três Tipos de Testes de Intersecção

**Onde:** `src/collisions.cpp` (arquivo dedicado conforme especificação)

| Tipo | Função | Uso |
|------|--------|-----|
| AABB x Plano | `testAABBPlane()`, `resolveAABBPlane()` | Colisão do jogador/inimigos com paredes, piso e teto da arena |
| AABB x AABB | `testAABBAABB()` | Colisão entre jogador, inimigos e pilares |
| Ponto x Esfera | `testPointSphere()` | Colisão de projéteis com inimigos e jogador |

Também implementado: `testBezierAABB()` para validar curvas de Bézier antes de serem usadas.

### 6. Modelos de Iluminação Difusa e Blinn-Phong

**Onde:** `src/shaders/shader_fragment.glsl`

- **Lambert (Difusa):** Aplicada em todos os objetos - `max(0, dot(n, l))`
- **Blinn-Phong (Especular):** Aplicada com diferentes valores de shininess:
  - Player: shininess = 128
  - Inimigos: shininess = 64
  - Dragão: shininess = 64
  - Varinha: shininess = 32
  - Pickups de vida: shininess = 4

**Iluminação dinâmica:** 8 tochas como fontes de luz pontuais com atenuação por distância: `1.0 / (1.0 + 0.7*d + 1.8*d²)`

### 7. Modelos de Interpolação de Phong e Gouraud

**Onde:** `src/shaders/shader_vertex.glsl`, `src/shaders/shader_fragment.glsl`

| Modelo | Objetos | Onde |
|--------|---------|------|
| **Gouraud** (por vértice) | Piso, paredes, teto da arena | Iluminação calculada no vertex shader, cor interpolada |
| **Phong** (por pixel) | Player, inimigos, dragão, projéteis, pickups, tochas | Normais interpoladas, iluminação calculada no fragment shader |

### 8. Mapeamento de Texturas em Todos os Objetos

**Onde:** `src/Renderer.cpp` (carregamento), `src/shaders/shader_fragment.glsl` (amostragem)

| Textura | Arquivo | Objeto |
|---------|---------|--------|
| Arqueira | `texturas/Arqueira.png` | Modelo do jogador |
| Varinha | `texturas/Varinha.png` | Arma do jogador |
| Monstro | `texturas/monstro.jpg` | Inimigos |
| Dragão | `texturas/dragon_skin.jpg` | Boss dragão |
| Chão | `texturas/Chao.png` | Piso da arena |
| Parede | `texturas/parede.jpg` | Paredes da arena |
| Teto | `texturas/telhado.jpg` | Teto da arena |
| Magia | `texturas/magica.jpg` | Projéteis do jogador |
| Fogo | `texturas/fogo.jpg` | Projéteis do dragão, tochas |
| Vida | `texturas/vida.png` | Pickups de vida |

**Mapeamento esférico:** Usado para projéteis (`src/shaders/shader_fragment.glsl`)

### 9. Movimentação com Curva de Bézier Cúbica

**Onde:** `src/Enemy.cpp`

- **Tipo:** Curvas de Hermite cúbicas (equivalente a Bézier cúbica)
- **Uso:** Inimigos calculam curvas suaves entre sua posição atual e o jogador
- **Implementação:**
  - Função `Enemy::computeBezierPosition()` - avalia posição na curva
  - Vetores tangentes geram movimento em "zig-zag" suave
  - Curva recalculada ao colidir com obstáculos (validação via `testBezierAABB()`)
  - Parâmetro t incrementado baseado no tempo para movimento suave

### 10. Animações Baseadas no Tempo (Δt)

**Onde:** `src/Game.cpp`, `src/Player.cpp`, `src/Enemy.cpp`, `src/Projectile.cpp`

- **deltaTime:** Calculado em `Game::update()` como diferença entre frames
- **Movimento do jogador:** `position += velocity * deltaTime`
- **Movimento de inimigos:** `bezierT += speed * deltaTime`
- **Projéteis:** `position += direction * speed * deltaTime`
- **Animações de morte:** Timer de 0.35s para animação de flash
- **Spawn de inimigos:** Baseado em tempo de jogo (gameSeconds)
- **Cooldown de dano:** Tempo entre hits para evitar dano múltiplo instantâneo

---

## Funcionalidades Extras

- **Efeitos Sonoros:** Biblioteca miniaudio - música de fundo, sons de tiro, dano, morte, cura, vitória/derrota
- **Trail de Projéteis:** Sistema de partículas simples com 6 pontos de histórico
- **Interface Gráfica:** Menus de dificuldade, HUD com barra de vida, crosshair, hitmarker, muzzle flash
- **Modo Pausa:** Câmera esférica para observação com labels identificando entidades
- **Múltiplos Níveis de Dificuldade:** 3 níveis ajustando spawn rate, quantidade de inimigos e velocidade

---

## Imagens

### Primeira Pessoa (dentro do jogo):
![Primeira pessoa](thumbnail/InicioJogo.png)

### Look-At / Terceira Pessoa (dentro do jogo):
![Terceira pessoa](thumbnail/PrimeiraPessoa.png)

---

## Manual de Utilização

### Menu Principal
| Tecla | Ação |
|-------|------|
| 1, 2 ou 3 | Escolher dificuldade e iniciar partida |
| ESC | Sair do jogo |

### Durante a Partida - Primeira Pessoa
| Controle | Ação |
|----------|------|
| W, A, S, D | Mover personagem relativo ao olhar |
| Arrastar mouse | Alterar câmera |
| Clique esquerdo | Atirar |
| P | Modo Pause |
| F | Trocar para Look-At |

### Durante a Partida - Look-At (Terceira Pessoa)
| Controle | Ação |
|----------|------|
| W, A, S, D | Mover personagem relativo aos eixos globais |
| Segurar clique esquerdo + arrastar | Alterar câmera |
| P | Modo Pause |
| F | Trocar para primeira pessoa |

### Modo Pause
| Controle | Ação |
|----------|------|
| A, D | Alternar visualização entre player e inimigos |
| Segurar clique esquerdo + arrastar | Alterar câmera |
| Scroll do mouse | Zoom |
| P | Sair do Pause |

### Tela de Resultado (Vitória ou Game Over)
| Tecla | Ação |
|-------|------|
| R | Reiniciar partida |
| M | Voltar ao menu principal |

---

## Como Compilar e Executar

### Linux (Recomendado)

**Pré-requisitos:**
- g++ com suporte a C++11
- OpenGL 3.3+
- Bibliotecas: GLFW3, X11, Xrandr, Xi, pthread, dl

**Instalação de dependências (Ubuntu/Debian):**
```bash
sudo apt-get install build-essential libglfw3-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev
```

**Compilar:**
```bash
make
```

**Executar:**
```bash
./bin/Linux/main
```

### Windows (via CMake)

**Pré-requisitos:**
- CMake 3.10+
- Visual Studio ou MinGW

**Compilar:**
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

**Executar:**
```bash
./bin/Release/main.exe
```

---

## Estrutura do Código

```
fgc/
├── src/
│   ├── main.cpp              # Ponto de entrada
│   ├── Game.cpp              # Loop principal e estados do jogo
│   ├── Player.cpp            # Lógica do jogador e câmeras
│   ├── Enemy.cpp             # Inimigos e curvas de Bézier
│   ├── Renderer.cpp          # Renderização OpenGL
│   ├── collisions.cpp        # Testes de colisão (arquivo dedicado)
│   ├── Input.cpp             # Entrada de teclado/mouse
│   ├── Projectile.cpp        # Sistema de projéteis
│   ├── sfx.cpp               # Efeitos sonoros
│   └── shaders/
│       ├── shader_vertex.glsl
│       └── shader_fragment.glsl
├── include/                  # Headers
├── modelos/                  # Modelos 3D (.obj)
├── texturas/                 # Texturas (.png, .jpg)
├── sfx/                      # Arquivos de áudio (.mp3)
└── Makefile
```

---

## Desenvolvimento

O grupo começou o desenvolvimento em cima do Laboratório 2 (câmera look-at e livre) e a partir daí foi construindo junto com os respectivos laboratórios.

**Cronologia:**
1. Implementação inicial com cubo e câmera look-at básico
2. Adição da pilha de matrizes modelo e cenário básico
3. Implementação de carregamento de .obj e modelos de inimigos
4. Expansão para proto-arena com colisões de plano e câmera primeira pessoa
5. **Apresentação parcial:** feedback e orientações do professor
6. Implementação de texturas e iluminação completa (difusa + Phong, Gouraud + por pixel)
7. Modelo do player composto (arqueira + varinha) com matrizes empilhadas
8. Arena completa com lógica de jogo (condições de vitória/derrota)
9. Introdução do boss dragão e sua lógica de ataque
10. Refinamento de colisões e curvas de Bézier Hermite
11. Adição de power-ups, modos de dificuldade e efeitos sonoros
12. Polimento de menus e interface

---

## Vídeo de Apresentação

[Link para o vídeo no YouTube - A SER ADICIONADO]
