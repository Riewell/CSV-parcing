/*  csv_parcing.c

  Обработка нескольких файлов csv для получения средних
  количественных остатков в определённой колонке/столбце.
  Version 0.9.2

  Copyright 2017 Konstantin Zyryanov <post.herzog@gmail.com>
  
  This file is part of CSV parcing program.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.
*/

//~ #include <stdio.h>
//~ #include <string.h>
#include "csv_parcing.h"

void print_help(const char *program_title, const int help_size);
int create_temp_file(const char *price_file, int value_col);
int create_result(char **raw_files, int quantity, const char *extension);

int main(int argc, char **argv)
{
	//Если не программа запущена без параметров - вывод краткой справки по использованию и завершение работы
	if (argc == 1)
	{
		print_help(argv[0], 0);
		return 1;
	}
	if (argc > 1)
	{
		//Если среди параметров запуска есть требование вывода справки - вывод её и завершение работы
		for (int i = 1; i < argc; i++)
		{
			if (!strncmp(argv[i], "-h", 2))
			{
				print_help(argv[0], 0);
				return 0;
			}
			if (!strncmp(argv[i], "--help", 6))
			{
				print_help(argv[0], 1);
				return 0;
			}
		}
		for (int i = 1; i < argc; i++)
		{
			int value_col=7; //колонка с ценами по умолчанию
			if (!strncmp(argv[i], "-o", 2) || !strncmp(argv[i], "--open", 6))
			{
				if (argc == 2)
				{
					puts("Не указан файл для обработки!");
					return 1;
				}
				for (int j = 1; j < argc; j++)
				{
					if (!strncmp(argv[j], "-v", 2) || !strncmp(argv[j], "--value-column", 14))
					{
						if (j+1 == argc)
						{
							puts("Не указан столбец для обработки!");
							return 1;
						}
						long int temp_val=strtol(argv[j+1], NULL, 10);
						if (!temp_val || (temp_val > 1000))
						{
							printf("Значение '%s' параметра %s непонятно. Завершение работы.\n", argv[j+1], argv[j]);
							return 1;
						}
						value_col=(int)temp_val;
					}
				}
				int result=create_temp_file(argv[i+1], value_col);
				return result;
			}
			else if (!strncmp(argv[i], "-a", 2) || !strncmp(argv[i], "--average", 11))
			{
				if (argc == 2)
				{
					puts("Не указаны файлы для обработки!");
					return 1;
				}
				//Количество актуальных параметров-значений, которые будут переданы для дальнейшей обработки
				int quantity;
				//Название программы и первый агумент (подразумевается данная команда "-a") пропускаются
				for (quantity=2; quantity < argc; quantity++)
				{
					//Если указана ещё какая-нибудь команда (включая текущую) -
					//она и её параметры-значения будут проигнорированы и отброшены
					if ((!strncmp(argv[quantity], "-o\000", 3) || !strncmp(argv[quantity], "--open\000", 7)) || \
					(!strncmp(argv[quantity], "-a\000", 3) || !strncmp(argv[quantity], "--average\000", 7)) || \
					(!strncmp(argv[quantity], "-c\000", 3) || !strncmp(argv[quantity], "--combine\000", 10)) || \
					(!strncmp(argv[quantity], "-v\000", 3) || !strncmp(argv[quantity], "--value-column\000", 15)))
					{
						printf("Команда проигнорирована: %s.\nВсе последующие параметры не учтены и не обработаны.\n", argv[quantity]);
						break;
					}
				}
				int result=create_result(argv, quantity-2, ".adat");
				return result;
			}
			else if (!strncmp(argv[i], "-c", 2) || !strncmp(argv[i], "--combine", 9))
			{
				if (argc == 2)
				{
					puts("Не указаны файлы для обработки!");
					return 1;
				}
				//Количество актуальных параметров-значений, которые будут переданы для дальнейшей обработки
				int quantity;
				//Название программы и первый агумент (подразумевается данная команда "-a") пропускаются
				for (quantity=2; quantity < argc; quantity++)
				{
					//Если указана ещё какая-нибудь команда (включая текущую) -
					//она и её параметры-значения будут проигнорированы и отброшены
					if ((!strncmp(argv[quantity], "-o\000", 3) || !strncmp(argv[quantity], "--open\000", 7)) || \
					(!strncmp(argv[quantity], "-a\000", 3) || !strncmp(argv[quantity], "--average\000", 7)) || \
					(!strncmp(argv[quantity], "-c\000", 3) || !strncmp(argv[quantity], "--combine\000", 10)) || \
					(!strncmp(argv[quantity], "-v\000", 3) || !strncmp(argv[quantity], "--value-column\000", 15)))
					{
						printf("Команда проигнорирована: %s.\nВсе последующие параметры не учтены и не обработаны.\n", argv[quantity]);
						break;
					}
				}
				int result=create_result(argv, quantity-2, ".csv");
				return result;
				//~ return 0;
			}
			else if (!strncmp(argv[i], "-v", 2) || !strncmp(argv[i], "--value-column", 14))
			{
				if (i+1 == argc)
				{
					puts("Не указан столбец для обработки!");
					return 1;
				}
				long int temp_val=strtol(argv[i+1], NULL, 10);
				if (!temp_val || (temp_val > 1000))
				{
					printf("Значение '%s' параметра %s непонятно. Завершение работы.\n", argv[i+1], argv[i]);
					return 1;
				}
				value_col=(int)temp_val;
				i++; //для пропуска обработки значения данного параметра
			}
			else
			{
				printf("Неверный параметр запуска \"%s\"\n", argv[i]);
				print_help(argv[0], 0);
				return 0;
			}
		}
	}
	return 0;
}
