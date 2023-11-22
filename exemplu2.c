#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>


void get_permissions(mode_t mode, char *permissions)
{
  sprintf(permissions, "%c%c%c%c%c%c%c%c%c%c",
	  (mode & S_IRUSR) ? 'R' : '-',
	  (mode & S_IWUSR) ? 'W' : '-',
	  (mode & S_IXUSR) ? 'X' : '-',
	  (mode & S_IRGRP) ? 'R' : '-',
	  (mode & S_IWGRP) ? 'W' : '-',
	  (mode & S_IXGRP) ? 'X' : '-',
	  (mode & S_IROTH) ? 'R' : '-',
	  (mode & S_IWOTH) ? 'W' : '-',
	  (mode & S_IXOTH) ? 'X' : '-',
	  '\0');
}

void scriere_in_Statistica(const char *tip, const char *detalii, int fd_output)
{
    char buffer[2050];
    int n=sprintf(buffer, "%s\n%s", tip, detalii);

    if(n>=0 && n<sizeof(buffer))
    {
        if(write(fd_output, buffer, n) == -1)
        {
            perror("Eroare la scrierea în fisierul de statistici");
        }
        else
        {
            printf("Scriere cu succes în fisierul de statistici\n");
        }
    }
    else
    {
        printf("Eroare la formatarea șirului\n");
    }
}


void FisierBMP(const char *nume, const char *director, int fd_output)
{
    char cale[PATH_MAX];
    sprintf(cale, "%s/%s", director, nume);

    int fd_input = open(cale, O_RDONLY);
    if (fd_input == -1)
      {
        perror("Eroare la deschiderea fisierului");
        return;
      }
    
    struct stat file_stat;
    if (stat(cale, &file_stat) == -1)
      {
        perror("Eroare la fisier");
        close(fd_input);
        return;
      }
    
    char permissions[11];
    get_permissions(file_stat.st_mode, permissions);

    int width, height;
    lseek(fd_input, 18, SEEK_SET);
    read(fd_input, &width, sizeof(int));
    read(fd_input, &height, sizeof(int));
    close(fd_input);
    
    char modificare[20];
    time_t modification_time = time(NULL);
    strftime(modificare, sizeof(modificare), "%d.%m.%Y", localtime(&modification_time));
    
    char buffer[1024];
    int n = sprintf(buffer,
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
                    permissions + 1, permissions + 4, permissions + 7);
   
    scriere_in_Statistica("Este un fisier BMP", buffer, fd_output);
  
}


void Fisier_fara_Extensie(const char *nume, const char *director, int fd_output)
{
  char cale[PATH_MAX];
  sprintf(cale, "%s/%s", director, nume);
  
  struct stat file_stat;
  if (stat(cale, &file_stat) == -1)
    {
      perror("Eroare la fisier");
      return;
    }
  
  char permissions[11];
  get_permissions(file_stat.st_mode, permissions);
 
  char modificare[20];
  time_t modification_time = time(NULL);
  strftime(modificare, sizeof(modificare), "%d.%m.%Y", localtime(&modification_time));
  
  char buffer[1024];
  int n = sprintf(buffer,
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
  
  scriere_in_Statistica("\nEste un fisier fara extensia BMP", buffer, fd_output);
}


void Director(const char *nume, const char *director, int fd_output)
{
  char cale[PATH_MAX];
  sprintf(cale, "%s/%s", director, nume);
  
  struct stat file_stat;
  if (stat(cale, &file_stat) == -1)
    {
      perror("Eroare la director");
      return;
    }
  
  char permissions[11];
  get_permissions(file_stat.st_mode, permissions);
  
  char buffer[1024];
  int n = sprintf(buffer,
		  "nume director: %s\n"
		  "identificatorul utilizatorului: %d\n"
		  "drepturi de acces user: %s\n"
		  "drepturi de acces grup: %s\n"
		  "drepturi de acces altii: %s\n",
		  nume, file_stat.st_uid,
		  permissions + 1, permissions + 4, permissions + 7);
  
  scriere_in_Statistica("\nEste un director", buffer, fd_output);
}


void Legatura_Simbolica(const char *nume, const char *director, int fd_output)
{
    char cale[PATH_MAX];
    sprintf(cale, "%s/%s", director, nume);
    
    struct stat file_stat;
    if (lstat(cale, &file_stat) == -1)
      {
	perror("Eroare la link simbolic");
	return;
      }
    
    char permissions[11];
    get_permissions(file_stat.st_mode, permissions);
    
    char target_path[PATH_MAX];
    ssize_t target_size = readlink(cale, target_path, sizeof(target_path) - 1);
    if (target_size == -1)
      {
        perror("Eroare la citirea link-ului simbolic");
        return;
      }
    target_path[target_size] = '\0';
    
    char buffer[1024];
    int n = sprintf(buffer,
		    "nume legatura: %s\n"
		    "dimensiune legatura: %ld\n"
		    "dimensiune fisier target: %ld\n"
		    "drepturi de acces user legatura: %s\n"
		    "drepturi de acces grup legatura: %s\n"
		    "drepturi de acces altii legatura: %s\n",
		    nume, file_stat.st_size, target_size,
		    permissions + 1, permissions + 4, permissions + 7);
    
    scriere_in_Statistica("\nEste legatura simbolica", buffer, fd_output);

}

void generare(const char *nume, const char *director, int fd_output)
{
  char cale[PATH_MAX];
  sprintf(cale, "%s/%s", director, nume);
  
  struct stat file_stat;
  if (stat(cale, &file_stat) == -1)
    {
      perror("Eroare la fisier");
      return;
    }
  
  if (S_ISREG(file_stat.st_mode))
    {
      if (strstr(nume, ".bmp"))
	{
	  FisierBMP(nume, director, fd_output);
	}
      else
	{
	  Fisier_fara_Extensie(nume, director, fd_output);
        }
    }
  else if (S_ISLNK(file_stat.st_mode))
    {
      Legatura_Simbolica(nume, director, fd_output);
    }
  else if (S_ISDIR(file_stat.st_mode))
    {
      Director(nume, director, fd_output);
    }
}


void convert_imagine(const char *cale_in, const char *cale_output)
{
    int fd_in=open(cale_in, O_RDONLY);
    int fd_out=open(cale_output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd_in==-1 || fd_out==-1)
    {
        perror("Eroare la deschiderea fisierelor de intrare");
        return;
    }

    u_int8_t header[54]; // citim header ul BMP (54 de octeti)
    if (read(fd_in, header, sizeof(u_int8_t)*54) == -1)
    {
        perror("Eroare la citirea header-ului");
        close(fd_in);
        close(fd_out);
        return;
    }

    if (write(fd_out, header, sizeof(u_int8_t)*54) == -1)
    {
        perror("Eroare la scrierea header-ului");
        close(fd_in);
        close(fd_out);
        return;
    }

    int width= *(int*)&header[18]; // citim dimensiunea imaginii
    int height= *(int*)&header[22];
    int row_size=width*3; // calculam dimensiunea imaginii (fără padding)
    int padding=(4-(row_size%4))%4;
    int size=(row_size+padding)*height;

    for(int i=0; i<height; i++) // citim și convertim pixel cu pixel
    {
        for(int j=0; j<width; j++)
        {
            u_int8_t pixel[3];
            if(read(fd_in, pixel, sizeof(u_int8_t)*3) == -1)
            {
                perror("Eroare la citirea pixelului");
                close(fd_in);
                close(fd_out);
                return;
            }

            u_int8_t grayscale = (u_int8_t)(0.299*pixel[2]+0.587*pixel[1]+0.114*pixel[0]);
            pixel[0]=grayscale; // setarea aceleiași culori de gri pentru cele trei canale
            pixel[1]=grayscale;
            pixel[2]=grayscale;

            if(write(fd_out, pixel, sizeof(u_int8_t)*3) == -1)
            {
                perror("Eroare la scrierea pixelului");
                close(fd_in);
                close(fd_out);
                return;
            }
        }

        lseek(fd_in, padding, SEEK_CUR); // Sărim peste padding, dacă este cazul
        for(int k=0; k<padding; k++)
        {
	  if(write(fd_out, "\0", sizeof(u_int8_t)) == -1)
            {
                perror("Eroare la scrierea padding-ului");
                close(fd_in);
                close(fd_out);
                return;
            }
        }
    }

    close(fd_in);
    close(fd_out);
}


void process_entry(const char *intrare, const char *input_dir, const char *output_dir)
{
  char cale_intrare[PATH_MAX];   // concatenăm numele fișierului de intrare cu directorul
  snprintf(cale_intrare, sizeof(cale_intrare), "%s/%s", input_dir, intrare);
  
  char cale_output_statistica[PATH_MAX];  // Concatenăm numele fișierului de ieșire pentru statistică cu directorul
  snprintf(cale_output_statistica, sizeof(cale_output_statistica), "%s/%s_statistica.txt", output_dir, intrare);

  pid_t pid_statistica = fork();    // Cream un proces fiu pentru statistică

  if(pid_statistica < 0)
    {
      perror("Eroare la fork pentru statistica");
      exit(EXIT_FAILURE);
    }

  if(pid_statistica == 0)
    {
      int fd_output_statistica = open(cale_output_statistica, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if(fd_output_statistica == -1)
	{
	  perror("Eroare la deschiderea fisierului de statistici");
	  exit(EXIT_FAILURE);
        }
     
      generare(intrare, input_dir, fd_output_statistica);
      close(fd_output_statistica);
      exit(EXIT_SUCCESS);
    }
  else
    {
      int status_statistica;
      waitpid(pid_statistica, &status_statistica, 0);
     
        if(WIFEXITED(status_statistica))
	  {
            printf("S-a încheiat procesul pentru statistică cu PID-ul %d și codul %d\n",pid_statistica, WEXITSTATUS(status_statistica));
	  }
	else
	  {
            printf("Procesul pentru statistică cu PID-ul %d nu s-a încheiat normal\n", pid_statistica);
	  }
        if(strstr(intrare, ".bmp") != NULL)
	  {
            pid_t copil_pid_gri = fork();
            if(copil_pid_gri < 0)
	      {
                perror("Eroare la fork pentru conversie la gri");
                exit(EXIT_FAILURE);
	      }
            if(copil_pid_gri == 0)
	      {
                char output_gri[PATH_MAX];
                snprintf(output_gri, sizeof(output_gri), "%s/%s_gri.bmp", output_dir, intrare);

                convert_imagine(cale_intrare, output_gri);
                exit(EXIT_SUCCESS);
	      }
	    else
	      {
                int copil_status_gri;
                waitpid(copil_pid_gri, &copil_status_gri, 0);
                if (WIFEXITED(copil_status_gri))
		  {
                    printf("S-a încheiat procesul pentru conversie la gri cu PID-ul %d și codul %d\n", copil_pid_gri, WEXITSTATUS(copil_status_gri));
		  }
		else
		  {
                    printf("Procesul pentru conversie la gri cu PID-ul %d nu s-a încheiat normal\n", copil_pid_gri);
		  }
	      }
	  }
    }
}


int main(int argc, char *argv[])
{
  if (argc != 3)
    {
      printf("Usage: %s <director_intrare> <director_iesire>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  
    DIR *dir = opendir(argv[1]);
    if (!dir)
      {
	perror("Eroare la deschiderea directorului");
        exit(EXIT_FAILURE);
      }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
      {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
	  {
	    process_entry(entry->d_name, argv[1], argv[2]);
	  }
      }
    
    closedir(dir);
    return 0;
}
