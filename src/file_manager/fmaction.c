#include "../../header/file_manager/shared.h"

//Делает активным правое окно
void action_winactive(unsigned index)
{
	//Если окно итак активно или не удалось перейти в директорию
	if(index == win_active || chdir(dir_patch[index]) == -1)
		return;

	//Смеа директории и сброс значений
	dir_print = 0;
	dir_inwin = 0;
	win_active = index;

	winref_all();
}

//Перемещает курсор активной папки вверх
void action_mvup()
{
	//Если дошли до начала директории
	if(dir_print + dir_inwin <= 0)
		return;
	//Если дошли до начала окна
	if(dir_inwin <= 0)
		dir_print--;
	else
		dir_inwin--;
	winref_content(win_active);
}

//Перемещает курсор активной папки вниз
void action_mvdown()
{
	int width;						//Ширина окна
	int height; 					//Высота окна

	getmaxyx(win[win_active], height, width);
	//Если пролистывать нечего
	if(dir_print + dir_inwin >= dir_count[win_active] - 1)
		return;
	//Если дошли до конца окна
	if(dir_inwin >= height - 1)
		dir_print++;
	else
		dir_inwin++;
	winref_content(win_active);
}

//Обработкик нажатия клавиш в окне ввода. Получает и разделяет параметры
int action_runinput(char *params[])
{
	int key = 0;				//Код нажатой клавиши
	char *ptr;					//Указатель используемый для strtok
	char input[255];			//Храних введенные/отображаемые данные
	unsigned cpindex = 0;		//Индекс символа в input с которого начинается вывод 
	unsigned data_len = 0;		//Длина массива input
	unsigned count_param = 0;	//Количество введенных параметров

	//Инициализируем данные и окно
	input[0] = '\0';
	win_createinput();
	winref_input(input, cpindex);

	while((key = getch()) != '\n')
	{
		switch(key)
		{
			break;
			case 27:
				win_destroyinput();
				return -1;
			break;
			case KEY_LEFT:
				if(cpindex > 0)
					cpindex--;
			break; 
			case KEY_RIGHT:
				if(cpindex < data_len)
					cpindex++;
			break;
			case KEY_BACKSPACE:
				if(data_len != 0)
				{
					data_len--;
					input[data_len] = '\0';
					if(cpindex > 0)
						cpindex--;	
				}
			break;
			default:
				if(data_len > 250)
					continue;
				input[data_len] = (char)key;
				input[data_len + 1] = '\0';
				data_len++;
				if(data_len > 26)
					cpindex++;
			break;
		}
		winref_input(input, cpindex);
	}

	//Разбиваем ввежденную строку
	ptr = strtok(input, " ");
	for(count_param = 1; ptr != NULL; count_param++)
	{
		strcpy(params[count_param], ptr);
		ptr = strtok (NULL, " ");
	}
	win_destroyinput();
	return count_param;
}

//Открывает директорию/файл
void action_open()
{
	struct dline newdir;	//Структура с инфорацией об открываемом обьекте
	char path[1024];		//Полный петь к новому обьекту

	//К текущей директории прибавляем имя директории в которую переходим
	newdir = dir_content[win_active][dir_print + dir_inwin];
	sprintf(path, "%s/%s", dir_patch[win_active], newdir.name);

	//Если это папка, то переходим в нее
	if(newdir.type == DT_DIR)
	{
		//Изменение рабочей директории
		if(chdir(path) == -1)
			return;
		getcwd(dir_patch[win_active], 1024);
		
		//Получение ее содержимого и вывод
		dir_count[win_active] = dir_get(path, &(dir_content[win_active]));
		dir_print = dir_inwin = 0;
		winref_all();
	}
	else	//Если это файл
	{
		struct stat st;		//Структура с информацией о файле
		pid_t pid;			//id нового процесса
		char *params[16];	//Массив парамеров для exec
		int status = 0;		//Для wait
		int count_param;	//Количество параметров
		char ch;

		//Выделяем память под параметры
		for(int i = 0; i < 16; i++)
			params[i] = malloc(255);
		//Делаем первым параметром имя файла, а последним NULL
		strcpy(params[0], newdir.name);
		count_param = action_runinput(params);
		params[count_param] = NULL;

		stat(newdir.name, &st);
		win_destroy();
		pid = fork();

		if(pid == 0)
		{
			//Если файл может быть запущен, пытаемся это сделать, иначе считаем его текстовым
			if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)
				execv(path, params);
			else
				execl( te_path, "text_editor", path, (char*)0 );
			_exit(EXIT_FAILURE);
		}
		else
		{
			waitpid(pid, &status, 0);
			printf("\nThe program is completed. Type any character to continue:");
			scanf("%c%*[^\n]", &ch);
			win_rerun();
		}
	}
}
