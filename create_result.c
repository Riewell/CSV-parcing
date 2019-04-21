/*  create_result.c

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
//~ #include <stdlib.h>
//~ 
//~ struct files{
	//~ FILE *temp_file;
	//~ struct files *next;
//~ };
//~ 
//~ //Структура данных, содержащая прочитанную из файла строку,
//~ //ссылку на следующую подобную структуру и массив числовых значений данной строки,
//~ //взятых из обрабатываемых файлов
//~ struct rawdata{
	//~ char *name[2000];
	//~ struct rd *next;
	//~ int values[];
//~ };

#include "csv_parcing.h"

int open_files(char **raw_files, const char *extension, int quantity, struct files **filePtr);
int open_file(FILE **opened_file, char *filename, const char *extension, const char *mode);
void close_files(struct files **filePtr);

int create_result(char **raw_files, int quantity, const char *extension)
{
	struct files *filePtr=NULL;
	int result=0;
	if (!strcmp(extension, ".adat"))
		result = open_files(raw_files, ".tdat", quantity, &filePtr);
	if (!strcmp(extension, ".csv"))
		result = open_files(raw_files, ".adat", quantity, &filePtr);
	if (result)
	{
		return result;
	}
	struct rawdata *data=NULL;
	struct rawdata *curdataPtr=NULL;
	if (filePtr)
	{
		struct files *curfilePtr=filePtr;
		int files_count=0; //переменая для подсчёта количества открытых временных файлов
		while (curfilePtr)
		{
			files_count++;
			curfilePtr=curfilePtr->next;
		}
		if (!(data=calloc(1, sizeof(struct rawdata)+files_count*sizeof( int ))))
		{
			perror("Проблема с памятью - остановка обработки");
			return 2;
		}
		char *temp_string=calloc(2000, 1);
		curfilePtr=filePtr;
		curdataPtr=data;
		while (!(feof(curfilePtr->temp_file)))
		{
			curfilePtr=filePtr;
			for (int i=0; i < files_count; i++)
			{
				int before_separator=0;
				int offset=0;
				//Обработка осуществлятся только в случае успешности операции считывания строки из файла
				if (fgets(temp_string, 2000, curfilePtr->temp_file))
				{
					if (temp_string[0] == '\"')
					{
						offset=1;
						int quote_count=1;
						do
						{
							before_separator++;
							if (temp_string[before_separator] == '\"')
								quote_count++;
						}
						while (strncmp(&temp_string[before_separator], TMP_SEP, 1) || quote_count%2);
						//~ strncpy(data->name, &temp_string[1], before_separator-2);
					}
					else
					{
						before_separator=strcspn(temp_string, TMP_SEP);
						//~ strncpy(data->name, temp_string, before_separator-1);
					}
					//Первый проход (строка в структуре пуста)
					if (curdataPtr->name[0] == '\000')
					{
						strncpy(curdataPtr->name, &temp_string[offset], before_separator-2*offset);
						curdataPtr->values[i]=atoi(&temp_string[before_separator+1]);
					}
					else	
					{
						//Если считанная строка не отличается от уже записанной в структуру - к массиву данных добавляется новое значение
						int difference=strncmp(&temp_string[offset], curdataPtr->name, before_separator-2*offset);
						if (!difference)
						{
							curdataPtr->values[i]=atoi(&temp_string[before_separator+1]);
						}
						//Если считанная строка больше записанной в структуру - создаётся новая структура,
						//следующая в списке, содержащая данную строку и её значение
						else if (difference > 0)
						{
							struct rawdata *new=NULL;
							if (!(new=calloc(1, sizeof(struct rawdata)+files_count*sizeof( int ))))
							{
								perror("Проблема с памятью - остановка обработки");
								return 2;
							}
							strncpy(new->name, &temp_string[offset], before_separator-2*offset);
							new->values[i]=atoi(&temp_string[before_separator+1]);
							//Если текущая структура - последняя в списке,
							//то новая структура просто добавляется в конец списка
							if (!curdataPtr->next)
								curdataPtr->next=new;
							else
							{
								struct rawdata *tempcurPtr=curdataPtr->next;
								struct rawdata *tempprevPtr=curdataPtr;
								int diff2=strncmp(&temp_string[offset], tempcurPtr->name, before_separator-2*offset);
								while (diff2 > 0 && tempcurPtr->next)
								{
									tempprevPtr=tempcurPtr;
									tempcurPtr=tempcurPtr->next;
									diff2=strncmp(&temp_string[offset], tempcurPtr->name, before_separator-2*offset);
								}
								//Если новая строка больше, чем во всех последующих структурах,
								//то новая структура добавляется в самый конец списка
								if (diff2 > 0)
									tempcurPtr->next=new;
								//Если новая строка совпадает со строкой в одной из последующих структур -
								//её значение добавляется к массиву такой структуры, сама новая структура удаляется
								if (!diff2)
								{
									tempcurPtr->values[i]=new->values[i];
									free(new);
								}
								//Если новая строка меньше, чем в одной из последующих структур -
								//новая структура вставляется перед ней
								if (diff2 < 0)
								{
									tempprevPtr->next=new;
									new->next=tempcurPtr;
								}
							}
						}
						//Если считанная строка меньше записанной в структуру - создаётся новая структура,
						//содержащая данную строку и её значение, идущая в списке перед текущей структурой
						else if (difference < 0)
						{
							struct rawdata *new=NULL;
							if (!(new=calloc(1, sizeof(struct rawdata)+files_count*sizeof( int ))))
							{
								perror("Проблема с памятью - остановка обработки");
								return 2;
							}
							strncpy(new->name, &temp_string[offset], before_separator-2*offset);
							new->values[i]=atoi(&temp_string[before_separator+1]);
							//Если текущая структура - первая в списке,
							//то новая структура просто добавляется в начало списка
							if (curdataPtr == data)
							{
								new->next=curdataPtr;
								data=new;
								curdataPtr=new;
							}
							else
							{
								struct rawdata *tempcurPtr=data;
								struct rawdata *tempprevPtr=data;
								int diff2=strncmp(&temp_string[offset], tempcurPtr->name, before_separator-2*offset);
								while (diff2 > 0 && tempcurPtr->next)
								{
									tempprevPtr=tempcurPtr;
									tempcurPtr=tempcurPtr->next;
									diff2=strncmp(&temp_string[offset], tempcurPtr->name, before_separator-2*offset);
								}
								//Если новая строка меньше, чем в одной из предыдущих структур -
								//она вставляется перед ней
								if (diff2 < 0)
								{
									new->next=tempcurPtr;
									//Если новая строка меньше строки в самой первой структуре -
									//новая структура становится первой в списке
									if (tempcurPtr == data)
										data=new;
									else
										tempprevPtr->next=new;
								}
								//Если новая строка совпадает со строкой в одной из последующих структур -
								//её значение добавляется к массиву такой структуры, сама новая структура удаляется
								if (!diff2)
								{
									tempcurPtr->values[i]=new->values[i];
									free(new);
								}
							}
						}
					}
					//~ curdataPtr->values[i]=atoi(&temp_string[before_separator+1]);
					
					//DEBUG
					//~ puts("");
					//~ printf("(%d) %s%s\n%d\n%s%d\n", i, temp_string, curdataPtr->name, before_separator, &temp_string[before_separator+1], curdataPtr->values[i]);
					
					memset(temp_string, 0, 2000);
					//Если достигнут конец файла - осуществлятся поиск другого файла в списке,
					//конец которого ещё не достигнут
					//Если такого файла нет (все файлы прочитаны до конца) -
					//переменной присваивается адрес указателя на последний доступный в списке файл
					if (feof(curfilePtr->temp_file))
					{
						for (int j=0; j< files_count; j++)
						{
							if (!curfilePtr->next)
								curfilePtr=filePtr;
							else
								curfilePtr=curfilePtr->next;
							if (!(feof(curfilePtr->temp_file)))
							{
								i=j;
								break;
							}
						}
					}
					//Если конец файла не достигнут - осуществлятся переход к следующему файлу
					else
						curfilePtr=curfilePtr->next;
					if (!curfilePtr)
						curfilePtr=filePtr;
				}
				//Если достигнут конец файла - осуществлятся поиск другого файла в списке,
				//конец которого ещё не достигнут
				//Если такого файла нет (все файлы прочитаны до конца) -
				//переменной присваивается адрес указателя на последний доступный в списке файл
				if (feof(curfilePtr->temp_file))
				{
					for (int j=0; j< files_count; j++)
					{
						if (!curfilePtr->next)
							curfilePtr=filePtr;
						else
							curfilePtr=curfilePtr->next;
						if (!(feof(curfilePtr->temp_file)))
						{
							i=j;
							break;
						}
					}
				}
			}
			if (curdataPtr->next)
			{
				curdataPtr=curdataPtr->next;
			}
		}
		free(temp_string);
		temp_string=NULL;
		//DEBUG
		//~ struct rawdata *testdata=data;
		//~ puts("");
		//~ while (testdata)
		//~ {
			//~ printf("%s\n", testdata->name);
			//~ for (int i=0; i < files_count; i++)
			//~ {
				//~ printf("%d ", testdata->values[i]);
			//~ }
			//~ puts("");
			//~ testdata=testdata->next;
		//~ }
		
		//Открытие служебного файла
		//и считывание из него номера (имени) следующего файла для записи обработанных данных
		//(если служебный файл не существует - он будет создан заново)
		FILE *dat_file=NULL;
		result=open_file(&dat_file, DATA_FILE, NULL, "r");
		if (result)
		{
			int string_lenght_1=strlen("Возникла проблема при открытии служебного файла \n ");
			int string_lenght_2=strlen(DATA_FILE);
			char *error_message=calloc(string_lenght_1+string_lenght_2*2+1, 1);
			snprintf(error_message, string_lenght_1+string_lenght_2+1, "Возникла проблема при открытии файла \"%s\"\n %s", DATA_FILE, DATA_FILE);
			if (!(dat_file = fopen(DATA_FILE, "w")))
			{
				perror(error_message);
				free(error_message);
				close_files(&filePtr);
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
				close_files(&filePtr);
				return 3;
			}
			free(error_message);
			error_message=NULL;
		}
		char *tmp_filename=calloc(100, 1);
		char temp_string_2[18]={""};
		int lenght_string_2=0;
		if (!strcmp(extension, ".adat"))
		{
			strncpy(temp_string_2, "last_average_file", 17);
			lenght_string_2=17;
		}
		if (!strcmp(extension, ".csv"))
		{
			strncpy(temp_string_2, "last_result_file", 16);
			lenght_string_2=16;
		}
		do
		{
			fgets(tmp_filename, 100, dat_file);
			if (!strncmp(tmp_filename, temp_string_2, lenght_string_2))
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
					if (!strcmp(extension, ".adat"))
						snprintf(tmp_filename, 100, "%d%s", tmp_name, extension);
					if (!strcmp(extension, ".csv"))
						snprintf(tmp_filename, 100, "result-%d%s", tmp_name, extension);
					free(temp_string);
					temp_string=NULL;
					break;
				}
			}
		} while (!feof(dat_file));
		if (fclose(dat_file))
		{
			perror("Ошибка обработки системного файла");
			return 3;
		}
		//Открытие временного файла, в который будут записаны результаты предыдущих вычислений
		FILE *result_file=NULL;
		open_file(&result_file, tmp_filename, extension, "w");
		//Запись средних значений
		if (!strcmp(extension, ".adat"))
		{
			struct rawdata *curdataPtr=data;
			while (curdataPtr)
			{
				int length=strlen(curdataPtr->name);
				if (strcspn(curdataPtr->name, TMP_SEP) < length || strcspn(curdataPtr->name, "\"") < length)
				{
					fprintf(result_file, "\"");
					fprintf(result_file, "%s\"%s", curdataPtr->name, TMP_SEP);
				}
				else
					fprintf(result_file, "%s%s", curdataPtr->name, TMP_SEP);
				int count=0;
				int res_value=0;
				for (int i=0; i< files_count; i++)
				{
					if (curdataPtr->values[i])
						count++;
					res_value=res_value+curdataPtr->values[i];
				}
				fprintf(result_file, "%d\n", count ? res_value/count : 0);
				curdataPtr=curdataPtr->next;
			}
		}
		//Запись скомбинированных средних значений
		if (!strcmp(extension, ".csv"))
		{
			struct rawdata *curdataPtr=data;
			while (curdataPtr)
			{
				int length=strlen(curdataPtr->name);
				if (strcspn(curdataPtr->name, TMP_SEP) < length || strcspn(curdataPtr->name, "\"") < length)
				{
					fprintf(result_file, "\"");
					fprintf(result_file, "%s\"", curdataPtr->name);
				}
				else
					fprintf(result_file, "%s", curdataPtr->name);
				for (int i=0; i<files_count; i++)
				{
					fprintf(result_file, "%s%d", TMP_SEP, curdataPtr->values[i]);
				}
				fputs("\n", result_file);
				curdataPtr=curdataPtr->next;
			}
		}
		//Закрытие вновь созданного файла результата и обновление системного файла
		if (fclose(result_file))
		{
			perror("Ошибка записи временного файла");
			puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
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
			return 3;
		}
		FILE *tmp;
		tmp=NULL;
		if (!(tmp = fopen(backup_filename, "r")))
		{
			perror("Ошибка обработки системного файла");
			puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
			free(backup_filename);
			return 3;
		}
		if (!(dat_file = fopen(DATA_FILE, "w")))
		{
			perror("Ошибка обработки системного файла");
			puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
			free(backup_filename);
			fclose(tmp);
			return 3;
		}
		temp_string=calloc(100, 1);
		while (!feof(tmp))
		{
			fgets(temp_string, 100, tmp);
			if ((!strncmp(temp_string, temp_string_2, lenght_string_2)) && !feof(tmp))
			{
				fprintf(dat_file, "%s %d\n", temp_string_2, atoi(&temp_string[lenght_string_2])+1);
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
			perror("Ошибка удаления временного системного файла");
		}
		free(backup_filename);
		backup_filename=NULL;
		if (fclose(dat_file))
		{
			perror("Ошибка обновления системного файла");
			puts("Проверьте права доступа к каталогу с программой и свободное место на диске с разделом, содержащим этот каталог");
			return 3;
		}
		printf("Обработка завершена. Результат записан в файл: %s\n", tmp_filename);
		free(tmp_filename);
		tmp_filename=NULL;
	}
	//Закрытие временных файлов
	if (filePtr)
	{
		close_files(&filePtr);
	}
	if (data)
	{
		struct rawdata *lastPtr=data;
		while (lastPtr)
		{
			data=data->next;
			free(lastPtr);
			lastPtr=data;
		}
	}
	return 0;
}
