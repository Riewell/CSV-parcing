/*  files_functions.c

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

#include "csv_parcing.h"

int next_file(struct files ***filePtr, FILE *temp_filePtr);
int open_file(FILE **opened_file, char *filename, const char *extension, const char *mode);
void close_files(struct files **filePtr);

int open_files(char **raw_files, const char *extension, int quantity, struct files **filePtr)
{
	//~ struct files *filePtr=NULL;
	for (int i = 2; i < quantity+2; i++)
	{
		FILE *temp_file=NULL;
		int substring_end=strcspn(raw_files[i], "-,;:&*/\\");
		if (substring_end < strlen(raw_files[i]))
		{
			int substring_start=0;
			while (substring_start <= strlen(raw_files[i]))
			{
				if (raw_files[i][substring_end] == '-')
				{
					char *temp_string=calloc(substring_end-substring_start+1, 1);
					strncpy(temp_string, &raw_files[i][substring_start], substring_end-substring_start);
					temp_string[substring_end-substring_start+1]='\000';
					substring_start=substring_end+1;
					substring_end=substring_start+(strcspn(&raw_files[i][substring_start], "-,;:&*/\\"));
					if (raw_files[i][substring_end] == '-')
					{
						printf("Неверно задан диапазон в параметре \"%s\" - прекращение работы\n", raw_files[i]);
						free(temp_string);
						close_files(filePtr);
						return 1;
					}
					char *temp_string_2=calloc(substring_end-substring_start+1, 1);
					strncpy(temp_string_2, &raw_files[i][substring_start], substring_end-substring_start);
					temp_string_2[substring_end-substring_start+1]='\000';
					int digit_name_1=0;
					int digit_name_2=0;
					digit_name_1=atoi(temp_string);
					digit_name_2=atoi(temp_string_2);
					if ((digit_name_1 > 0) && (digit_name_2 > 0))
					{
						if (digit_name_1 > digit_name_2)
						{
							int temp=digit_name_1;
							digit_name_1=digit_name_2;
							digit_name_2=temp;
						}
						int digit_str_length=0;
						if (strlen(temp_string) >= strlen(temp_string_2))
							digit_str_length=strlen(temp_string);
						else
							digit_str_length=strlen(temp_string_2);
						digit_str_length=digit_str_length+1;
						for (int i=digit_name_1; i<=digit_name_2; i++)
						{
							char *digit_str=calloc(digit_str_length, 1);
							snprintf(digit_str, digit_str_length, "%d", i);
							if (open_file(&temp_file, digit_str, extension, "r"))
							{
								close_files(filePtr);
								free(digit_str);
								free(temp_string);
								free(temp_string_2);
								return 1;
							}
							if (next_file(&filePtr, temp_file))
							{
								close_files(filePtr);
								free(digit_str);
								free(temp_string);
								free(temp_string_2);
								return 2;
							}
							free(digit_str);
							digit_str=NULL;
							temp_file=NULL;
						}
					}
					else
					{
						if (open_file(&temp_file, temp_string, extension, "r"))
						{
							close_files(filePtr);
							free(temp_string);
							free(temp_string_2);
							return 1;
						}
						if (next_file(&filePtr, temp_file))
						{
							close_files(filePtr);
							free(temp_string);
							free(temp_string_2);
							return 2;
						}
						if (open_file(&temp_file, temp_string_2, extension, "r"))
						{
							close_files(filePtr);
							free(temp_string);
							free(temp_string_2);
							return 1;
						}
						if (next_file(&filePtr, temp_file))
						{
							close_files(filePtr);
							free(temp_string);
							free(temp_string_2);
							return 2;
						}
					}
					substring_start=substring_end+1;
					substring_end=substring_start+(strcspn(&raw_files[i][substring_start], "-,;:&*/\\"));
					free(temp_string);
					free(temp_string_2);
					temp_string=NULL;
					temp_string_2=NULL;
				}
				else
				{
					char *temp_string=calloc(substring_end-substring_start+1, 1);
					strncpy(temp_string, &raw_files[i][substring_start], substring_end-substring_start);
					temp_string[substring_end-substring_start+1]='\000';
					if (open_file(&temp_file, temp_string, extension, "r"))
					{
						close_files(filePtr);
						free(temp_string);
						return 1;
					}
					if (next_file(&filePtr, temp_file))
					{
						close_files(filePtr);
						free(temp_string);
						return 2;
					}
					substring_start=substring_end+1;
					substring_end=substring_start+(strcspn(&raw_files[i][substring_start], "-,;:&*/\\"));
					free(temp_string);
					temp_string=NULL;
				}
			}
		}
		else
		{
			if (open_file(&temp_file, raw_files[i], extension, "r"))
			{
				close_files(filePtr);
				return 1;
			}
			if (next_file(&filePtr, temp_file))
			{
				close_files(filePtr);
				return 2;
			}
		}
	}
	//~ if (filePtr)
	//~ {
		//~ close_files(filePtr);
	//~ }
	return 0;
}

//~ //Функция для добавления названий и значений в связанный список (массив структур) разделов/подразделов
int next_file(struct files ***filePtr, FILE *temp_filePtr)
{
	struct files *new_filePtr=NULL;
	if (!(new_filePtr=calloc(1, sizeof(struct files))))
	{
		perror("Проблема с памятью - остановка обработки");
		return 2;
	}
	//~ strcpy(new_filePtr->sec_name, sec_name);
	new_filePtr->temp_file=temp_filePtr;
	new_filePtr->next=NULL;
	if (!(**filePtr))
		**filePtr=new_filePtr;
	else
	{
		struct files *last_filePtr=NULL;
		last_filePtr=**filePtr;
		while (last_filePtr->next)
		{
			last_filePtr=last_filePtr->next;
		}
		last_filePtr->next=new_filePtr;
	}
	return 0;
}

//~ int open_file(FILE *opened_file, char *filename, const char *extension, const char *mode, const char *task)
int open_file(FILE **opened_file, char *filename, const char *extension, const char *mode)
{
	//~ FILE *temp_file=NULL;
	int string_lenght_1=strlen("Возникла проблема при открытии файла \n ");
	int string_lenght_2=strlen(filename);
	char *error_message=calloc(string_lenght_1+string_lenght_2*2+1, 1);
	snprintf(error_message, string_lenght_1+string_lenght_2+1, "Возникла проблема при открытии файла \"%s\"\n %s", filename, filename);
	if (!(*opened_file = fopen(filename, mode)))
	{
		if (extension)
		{
			char *full_filename=calloc(strlen(filename)+strlen(extension)+1, 1);
			strcpy(full_filename, filename);
			strcat(full_filename, extension);
			//~ strcat(filename, extension);
			//~ if (!(opened_file = fopen(filename, mode)))
			*opened_file = fopen(full_filename, mode);
			free(full_filename);
		}
		if (!(*opened_file))
		{					
			perror(error_message);
			//~ puts("Переход к следующему файлу...");
			free(error_message);
			return 1;
		}
	}
	free(error_message);
	error_message=NULL;
	return 0;
}

void close_files(struct files **filePtr)
{
	if (*filePtr)
	{
		struct files *tempPtr=NULL;
		tempPtr=*filePtr;
		while (tempPtr)
		{
			if (tempPtr->temp_file)
				fclose(tempPtr->temp_file);
			tempPtr=tempPtr->next;
		}
	}
	return;
}
