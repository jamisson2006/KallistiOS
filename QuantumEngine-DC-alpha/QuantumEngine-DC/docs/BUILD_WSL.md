# Guia de compilação — Ubuntu / WSL → .elf e .cdi → Flycast

Passo a passo completo para compilar o **Quantum Engine (Dreamcast Port)**
no Ubuntu (nativo ou WSL) e rodar no **Flycast**.

---

## 0. Pré-requisitos do sistema (Ubuntu/WSL)

```bash
sudo apt update
sudo apt install -y build-essential git wget curl python3 \
                    libpng-dev libjpeg-dev libelf-dev \
                    texinfo gawk bison flex sed pkg-config \
                    meson ninja-build cmake gettext libisofs-dev
```

---

## 1. Instalar o KallistiOS (KOS) — uma vez só

> Se você **já** tem o KOS em `/opt/toolchains/dc/kos`, pule para o passo 2.

```bash
# Diretório padrão do toolchain do Dreamcast
sudo mkdir -p /opt/toolchains/dc
sudo chown -R $USER:$USER /opt/toolchains/dc
cd /opt/toolchains/dc

# 1.1 Clonar KOS (branch estável 2.2.x) e kos-ports
git clone https://github.com/KallistiOS/KallistiOS.git kos -b master
git clone https://github.com/KallistiOS/kos-ports.git

# 1.2 Compilar o cross-compiler SH-4 (demora bastante, ~30-60 min)
cd /opt/toolchains/dc/kos/utils/dc-chain
cp Makefile.default.cfg Makefile.cfg    # edite makejobs= p/ nº de núcleos, se quiser
./download.sh
./unpack.sh
make          # compila binutils + gcc + newlib para SH-4

# 1.3 Configurar e compilar o KOS
cd /opt/toolchains/dc/kos
cp doc/environ.sh.sample environ.sh     # (ou use o environ.sh já existente)
source environ.sh
make          # compila a KallistiOS
```

Ao final, você terá `kos-cc`, `kos-c++`, `kos-elf2bin`, etc. no PATH quando
o `environ.sh` estiver carregado.

---

## 2. Ferramenta de imagem: mkdcdisc (recomendada)

O `mkdcdisc` gera o `.cdi` em um passo só (mais simples que o fluxo antigo):

```bash
cd /opt/toolchains/dc
git clone https://gitlab.com/simulant/mkdcdisc.git
cd mkdcdisc
meson setup builddir
meson compile -C builddir
sudo cp builddir/mkdcdisc /usr/local/bin/     # deixa no PATH
mkdcdisc --version
```

---

## 3. Compilar o Quantum Engine

```bash
# Sempre carregue o ambiente KOS antes de compilar:
source /opt/toolchains/dc/kos/environ.sh

# Entre na pasta do projeto (ajuste o caminho de onde você extraiu o zip)
cd ~/QuantumEngine-DC

# Limpa e compila o ELF
make clean
make            # -> gera quantum.elf

# Gera a imagem .cdi para o Flycast
make cdi        # -> gera quantum.cdi
```

Se tudo der certo você verá `quantum.elf` e `quantum.cdi` na pasta.

> **Editou algo em `res/`?** Basta rodar `make` de novo — a pasta é
> reempacotada no romdisk automaticamente.

---

## 4. Rodar no Flycast

Instalar o Flycast (uma opção simples é o AppImage do site oficial, ou):

```bash
# Exemplo via flatpak
flatpak install flathub org.flycast.Flycast
```

Rodar a imagem:

```bash
flycast quantum.cdi
# ou, se instalado via flatpak:
flatpak run org.flycast.Flycast quantum.cdi
```

Você deve ver a **tela de splash** (logo QUANTUM ENGINE) por alguns
segundos e então o **menu principal** (New Game / Help / Options / Exit).
Navegue com o **D-pad**, selecione com **A**, volte com **B**.

---

## 5. Capturar o log (debug)

O engine escreve logs via `printf` (visíveis no console do Flycast). Para
salvar em arquivo:

```bash
flycast quantum.cdi 2>&1 | tee quantum_log.txt
```

Procure por linhas com prefixo `[QE]`, `[QE/WARN]`, `[QE/ERR]`.

---

## 6. Alternativa: rodar o .elf direto (sem gerar .cdi)

Para iteração rápida, o Flycast também roda o `.elf` (via dc-load/ELF):

```bash
make
flycast quantum.elf
```

E o KOS tem o alvo `run` (usa o loader configurado em `$KOS_LOADER`):

```bash
make run
```

---

## Problemas comuns

| Sintoma | Causa provável | Solução |
|---|---|---|
| `kos-cc: command not found` | ambiente KOS não carregado | `source /opt/toolchains/dc/kos/environ.sh` |
| `mkdcdisc: not found` no `make cdi` | ferramenta não instalada | passo 2 (o script cai no fluxo clássico se houver `scramble`+`cdi4dc`) |
| Tela preta no Flycast | BIOS/flash ausente no Flycast | configure a BIOS do DC no Flycast, ou rode o `.elf` direto |
| `/setting.txt missing` na tela | romdisk não gerou | rode `make clean && make` (recria o romdisk a partir de `res/`) |
| Erros de RTTI/exceptions | só afetam C++; este port é C puro | não se aplica |

---

## Resumo dos comandos (cola rápida)

```bash
source /opt/toolchains/dc/kos/environ.sh
cd ~/QuantumEngine-DC
make clean && make && make cdi
flycast quantum.cdi 2>&1 | tee quantum_log.txt
```
