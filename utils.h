#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h> // Para FILE

// Función para contar el número de líneas en un archivo, ignorando líneas vacías
int count_lines(FILE *file);

// Función para convertir una cadena a mayúsculas
void str_upper(char *str);

// Función para verificar si una cadena es numérica
int is_numeric(char *str);

#endif