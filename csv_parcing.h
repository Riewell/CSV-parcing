/*  csv_parcing.h

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Название для системного файла, хранящего названия последних обработанных временных файлов
#define DATA_FILE "files.dat"
//Разделитель, используемый во временных файлах
//(вне зависимости от того, какой разделитель был использован в исходном файле)
#define TMP_SEP ","

//Структура для связанного списка разделов и подразделов
//Используется для сохранения значений в процессе их обработки до записи в файл
struct sections{
	char sec_name[2000];
	int sec_value;
	struct sections *next;
};

struct files{
	FILE *temp_file;
	struct files *next;
};

//Структура данных, содержащая прочитанную из файла строку,
//ссылку на следующую подобную структуру и массив числовых значений данной строки,
//взятых из обрабатываемых файлов
struct rawdata{
	char name[2000];
	struct rawdata *next;
	int values[];
};
