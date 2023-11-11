#include <dirent.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>
#include<limits.h>

void generare(const char *nume, const char *director, int fd_output)
{
  char cale[PATH_MAX];
  sprintf(cale, "%s/%s", director,nume);
  
  int fd_input = open(cale, O_RDONLY);
  if (fd_input == -1)
    {
      perror("Eroare la deschiderea fisierului");
      exit -1;
    }

  struct stat file_stat;
  if (stat(cale, &file_stat) == -1)
    {
      perror("Eroare la fisier");
      close(fd_input);
      return;
    }

  char permissions[11]; // 10 caractere pentru permisiuni + terminatorul de șir '\0' 
  sprintf(permissions, "%c%c%c%c%c%c%c%c%c%c",
	  (file_stat.st_mode & S_IRUSR) ? 'R' : '-',
	  (file_stat.st_mode & S_IWUSR) ? 'W' : '-',
	  (file_stat.st_mode & S_IXUSR) ? 'X' : '-',
	  (file_stat.st_mode & S_IRGRP) ? 'R' : '-',
	  (file_stat.st_mode & S_IWGRP) ? 'W' : '-',
	  (file_stat.st_mode & S_IXGRP) ? 'X' : '-',
	  (file_stat.st_mode & S_IROTH) ? 'R' : '-',
	  (file_stat.st_mode & S_IWOTH) ? 'W' : '-',
	  (file_stat.st_mode & S_IXOTH) ? 'X' : '-',
	  '\0');

  
  // Sarim peste primii 18 octeți, mutam cursorul cu 18 octeti
  lseek(fd_input, 18, SEEK_SET);
  
  // Citește lățimea și înălțimea imaginii din header
  int width, height;
  read(fd_input, &width, sizeof(int));
  read(fd_input, &height, sizeof(int));
  close(fd_input);

  char modificare[20];
  time_t modification_time = time(NULL);
  strftime(modificare, sizeof(modificare), "%d.%m.%Y", localtime(&modification_time));

  char buffer[1024];
  int n;

  if (S_ISREG(file_stat.st_mode))
    {
    if (strstr(nume, ".bmp"))
      {
	n = sprintf(buffer,
		    "Este un fisier cu extensia bmp\n"
		    "nume fisier: %s\n"
		    "inaltime: %d\n"
		    "lungime: %d\n"
		    "dimensiune: %ld\n"
		    "identificatorul utilizatorului: %d\n"
		    "timpul ultimei modificari: %s\n"
		    "contorul de legaturi: %ld\n"
		    "drepturi de acces user: %s\n"
		    "drepturi de acces grup: %s\n"
		    "drepturi de acces altii: %s\n",
		    nume, height, width, file_stat.st_size, file_stat.st_uid,
		    modificare, file_stat.st_nlink,
		    permissions + 1 , permissions + 4, permissions + 7);
      }
    else
      {
	n = sprintf(buffer,
		    "\nEste un fisier obisnuit\n"
		    "nume fisier: %s\n"
		    "dimensiune: %ld\n"
		    "identificatorul utilizatorului: %d\n"
		    "timpul ultimei modificari: %s\n"
		    "contorul de legaturi: %ld\n"
		    "drepturi de acces user: %s\n"
		    "drepturi de acces grup: %s\n"
		    "drepturi de acces altii: %s\n",
		    nume, file_stat.st_size, file_stat.st_uid,
		    modificare, file_stat.st_nlink,
		    permissions + 1, permissions + 4, permissions + 7);
      }
    }
  else if (S_ISLNK(file_stat.st_mode))
    {
      n = sprintf(buffer,
		  "\nEste o legatura simbolica\n"
		  "nume legatura: %s\n"
		  "dimensiune legatura: %ld\n"
		  "dimensiune fisier target: %ld\n"
		  "drepturi de acces user legatura: %s\n"
		  "drepturi de acces grup legatura: %s\n"
		  "drepturi de acces altii legatura: %s\n",
		  nume, file_stat.st_size,
		  (stat(cale, &file_stat) == -1) ? -1 : file_stat.st_size,
		  permissions + 1, permissions + 4, permissions + 7);
    }
  else if (S_ISDIR(file_stat.st_mode))
    {
      n = sprintf(buffer,
		  "\nEste un director\n"
		  "nume director: %s\n"
		  "identificatorul utilizatorului: %d\n"
		  "drepturi de acces user: %s\n"
		  "drepturi de acces grup: %s\n"
		  "drepturi de acces altii: %s\n",
		  nume, file_stat.st_uid,
		  permissions + 1, permissions + 4, permissions + 7);
    }
  
  if (n >= 0 && n < sizeof(buffer))
    {
      if (write(fd_output, buffer, n) == -1)
	{
	  perror("Eroare la scrierea in fisierul de statistici");
	}
      else
	{
	  printf("Scriere cu succes in fisierul de statistici\n");
	}
    }
  else
    {
      printf("Eroare la formatarea șirului\n");
    }
  
  close(fd_input);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    {
      printf("Usage: %s <director_intrare>\n", argv[0]);
      exit -1;
    }

  DIR *dir = opendir(argv[1]);
  if (!dir)
    {
      perror("Eroare la deschiderea directorului");
      exit -1;
    }
  
  int fd_output = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd_output == -1)
    {
      perror("Eroare la deschiderea fisierului de statistici");
      exit -1;
  }

  struct dirent *intrare;
  while ((intrare = readdir(dir)) != NULL)
    {
      if (strcmp(intrare->d_name, ".") != 0 && strcmp(intrare->d_name, "..") != 0)
	{
	  generare(intrare->d_name, argv[1], fd_output);
	}
    }
  
  closedir(dir);
  close(fd_output);
  
  return 0;
}
