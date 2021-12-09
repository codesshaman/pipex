/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex-bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jleslee <jleslee@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/12/07 22:03:56 by jleslee           #+#    #+#             */
/*   Updated: 2021/12/09 18:46:41 by jleslee          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "pipex.h"

int	openfile(char *filename, int mode)
{
	if (mode == 0)
	{
		if (access(filename, F_OK))
		{
			write(2, "File not found\n", 15);
			return (0);
		}
		return (open(filename, O_RDONLY));
	}
	else
		return (open(filename, O_CREAT | O_WRONLY | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
}

char	*full_command_path(char *cmd, char **env)
{
	char	*path;
	char	*dir;
	char	*bin;
	int		i;

	i = 0;
	while (env[i] && ncompare(env[i], "PATH=", 5))
		i++;
	if (!env[i])
		return (cmd);
	path = env[i] + 5;
	while (path && len_ch(path, ':') > -1)
	{
		dir = str_ndup(path, len_ch(path, ':'));
		bin = make_command(dir, cmd);
		free(dir);
		if (access(bin, F_OK) == 0)
			return (bin);
		free(bin);
		path += len_ch(path, ':') + 1;
	}
	return (cmd);
}

void	second_command_processing(char *cmd, char **env)
{
	char	**args;
	char	*path;

	args = str_split(cmd, ' ');
	if (len_ch(args[0], '/') > -1)
		path = args[0];
	else
		path = full_command_path(args[0], env);
	execve(path, args, env);
	write(2, "Сommand not found\n", 19);
	exit(127);
}

void	first_command_processing(char *cmd, char **env, int fdin)
{
	pid_t	pid;
	int		pipefd[2];

	pipe(pipefd);
	pid = fork();
	if (pid)
	{
		close(pipefd[1]);
		dup2(pipefd[0], 0);
		waitpid(pid, NULL, 0);
	}
	else
	{
		close(pipefd[0]);
		dup2(pipefd[1], 1);
		if (fdin == 0)
			exit(1);
		else
			second_command_processing(cmd, env);
	}
}

int	main(int ac, char **av, char **env)
{
	int	fdin;
	int	fdout;
	int	i;

	i = 3;
	if (ac >= 5)
	{
		fdin = openfile(av[1], 0);
		fdout = openfile(av[ac - 1], 1);
		dup2(fdin, 0);
		dup2(fdout, 1);
		first_command_processing(av[2], env, fdin);
		while (i < ac - 2)
			first_command_processing(av[i++], env, 1);
		second_command_processing(av[i], env);
	}
	else
		write(2, "Invalid number of arguments.\n", 29);
	return (1);
}
