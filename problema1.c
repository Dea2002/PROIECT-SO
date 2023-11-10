#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
  // Verifică numărul corect de argumente în linia de comandă
  if (argc != 2)
    {
      printf("Usage: %s <fisier_intrare>\n", argv[0]);
      exit -1;
    }

  // Deschidem fișierul BMP pentru citire
  int fd_input = open(argv[1], O_RDONLY);   // iar daca nu reuseste deschiderea afi  
  if (fd_input == -1)                      // un mesaj de eroare si incheie programul
    {
      perror("Eroare la deschiderea fisierului de intrare");
      exit(-1);
    }

  struct stat file_stat;   // fol fct stat pt a obtine informatii despre fisierul
  if (stat(argv[1], &file_stat) == -1)  // specificat in linie de comanda si le stocheaza in structura file_stat
    {
      perror("Error getting file status");
      exit -1;
    }

  // obtinem drepturile de acces ale fisierului si se transforma intr un sir de caractere formatat
  // si sunt determinate folosind operatori pe biti si vedem daca fiecare permisiune este activata sau nu in campul st_mode
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

  
  // Deschide fișierul de statistici pentru scriere
  int fd_output = open("statistica.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd_output == -1)
    {
      perror("Eroare la deschiderea fisierului de statistici");
      return 1;
  }
  
  // Obține informații despre timpul ultimei modificări sub forma unui șir de caractere
  // sub forma dd.mm.yyyy, folosind fct strftime
  char modificare[20];
  time_t modification_time = time(NULL);
  strftime(modificare, sizeof(modificare), "%d.%m.%Y", localtime(&modification_time));

  
  char buffer[1024];
  int n=sprintf(buffer,
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
		argv[1], height, width, file_stat.st_size, file_stat.st_uid,
		modificare, file_stat.st_nlink,
		permissions + 1 , permissions + 4, permissions + 7);


// Verifică dacă sprintf a reușit să formateze corect șirul și să atribuie numărul de caractere scrise variabilei n
 if (n >= 0 && n < sizeof(buffer))
   {
     // Scrie șirul 
     if (write(fd_output, buffer, n) == -1)
       {
	 perror("Eroare la scrierea in fisierul de statistici");
       }
     else
       {
	 printf("Scriere cu succes in fisierul de statistici.\n");
       }
   }
 else
   {
     // A apărut o eroare la formatarea șirului
     fprintf(stderr, "Eroare la formatarea șirului.\n");
   }
 

 close(fd_output);
 return 0;
}
