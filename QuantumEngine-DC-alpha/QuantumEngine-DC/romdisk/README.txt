Pasta "res" — arquivos internos do Quantum Engine (Dreamcast)
=============================================================

Esta pasta funciona EXATAMENTE como o classpath de recursos do
projeto Eclipse original: qualquer arquivo aqui e' acessivel pelo
engine com um caminho iniciando em "/".

  Eclipse:   getResourceAsStream("/setting.txt")
  Dreamcast: res/setting.txt  ->  romdisk  ->  /rd/setting.txt

O Makefile copia "res/" para "romdisk/" no momento do build e
gera a imagem romdisk embutida no .elf. Portanto:

  - Edite os arquivos do jogo AQUI (em res/).
  - Rode "make" — o conteudo e' reempacotado automaticamente.

Conteudo atual (alpha):
  setting.txt              configuracao global
  font.txt                 config de fonte (stub — fonte embutida no alpha)
  languages/languages.txt  lista de idiomas
  languages/english.txt    strings de interface em ingles
