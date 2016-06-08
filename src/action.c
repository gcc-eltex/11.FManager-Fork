#include "../header/shared.h"

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

//Открывает директорию/файл
void action_open()
{
	struct dline newdir;
	char path[1024];
	int status = 0;
	pid_t pid;

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
		dir_print = 0;
		dir_inwin = 0;
		winref_border(win_active + 2);
		winref_content(win_active);
	}
	//Если это файл
	else
	{
		struct stat inffile;
		char ch;

		win_destroy();
		stat(newdir.name, &inffile);
		pid = fork();

		if(pid == 0)
		{
			//Если файл может быть запущен, пытаемся это сделать
			if(inffile.st_mode & S_IXUSR || inffile.st_mode & S_IXGRP || inffile.st_mode & S_IXOTH)
				execl( path, newdir.name, (char*)0 );
			//Иначе открываем его как текстовый
			else
				execl( te_path, "text_editor", path, (char*)0 );
			_exit(EXIT_FAILURE);
		}
		else
		{
			waitpid(pid, &status, 0);
			printf("\nThe program is completed. Type any character to continue:");
			scanf("%c", &ch);
			scanf("%*[^\n]");
			win_rerun();
		}
	}
}