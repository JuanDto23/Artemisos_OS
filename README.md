# Artemisos_OS
Proyecto de simulador de un sistema operativo del curso de Lab. de Sistemas Operativos 2025-2025.

# Compilación
Para la compilación del proyecto, se requiere previamente la instalación de la biblioteca de ncurses.
Para la instalación de ncurses en Linux, puede visitar el siguiente enlace: 
(https://www.cyberciti.biz/faq/linux-install-ncurses-library-headers-on-debian-ubuntu-centos-fedora/)

El comando de compilación, utilizando el compilador de C y C++ del proyecto GNU, es:
```gcc main.c pcb.c queue.c gui.c memory.c prompt.c utils.c -o pcb.out -Wall -lncurses -lm```
