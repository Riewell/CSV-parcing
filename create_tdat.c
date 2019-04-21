/*  create_tdat.c

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

//~ #include <stdlib.h>
//~ #include <stdio.h>
//~ #include <string.h>
//~ #include <ctype.h>
//~ 
//~ //Название для системного файла, хранящего названия последних обработанных временных файлов
//~ #define DATA_FILE "files.dat"
//~ //Разделитель, используемый во временных файлах
//~ //(вне зависимости от того, какой разделитель был использован в исходном файле)
//~ #define TMP_SEP ","
//~ 
//~ //Структура для связанного списка разделов и подразделов
//~ //Используется для сохранения значений в процессе их обработки до записи в файл
//~ struct sections{
		//~ char sec_name[2000];
		//~ int sec_value;
		//~ struct sections *next;
//~ };

#include "csv_parcing.h"
#include <ctype.h>

int open_file(FILE **opened_file, const char *filename, const char *extension, const char *mode);
int print_name(const char *category, char *temp_string, const char separator[0], const int offset, struct sections **section);
int count(const char *category, char *temp_string, const char separator[0], const int offset);
int next_element(struct sections **sectionPtr, const char *sec_name);

int create_temp_file(const char *price_filename, int value_col)
{
	printf("Обработка файла \"%s\"...\n", price_filename);
	//Открытие файла
	FILE *price_file=NULL;
	//~ int string_lenght_1=strlen("Возникла проблема при открытии файла \n ");
	//~ int string_lenght_2=strlen(price_filename);
	//~ char *error_message=calloc(string_lenght_1+string_lenght_2*2+1, 1);
	//~ snprintf(error_message, string_lenght_1+string_lenght_2+1, "Возникла проблема при открытии файла \"%s\"\n %s", price_filename, price_filename);
	//~ if (!(price_file = fopen(price_filename, "r")))
	if (open_file(&price_file, price_filename, ".csv", "r"))
	{
		//~ perror(error_message);
		puts("Переход к следующему файлу...");
		//~ free(error_message);
		return 1;
	}
	//~ free(error_message);
	//~ error_message=NULL;
	//Открытие служебного файла, в который записывается информация о последних обработанных файлах
	//(если служебный файл не существует - он будет создан)
	FILE *dat_file=NULL;
	int string_lenght_1=strlen("Возникла проблема при открытии служебного файла \n ");
	int string_lenght_2=strlen(DATA_FILE);
	char *error_message=calloc(string_lenght_1+string_lenght_2*2+1, 1);
	snprintf(error_message, string_lenght_1+string_lenght_2+1, "Возникла проблема при открытии файла \"%s\"\n %s", DATA_FILE, DATA_FILE);
	//Если системный файл не удаётся открыть для чтения - выполняется попытка его создания,
	//после чего будет снова предпринята попытка его открытия в режиме для чтения
	if (!(dat_file = fopen(DATA_FILE, "r")))
	{
		if (!(dat_file = fopen(DATA_FILE, "w")))
		{
			perror(error_message);
			free(error_message);
			fclose(price_file);
			return 3;
		}
		fprintf(dat_file, "last_temp_file 0\n");
		fprintf(dat_file, "last_average_file 0\n");
		fprintf(dat_file, "last_result_file 0\n");
		fclose(dat_file);
		if (!(dat_file = fopen(DATA_FILE, "r")))
		{
			perror(error_message);
			free(error_message);
			fclose(price_file);
			return 3;
		}
	}
	char *tmp_filename=calloc(100, 1);
	do
	{
		fgets(tmp_filename, 100, dat_file);
		if (!strncmp(tmp_filename, "last_temp_file", 14))
		{
			int before_separator=strcspn(tmp_filename, " ");
			if (before_separator < strlen(tmp_filename))
			{
				char *temp_string=calloc(strlen(tmp_filename)-before_separator-1, 1);
				//Копирование подстроки с номером следующего файла, без промежуточного пробела и символа перевода строки
				strncpy(temp_string, &tmp_filename[before_separator+1], (strlen(tmp_filename)-before_separator-2));
				memset(tmp_filename, 0, 100);
				strncpy(tmp_filename, temp_string, strlen(temp_string));
				//TODO: переделать на strtol
				int tmp_name=atoi(tmp_filename);
				tmp_name++;
				memset(tmp_filename, 0, 100);
				snprintf(tmp_filename, 100, "%d.%s", tmp_name, "tdat");
				free(temp_string);
				temp_string=NULL;
				break;
			}
		}
	} while (!feof(dat_file));
	free(error_message);
	error_message=NULL;
	if (!strncmp(tmp_filename, "", 1))
		strncpy(tmp_filename, "1.tdat", 6);
	//Открытие временного файла, в который будут записываться промежуточные результаты подсчётов
	FILE *temp_file=NULL;
	//~ char *tmp_filename=calloc(strlen(price_filename)+strlen(".tmp")+1,1);
	//~ snprintf(tmp_filename, strlen(price_filename)+strlen(".tmp")+1, "%s.tmp", price_filename);
	string_lenght_1=strlen("Возникла проблема при открытии временного файла \n ");
	error_message=calloc(string_lenght_1+1, 1);
	snprintf(error_message, string_lenght_1+1, "Возникла проблема при открытии временного файла \"%s\"\n %s", tmp_filename, tmp_filename);
	if ((temp_file = fopen(tmp_filename, "r")))
	{
		char answer[10];
		printf("Данный файл уже был обработан - продолжение уничтожит все данные в нём.\nПродолжить обработку [Y/n]? ");
		fgets(answer, 10, stdin);
		if (!strncmp(answer, "n", 1) || !strncmp(answer, "N", 1) || !strncmp(answer, "no", 2) \
		 || !strncmp(answer, "No", 2) || !strncmp(answer, "NO", 2) || \
		  !strncmp(answer, "н", 2) || !strncmp(answer, "Н", 2) || !strncmp(answer, "нет", 6) \
		  || !strncmp(answer, "Нет", 6) || !strncmp(answer, "НЕТ", 6))
		{
			free(error_message);
			free(tmp_filename);
			fclose(dat_file);
			fclose(temp_file);
			fclose(price_file);
			return 0;
		}
		fclose(temp_file);
		temp_file=NULL;
	}
	if (!(temp_file = fopen(tmp_filename, "w")))
	{
		perror(error_message);
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		free(error_message);
		fclose(dat_file);
		free(tmp_filename);
		fclose(price_file);
		return 3;
	}
	//~ free(tmp_filename);
	free(error_message);
	//~ tmp_filename=NULL;
	error_message=NULL;
	char *category=calloc(2000, 1);
	if (!category)
	{
		puts("Проблема с памятью во время чтения файла прайса");
		fclose(price_file);
		fclose(temp_file);
		return 2;
	}
	//Уточнение символа разделителя (',' или ';' - разные офисные пакеты сохраняют по разному)
	//~ char separator[0]=0;
	char separator[2]={0};
	while (!separator[0] && !feof(price_file))
	{
		//Предположительно, 2000 символов должно хватить на строку в прайсе
		//(самая большая замеченная строка состояла из ~380 кириллических символов, что равно ~760 байтам)
		fgets(category, 2000, price_file);
		//Если первые четыре символа строки - знаки одинаковые пунктуации или табуляции - они считается разделителями
		//TODO: добавить возможность изменять строку/слово/символы для сравнения
		if (((ispunct(category[0]) && ispunct(category[1]) && ispunct(category[2]) && ispunct(category[3])) && \
		((category[0] == category[1]) && (category[1] == category[2]) && (category[2] == category[3]))) || \
		((category[0] == '\t') && (category[1] == '\t') && (category[2] == '\t') && (category[3] == '\t')))
		{
			separator[0]=category[0];
		}
	}
	if (!separator[0])
	{
		puts("Не могу прочитать файл - проблема с разделителями");
		puts("Поддерживаются только файлы с разделителями ',' и ';'");
		free(category);
		category=NULL;
		fclose(dat_file);
		fclose(price_file);
		fclose(temp_file);
		//Удаление временного файла, во избежание конфликтов имён при следующих запусках с другими данными
		remove(tmp_filename);
		free(tmp_filename);
		tmp_filename=NULL;
		return 1;
	}
	free(tmp_filename);
	tmp_filename=NULL;
	//Основной цикл
	char *temp_string=calloc(2000, 1);
	if (!temp_string)
	{
		puts("Проблема с памятью во время чтения файла прайса");
		free(category);
		category=NULL;
		fclose(dat_file);
		fclose(price_file);
		fclose(temp_file);
		return 2;
	}
	struct sections *catPtr=NULL;
	struct sections *subPtr=NULL;
	struct sections *last_subPtr=NULL;
	int cat_count=0; //счётчик для категорий
	int sub_count=-1; //счётчик для подкатегорий; отрицательная инициализация на случай остутствия подкатегории
	int unit_count=0; //общий счётчик для номенклатурных позиций
	while (!feof(price_file))
	{
		memset(category, 0, 2000);
		memset(temp_string, 0, 2000);
		struct sections *last_sectionPtr=NULL;
		fgets(category, 2000, price_file);
		int offset=0;
		//При использовании в качестве разделителя ',' остальные запятые обрамляются в двойные кавычки -
		//исключаем их из сравнения
		if (category[0] == '"')
			offset++;
		//~ //Поиск названия раздела по шаблону "ЦифраЦифраТочкаПробел"
		//~ if (isdigit(category[offset]) && isdigit(category[offset+1]) && category[offset+2] == '.' && category[offset+3] == ' ')
		//Поиск названия раздела по шаблону "ЦифраТочкаПробел" - x0._x,
		//при условии, что найденная подстрока находится до первого разделителя
		int before_point=strcspn(category, ".");
		if (isdigit(category[before_point-1]) && (category[before_point+1] == ' ') && (before_point < strcspn(category, &separator[0])))
		{
			if (cat_count)
			{
				if (sub_count > 0)
				{
					//~ printf("(%d позиций в подгруппе)\n", sub_count);
					//Если следующая группа не будет содержать подгрупп - этот счётчик будет исключён из подсчёта
					sub_count=-1;
				}
				//~ printf("(%d позиций в группе)\n", cat_count);
				cat_count=0;
			}
			if (catPtr)
			{
				int length=strlen(catPtr->sec_name);
				//Обработка разделителей (запятые/точки с запятой) и кавычек в названиях групп и подгрупп
				if (strcspn(catPtr->sec_name, TMP_SEP) < length || strcspn(catPtr->sec_name, "\"") < length)
				{
					fprintf(temp_file, "\"");
					fprintf(temp_file, "%s\"%s", catPtr->sec_name, TMP_SEP);
					fprintf(temp_file, "%d\n", catPtr->sec_value);
				}
				else
					fprintf(temp_file, "%s%s%d\n", catPtr->sec_name, TMP_SEP, catPtr->sec_value);
				memset(catPtr, 0, sizeof(struct sections));
				free(catPtr);
				catPtr=NULL;
				last_subPtr=NULL;
				if (subPtr)
				{
					last_sectionPtr=subPtr;
					while (last_sectionPtr)
					{
						length=strlen(last_sectionPtr->sec_name);
						if (strcspn(last_sectionPtr->sec_name, TMP_SEP) < length || strcspn(last_sectionPtr->sec_name, "\"") < length)
						{
							fprintf(temp_file, "\"");
							fprintf(temp_file, "%s\"%s", last_sectionPtr->sec_name, TMP_SEP);
							fprintf(temp_file, "%d\n", last_sectionPtr->sec_value);
						}
						else
							fprintf(temp_file, "%s%s%d\n", last_sectionPtr->sec_name, TMP_SEP, last_sectionPtr->sec_value);
						last_sectionPtr=last_sectionPtr->next;
						memset(subPtr, 0, sizeof(struct sections));
						free(subPtr);
						subPtr=last_sectionPtr;
					}
				}
			}
				
			if (print_name(category, temp_string, &separator[0], offset, &catPtr))
				return 2;
			continue;
		}
		//~ //Поиск названия подраздела по шаблону "ЦифраЦифраТочкаЦифраЦифраПробел"
		//~ if (isdigit(category[offset]) && isdigit(category[offset+1]) && category[offset+2] == '.' && isdigit(category[offset+3]) && isdigit(category[offset+4]) && category[offset+5] == ' ')
		//Поиск названия подраздела по шаблону "ЦифраТочкаЦифраЦифраПробел" - x0.00_x,
		//при условии, что найденная подстрока находится до первого разделителя
		if (isdigit(category[before_point-1]) && (strcspn(&category[before_point+1], " ") < strcspn(&category[before_point+1], ".")) && (before_point < strcspn(category, &separator[0])))
		{
			if (!strncmp(category, "0.08 ", 5))
				puts("!");
			//Если счётчик был ранее исключён - возвращаем его в строй
			if (sub_count < 0)
				sub_count=0;
			if (sub_count)
			{
				//~ printf("(%d позиций в подгруппе)\n", sub_count);
				sub_count=0;
			}
			if (print_name(category, temp_string, &separator[0], offset, &subPtr))
				return 2;
			last_sectionPtr=subPtr;
			while (last_sectionPtr->next)
			{
				last_sectionPtr=last_sectionPtr->next;
			}
			last_subPtr=last_sectionPtr;
			continue;
		}
		//Поиск следующей номенклатурной позиции по коду
		//Код находится в начале строки и содержит только цифры
		int before_separator=0;
		long int unit_code=0; //для предотвращения переполнения на случай очень сильно распухшей номенклатуры
		before_separator=strcspn(&category[offset], &separator[0]);
		if (before_separator)
		{
			char *tile_string=calloc(2000, 1);
			strncpy(temp_string, &category[offset], before_separator);
			//Два преобразования: сначала в long int, затем результат - обратно в строку
			//Если изначальная и конечная строки совпадают - преобразование прошло успешно,
			//код (соответственно, и строка с номенклатурой) найден
			//В противном случае осуществляется переход к следуйшей строке
			unit_code=strtol(temp_string, NULL, 10);
			snprintf(tile_string, 2000, "%li", unit_code);
			if (strcmp(temp_string, tile_string))
			{
				//Для обхода ошибок в случае нахождения кодов, начинающихся с нуля
				//(например "0060" или "00-00001270")
				//Если в найденной ранее последовательности символов нет ничего кроме цифр и дефиса,
				//данная последовательность считается действительным кодом
				//В противном случае переменной unit_code присваивается отрицательное значение и цикл прерывается
				for (int i=0; i<strlen(temp_string); i++)
				{
					if (!(isdigit(temp_string[i])) && temp_string[i] != '-')
					{
						free(tile_string);
						tile_string=NULL;
						//~ continue;
						unit_code=-1;
						break;
					}
				}
				//FIXME: для обхода описаной выше проблемы (в результате предыдущих преобразований
				//значение переменной unit_code равно "0")
				if (unit_code < 0)
					unit_code=0;
				else if (!unit_code)
					unit_code=1;
			}
			free(tile_string);
			tile_string=NULL;
		}
		//Если код товара найден - обработка строки продолжается
		if (unit_code)
		{
			//~ if (unit_code == 60886)
				//~ puts("!");
			//Колонка остатков
			//TODO: перенести в настройки
			//~ int value_col=6;
			for (int i=1; i < value_col-1; i++) //колонка кода уже пройдена, поиск начала колонки остатков
			{
				//~ printf("%s\n", &category[before_separator]);
				//Обработка запятых, не являющихся разделителями (текст с такими запятыми обрамлён двойными кавычками)
				if (category[before_separator+1] == '\"')
				{
					//Считаются символы до закрывающей кавычки,
					//затем прибавляются отсуп после открывающей кавычки и символ разделителя (запятой)
					int quote_count=0;
					do
					{
						before_separator++;
						if (category[before_separator] == '\"')
							quote_count++;
						//Если наименование растянуто на две строки - переход на следующую строку с сохранением текущего состояния
						if (category[before_separator] =='\n')
						{
							memset(category, 0, 2000);
							fgets(category, 2000, price_file);
							before_separator=0;
							//~ printf("%s\n", &category[before_separator]);
						}
					}
					//Для обхода наименований вроде "Some ""THING"" " (несколько кавычек подряд (предположительно, парных)
					while (category[before_separator] != separator[0] || quote_count%2);
				}
				else
				{
					before_separator=before_separator+strcspn(&category[before_separator+1], &separator[0])+1; //отсуп после разделителя
					//Для обхода непонятной ошибки со "сменой" разделителя в функции выше на пробел
					//(проявляется при растянутости наименования предыдущей или предпредыдущей позиции на две строки)
					//FIXME: найти причину ошибки, если возможно - исправить
					if (category[before_separator] != separator[0])
					{
						do
						{
							before_separator++;
						}
						while (category[before_separator] != separator[0]);
					}
				}
			}
			if (category[before_separator+1] == '\"')
				offset=1;
			else
				offset=0;
			//Сложение количественных остатков в данной строке с предыдущими
			int value=0;
			value=count(&category[before_separator+1], temp_string, &separator[0], offset);
			unit_count=unit_count+value;
			//Сложение со счётчиком подкатегорий производится только если счётчик не был ранее исключён из подсчётов
			//(в случае отсутствия подгрупп в данной д=группе товаров)
			if (sub_count >= 0)
				sub_count=sub_count+value;
			cat_count=cat_count+value;
			catPtr->sec_value=catPtr->sec_value+value;
			if (last_subPtr)
				last_subPtr->sec_value=last_subPtr->sec_value+value;
		}
	}
	//Печать результата производится только если счётчик не пуст и не исключён
	if (sub_count > 0)
		//~ printf("(%d позиций в подгруппе)\n", sub_count);
	//~ printf("(%d позиций в группе)\n", cat_count);
	//~ printf("Обработка файла завершена (\"%s\")\nВсего %d позиций\n", price_filename, unit_count);
	printf("Обработка файла завершена (\"%s\")\nВсего %d позиций\n", price_filename, unit_count);
	free(category);
	category=NULL;
	//~ catPtr->sec_value=catPtr->sec_value+value;
	int length=strlen(catPtr->sec_name);
	if (strcspn(catPtr->sec_name, TMP_SEP) < length || strcspn(catPtr->sec_name, "\"") < length)
	{
		fprintf(temp_file, "\"");
		fprintf(temp_file, "%s\"%s", catPtr->sec_name, TMP_SEP);
		fprintf(temp_file, "%d\n", catPtr->sec_value);
	}
	else
		fprintf(temp_file, "%s%s%d\n", catPtr->sec_name, TMP_SEP, catPtr->sec_value);
	memset(catPtr, 0, sizeof(struct sections));
	free(catPtr);
	catPtr=NULL;
	if (subPtr)
	{
		struct sections *last_sectionPtr=NULL;
		last_sectionPtr=subPtr;
		while (last_sectionPtr)
		{
			length=strlen(last_sectionPtr->sec_name);
			if (strcspn(last_sectionPtr->sec_name, TMP_SEP) < length || strcspn(last_sectionPtr->sec_name, "\"") < length)
			{
				fprintf(temp_file, "\"");
				fprintf(temp_file, "%s\"%s", last_sectionPtr->sec_name, TMP_SEP);
				fprintf(temp_file, "%d\n", last_sectionPtr->sec_value);
			}
			else
				fprintf(temp_file, "%s%s%d\n", last_sectionPtr->sec_name, TMP_SEP, last_sectionPtr->sec_value);
			last_sectionPtr=last_sectionPtr->next;
			memset(subPtr, 0, sizeof(struct sections));
			free(subPtr);
			subPtr=last_sectionPtr;
		}
	}
	fclose(price_file);
	if (fclose(temp_file))
	{
		perror("Ошибка записи временного файла");
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		return 3;
	}
	if (fclose(dat_file))
	{
		perror("Ошибка обработки системного файла");
		free(temp_string);
		fclose(temp_file);
		return 3;
	}
	dat_file=NULL;
	char *backup_filename=calloc(strlen(DATA_FILE)+strlen(".bak")+1,1);
	snprintf(backup_filename, strlen(DATA_FILE)+strlen(".bak")+1, "%s.bak", DATA_FILE);
	if (rename(DATA_FILE, backup_filename))
	{
		perror("Ошибка обработки системного файла");
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		free(backup_filename);
		free(temp_string);
		fclose(temp_file);
		return 3;
	}
	FILE *tmp;
	tmp=NULL;
	if (!(tmp = fopen(backup_filename, "r")))
	{
		perror("Ошибка обработки системного файла");
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		free(backup_filename);
		free(temp_string);
		fclose(temp_file);
		return 3;
	}
	if (!(dat_file = fopen(DATA_FILE, "w")))
	{
		perror("Ошибка обработки системного файла");
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		free(backup_filename);
		free(temp_string);
		fclose(tmp);
		fclose(temp_file);
		return 3;
	}
	while (!feof(tmp))
	{
		fgets(temp_string, 100, tmp);
		if ((!strncmp(temp_string, "last_temp_file", 14)) && !feof(tmp))
		{
			fprintf(dat_file, "last_temp_file %d\n", atoi(&temp_string[14])+1);
			printf("Результат записан в файл: %d.tdat\n", atoi(&temp_string[14])+1);
			continue;
		}
		if (!feof(tmp))
			fprintf(dat_file, "%s", temp_string);
	}
	free(temp_string);
	temp_string=NULL;
	fclose(tmp);
	tmp=NULL;
	if (remove(backup_filename))
	{
		perror(error_message);
	}
	free(backup_filename);
	backup_filename=NULL;
	if (fclose(dat_file))
	{
		perror("Ошибка обновления системного файла");
		puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
		fclose(temp_file);
		return 3;
	}
	return 0;
}

//Вывод названия категории/подкатегории
//TODO: реализовать вывод в файл
int print_name(const char *category, char *temp_string, const char separator[0], const int offset, struct sections **section)
{
	int length;
	//На вывод отправляется только требуемая колонка, остальные символы в строке отбрасываются
	if (offset)
		length=strcspn(&category[offset], "\"");
	else
		length=strcspn(&category[offset], &separator[0]);
	strncpy(temp_string, &category[offset], length);
	temp_string[length]='\000';
	//~ printf("%s\n", temp_string);
	next_element(*(&section), temp_string);
	return 0;
}

//Обработка количественных остатков - обрезание лишних символов и удаление пробелов между цифрами
int count(const char *category, char *temp_string, const char separator[0], const int offset)
{
	int length;
	//На вывод отправляется только требуемая колонка, остальные символы в строке отбрасываются
	if (offset)
		length=strcspn(&category[offset], "\"");
	else
		length=strcspn(&category[offset], &separator[0]);
	strncpy(temp_string, &category[offset], length);
	temp_string[length]='\000';
	//Удаление пробелов между цифрами
	for (int i=0; i<length; i++)
	{
		if (!isdigit(temp_string[i]) && temp_string[i] != '\000')
		{
			char *temp_space1=calloc(length, 1);
			char *temp_space2=calloc(length, 1);
			strncpy(temp_space1, temp_string, i);
			strncpy(temp_space2, &temp_string[i+1], length-i-1);
			memset(temp_string, 0, 2000);
			strncpy(temp_string, temp_space1, i);
			strcat(temp_string, temp_space2);
			free(temp_space1);
			free(temp_space2);
			temp_space1=NULL;
			temp_space2=NULL;
			i--;
		}
	}
	//~ printf("%s\n", temp_string);
	length=atoi(temp_string); //TODO: добавить проверку успешности преобразования
	//~ printf("%d\n", length);
	return length;
}

//Функция для добавления названий и значений в связанный список (массив структур) разделов/подразделов
int next_element(struct sections **sectionPtr, const char *sec_name)
{
	struct sections *new_sectionPtr=NULL;
	if (!(new_sectionPtr=calloc(1, sizeof(struct sections))))
	{
		perror("Проблема с памятью - остановка обработки");
		return 2;
	}
	strcpy(new_sectionPtr->sec_name, sec_name);
	//~ new_sectionPtr->sec_value=value;
	new_sectionPtr->next=NULL;
	if (!(*sectionPtr))
		*sectionPtr=new_sectionPtr;
	else
	{
		struct sections *last_sectionPtr=NULL;
		last_sectionPtr=*sectionPtr;
		while (last_sectionPtr->next)
		{
			last_sectionPtr=last_sectionPtr->next;
		}
		last_sectionPtr->next=new_sectionPtr;
	}
	return 0;
}

//Проверка параметров, заданных пользователем
//Функция возвращает или введённое пользователм значение, либо "-2" в качестве ошибки
//Обработка корректности непосредственно самих значений производится на уровне обработки параметров
short int parameters_check(const char *parameter_name, const char *parameter_value, const short int default_value, const short int *parameter)
{
	//Защита от значений, превышающих размер short int, а также от нечисловых символов
	long int temp_int;
	char *tile_string[100]={""};
	temp_int=strtol(parameter_value, tile_string, 10);
	if ((!temp_int) && (parameter_value == *tile_string))
	{
		printf("Значение '%s' параметра '%s' непонятно. ", parameter_value, parameter_name);
		if (*parameter == default_value)
			printf("Установлено значение по умолчанию: %i.\n", default_value);
		else
			printf("Значение не изменено (%hi).\n", *parameter);
		return -2;
	}
	short int value=temp_int;
	//Если в параметрах передано дробное число/мусор -
	//такие значения будут усечены до целых чисел/исправлены на нули или значения по умолчанию
	char value_string[10]; //в 10 cимволов должно уместиться достаточно большое число
	snprintf(value_string, 10, "%hi", value);
	if (strcmp(value_string, parameter_value))
	{
		printf("Значение '%s' параметра '%s' непонятно. ", parameter_value, parameter_name);
		if (*parameter == default_value)
			printf("Установлено значение по умолчанию: %i.\n", default_value);
		else
			printf("Значение не изменено (%hi).\n", *parameter);
		return -2;
	}
	return value;
}
