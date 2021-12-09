/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jleslee <jleslee@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/05 19:14:30 by jleslee           #+#    #+#             */
/*   Updated: 2021/12/09 16:48:03 by jleslee          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

// Этап 1. Читаем входной и выходной файлы

int	openfile (char *file, int mode)
{
	if (mode == INFILE)
	{
		if (access(file, F_OK))
		{
			write(STDERR, "File not found\n", 15);
			return (STDIN);
		}
		return (open(file, O_RDONLY));
	}
	else
		return (open(file, O_CREAT | O_WRONLY | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
}

// Находим полный путь к команде в переменных окружения
// Пока мы не нашли нужный путь в env, итерируемся
// Если пути нет, возвращаем команду
// Если путь найден, 

char	*full_command_path(char *cmd, char **env)
{
	int		i;
	char	*path;
	char	*dir;
	char	*full;

	i = 0;
	while (env[i] && ncompare(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		return (cmd);
	path = env[i] + 5;
	while (path && find_ch(path, ':') > -1)
	{
		dir = str_ndup(path, find_ch(path, ':'));
		full = path_join(dir, cmd);
		free(dir);
		if (access(full, F_OK) == 0)
			return (full);
		free(full);
		path += find_ch(path, ':') + 1;
	}
	return (cmd);
}

// Обрабатываем вторую команду,
// Отделяя флаги и выполняя поиск

void	second_command_processing(char *command, char **env)
{
	char	**args;
	char	*path;

	args = str_split(command, ' ');
	if (find_ch(args[0], '/') > -1)
		path = args[0];
	else
		path = full_command_path(args[0], env);
	execve(path, args, env);
	write(STDERR, "Сommand not found\n", 19);
	exit(127);
}

// Клонируем процесс форком и обрабатываем первую команду:

void	first_command_processing(char *command, char **env, int infile_fd)
{
	pid_t	pid;
	int		pipefd[2];

	pipe(pipefd);
	pid = fork();
	if (pid)
	{
		close(pipefd[1]);
		dup2(pipefd[0], STDIN);
		waitpid(pid, NULL, 0);
	}
	else
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT);
		if (infile_fd == STDIN)
			exit(1);
		else
			second_command_processing(command, env);
	}
}

// 1. Получаем дескрипторы входного и выходного файлов
// 2. дублируем их, закрывая на вход и на выход
// 3. клонируем процесс и выполняем 1 команду
// 4. выполняем вторую команду и схлопываем процесс

int	main (int ac, char **av, char **env)
{
	int	infile_fd;
	int	outfile_fd;

	if (ac == 5)
	{
		infile_fd = openfile(av[1], INFILE);
		outfile_fd = openfile(av[4], OUTFILE);
		dup2(infile_fd, STDIN);
		dup2(outfile_fd, STDOUT);
		first_command_processing(av[2], env, infile_fd);
		second_command_processing(av[3], env);
	}
	else
		write(STDERR, "Нужно 4 аргумента\n", 29);
	return (1);
}