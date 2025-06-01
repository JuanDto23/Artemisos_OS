#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
// Bibliotecas propias
#include "utils.h"

// Convierte a mayúsculas una cadena
void str_upper(char *s)
{
  // Recorre la cadena
  for (int i = 0; s[i] != '\0'; i++)
  {
    // Si el carácter es una letra minúscula
    if (s[i] >= 'a' && s[i] <= 'z')
    {
      s[i] = s[i] - 32; // Convierte a mayúscula
    }
  }
}

// Verifica si una cadena es numérica
int is_numeric(char *str)
{
  // Verificar si la cadena es nula
  if (str == NULL)
    return false;

  // Manejar el caso de un número negativo
  if (*str == '-')
    str++; // avanza al siguiente caracter

  // Si el siguiente carácter no es número, regresa 0
  if (*str == '\0')
    return false;

  // Verificar cada carácter de la cadena
  while (*str)
  {
    if (!isdigit(*str))
      return false; // Si se encuentra un carácter no numérico, regresar 0
    str++;
  }
  return true; // Si todos los caracteres son dígitos, regresar 1
}

// Cuenta el número de lineas igonarando líneas vacías
int count_lines(FILE *file)
{
  int lines = 0;
  int c, last = '\n';
  while ((c = fgetc(file)) != EOF)
  {
    // Dos saltos de líneas consecutivos indican una línea vacía
    if (c == '\n' && last != '\n')
      lines++;
    last = c;
  }
  rewind(file); // Regresa al inicio del archivo
  return lines;
}