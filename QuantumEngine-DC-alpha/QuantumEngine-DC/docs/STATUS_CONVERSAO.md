# Status da Conversão — Quantum Engine J2ME → Dreamcast (KallistiOS)

Documento honesto do que **já** foi convertido para C e do que **falta**.
O engine original tem **109 arquivos Java / ~34.500 linhas**. Este port é
por fases, priorizando a lógica que destrava conteúdo real (menu, idiomas,
fontes, config, carregamento de modelos `.3d`, sons).

---

## ✅ CONVERTIDO E VALIDADO (host)

### Núcleo / plataforma
| Java | C | Nota |
|---|---|---|
| `utils/Main` (boot/loop) | `src/main.c` | boot, loop, integra tudo |
| `utils/QFPS` + `FPS` | `src/core/qfps.c` | clamp `frameTime<0→0` preservado |
| `utils/IniFile` | `src/config/inifile.c` | parser chave=valor |
| `utils/Asset` | `src/utils_c/asset.c` | **cache de meshes/tex/sons/imagens** |
| (Sound J2ME) | `src/utils_c/sound.c` | **áudio via AICA/KOS (snd_sfx)** |
| `utils/StringTools` (resource) | `src/core/res.c` | `/x` → `/rd/x` (romdisk) |

### Matemática (100% fiel, fixed-point)
| Java | C |
|---|---|
| `Math/Matrix` (fp=14, sin/cos graus) | `src/math3d/matrix.c` |
| `Math/Vector3D` | `src/math3d/vector3d.c` |
| `Math/MathUtils` (calcLight) | `src/math3d/vector3d.c` |
| `Math/MathUtils2` (invSqrt) | `src/math3d/vector3d.c` |

### Renderização de software (núcleo)
| Java | C | Nota |
|---|---|---|
| `Rendering/DirectX7` | `src/render3d/directx7.c` | fov, camera, finalMatrix |
| `Rendering/Vertex` | `src/render3d/directx7.c` | transform + project fiéis |
| `Rendering/Texture` | `src/render3d/directx7.c` | widthBIT (potência de 2) |
| `Rendering/TexturingAffine` | `src/render3d/texturing_affine.c` | **paintAffine_0 + line_0 + modo 9 luz** |
| `Rendering/RenderObject` | `src/render3d/polygon.c` | normais, getLight |
| `Meshes/Polygon3V` / `Polygon4V` | `src/render3d/polygon.c` | isVisible, render |
| `Rendering/RenderObjectBuffer` | `src/render3d/renderbuffer.c` | painter's sort |
| `Rendering/Camera` | `src/render3d/renderbuffer.c` | interpolação suave |

### Colisão (NOVO — 100% fiel, validado no host)
| Java | C | Nota |
|---|---|---|
| `Collision/Ray` | `src/collision/ray.c` | defaults fiéis (onlyCollidable=true) |
| `Collision/RayCast` | `src/collision/raycast.c` | superFast + AABB + recalc de normal + isRayOnPolygon |
| `Collision/SphereCast` | `src/collision/spherecast.c` | empurra pos p/ fora (−nor·dis≫12) |
| `Collision/Height` | `src/collision/height.c` | reset NÃO zera cy (fiel) |
| `Collision/HeightComputer` | `src/collision/heightcomputer.c` | underRoof (ny>2048), transformReturn comentado como no Java |
| `Math/MathUtils2` (geometria) | `src/math3d/mathutils2.c` | isPointOnPolygon (todas as variantes), distanceToLine/Ray/Face, computePolygonY, side, calcNormal — com semântica de int do Java (jmul/jd2i) |
| `DirectX7.transformSave/Return` | `src/render3d/directx7.c` | transformReturn portado LITERAL (ver comentário no código) |

Correções de fidelidade aplicadas na base:
- `invSqrt`: constante corrigida para **1597463007 (0x5F375A86)** — a do Java —
  no lugar da 0x5f3759df (Quake) que estava em `vector3d.c`.
- `Texture3` ganhou `castShadow` e `collision` (default `true`), usados pelos
  filtros de RayCast/SphereCast/HeightComputer — como em `Texture.java`.
- Teste funcional: `tests/host_collision_test.c` (19 casos, todos passando).

### Meshes / modelos
| Java | C | Nota |
|---|---|---|
| `Meshes/Mesh` + `Room.loadMeshes`/`createFrom3d` | `src/meshes/mesh.c` | **loader `.3d` big-endian FIEL (validado: lê verts/tri/quad certo)** |
| `Rendering/MultyTexture` | `src/meshes/multytexture.c` | multi-textura por índice |

### Telas / HUD
| Java | C |
|---|---|
| `HUD/Base/Font` | `src/render/dc_font.c` (PNG) + `font.c` (8×8 fallback) |
| `HUD/SplashScreen` | `src/screens/splash.c` |
| `HUD/Menu` + `Selectable` + `ItemList` | `src/screens/menu.c` |
| `utils/canvas/MainCanvas` + `MyCanvas` | `src/screens/screen.h` + `screenmgr.c` |
| `Gameplay/GameScreen` (parcial: FPS + cena teste) | `src/screens/gamescreen.c` |
| `HUD/Base/GameKeyboard` | `src/core/input.c` (controle DC → FPS) |
| `Rendering/RawImage` | `src/render/raw_image.c` (PNG via stb_image) |

---

## 🐛 BUG CONHECIDO (a resolver com prints do Dreamcast)

**Modelos `.3d` carregam mas não aparecem na tela.**
Diagnóstico do host: os vértices projetam com `rz` **positivo** (ex.: 1060,
1853), mas o engine espera `rz` **negativo** na frente da câmera (o culling
rejeita `rz > 0`). É um bug de **sinal de Z no setup de câmera/projeção** —
NÃO do loader (a geometria é lida corretamente: 8 verts, 6 quads).

Provável causa: a convenção de sinal entre `Camera.set` (translate x,y,z) e
`Vertex.project` (usa `-rz`). Isso resolve ajustando o sinal de Z da posição
da câmera OU a ordem de multiplicação da matriz — exatamente o tipo de ajuste
que faremos com um print/log do Flycast. A cena de teste do `gamescreen`
(chão + cubos proceduraris) usa o mesmo pipeline e serve de bancada.

---

## ⏳ AINDA NÃO CONVERTIDO (próximas fases)

### Rasterizadores adicionais (fog/efeitos)
- `TexturingAffine` modos 1,2,3,4,7,8,10,11,13 (fog blend, glass, dither, RGB)
  → hoje caem no modo 0/9. `TexturingPers` (perspectiva), `TexturingFloors`.
- `Meshes/LightedPolygon3V/4V`, `ColorLightedPolygon3V/4V` (luz por vértice)
- `Rendering/NearClipper` (clipping do plano near)
- `Rendering/ByteRawImage`, `TextureCompressor`, `ImageResize` (mipmaps)

### Mapas / mundo
- `Gameplay/Map/LevelLoader` (1579 linhas — carrega `levelN.txt`)
- `Gameplay/Map/House`, `Room`, `Portal`, `Scene`, `Skybox`
- `Gameplay/Map/LightMapper` (1712 linhas), `Light`, `Dijkstra`
- `Gameplay/Map/RoomObject`, `Character`, `HouseCreator`

### Gameplay
- `Gameplay/GameScreen` (1190 linhas — loop de jogo completo)
- `Weapon`, `Arsenal`, `Magazine`, `Shop`, `Respawn`
- `Gameplay/Objects/*` (MeshObject, NPCSpawner, Teleport, LVLChange, etc.)
- `Gameplay/Inventory/*`, `InventoryScreen`, `DialogScreen`

### IA
- `AI/Player` (478), `NPC` (443), `Bot` (317), `Zombie`, `BigZombie`, `TPPose`
- `AI/misc/*` (Blood, Corpse, Splinter, Trace)

### HUD restante
- `PauseScreen`, `Setting`, `KeysSettings`, `DialogScreen`, `TitleScreen`,
  `GameHelp`, `Help`, `LevelSelection`, `LoadingScreen`, `DeveloperMenu`,
  `Benchmark`, `VideoPlayer`, `TextView`, `HUDInfo`
- `Gameplay/WeatherGenerator`, `utils/WeaponCreator`, `DelayedDialog`

### Animação de mesh
- `Meshes/Morphing`, `MeshClone`, `MeshImage`, `Sprite` (876), `BoundingBox`

### NÃO será convertido (por decisão do projeto)
- `Meshes/M3GParser` — o Quantum usa modelos `.3d` com **render por software**,
  não M3G. (Confirmado pelo usuário.)

---

## Como testar o que já funciona

```bash
source /opt/toolchains/dc/kos/environ.sh
cd QuantumEngine-DC
make clean && make && make cdi
flycast quantum.cdi 2>&1 | tee quantum_log.txt
```

Funciona hoje: boot → splash → menu (idiomas via `res/languages/`) →
"New Game" abre a cena FPS 3D (chão + cubos, pipeline fiel). Os modelos `.3d`
de `res/meshes/` carregam (veja o log `[QE] mesh .3d: ...`), mas ainda não
desenham por causa do bug de sinal de Z acima.

**Mande prints/logs** e resolvemos os bugs 3D e as próximas fases um a um.
