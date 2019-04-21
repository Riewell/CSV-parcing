/*  help.c

  Обработка нескольких файлов csv для получения средних
  количественных остатков в определённой колонке/столбце.
  Version 0.8

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

void print_help(const char *program_title, const int help_size)
{
	int line_length=strlen(program_title);
	int indent=0;
	
	//Вывод форматированной краткой справки (для параметра "-h")
	//Последняя строка ("[-h|--help]") обрабатывается и выводится отдельно, а также служит признаком конца массива
	char *help_strings[]={"[-v ЗНАЧЕНИЕ|--value-column ЗНАЧЕНИЕ]",
						"[-o ФАЙЛ|--open ФАЙЛ]",
						"[-a ФАЙЛЫ|--average ФАЙЛЫ]",
						"[-c ФАЙЛЫ|--combine ФАЙЛЫ]",
						"[-h|--help]"};
	printf("\n  Использование: \n\n%s", program_title);
	//Проверка возможности использования отступа для новых строк шириной в название программы
	//Если название программы и строка помощи больше, чем 80 (ширина термина+пробел) - устанавливается значение по умолчанию (10)
	//Последнюю строку массива ("[-h|--help]") не обрабатываем - она будет выводиться в отдельной строке
	int i=0;
	while (strncmp(help_strings[i], "[-h", 3))
	{
		if (line_length+strlen(help_strings[i]) < 79)
			indent=line_length+1;
		else
			indent=10;
		i++;
	}
	i=0;
	while (strncmp(help_strings[i], "[-h", 3))
	{
		//Если название программы и первой строки справки меньше 80 (ширина терминала+пробел; задать иначе (узнать из окружения)?),
		//то первая строка справки выводится в той же строке
		//FIXME: неверно учитывается размер кириллических символов!
		line_length=line_length+strlen(help_strings[i]);
		if (line_length < 79)
		{
			printf(" %s", help_strings[i]);
		}
		//Иначе - производится перевод на новую строку с отступом в 10 пробелов и устанавливается признак перевода строки
		else
		{
			printf("\n%*s%s", indent, " ", help_strings[i]);
			line_length=strlen(help_strings[i])+indent;
		}
		i++;
	}
	//Последняя строка краткой справки ("[-h|--help]") выводится на отдельную строку
	printf("\n\n%s", program_title);
	if (strlen(program_title)+strlen(help_strings[i]) < 79)
	{
		printf(" %s", help_strings[i]);
	}
	else
	{
		printf("\n%*s%s", indent, " ", help_strings[i]);
	}
	printf("\n\n");

	//Если указан параметр "--help" - дополнительно выводится полная справка
	//TODO: перенести данные из внешнего файла сюда
	if (help_size)
	{
		FILE *help_file=NULL;
		help_file=fopen("help.txt", "r");
		if (help_file == NULL)
		{
			puts("Файл справки отсутствует или повреждён");
			return;
		}
		//char printhelp[200];
		while (!feof(help_file) && !ferror(help_file))
		{
			char printhelp[200]={'\000'};
			fgets(printhelp, 200, help_file);
			printf("%s", printhelp);
		}
		fclose(help_file);
		//~ printf("\n\n");
	}
	return;
}
