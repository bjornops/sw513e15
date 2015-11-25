#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h> 

int main(int argc, char *argv[])
{
    int sendRequest = 0;
    char openFileName[50];

    // Argumenter!
    if(argc == 2)
    {
        if(strcmp(argv[1], "request") == 0) // Send request!
        {
            sendRequest = 1;
        }
        else if(strcmp(argv[1], "success") == 0) // Request er sendt!
        {
            sendRequest = 2;
        }
        else
        {
            sendRequest = 3;
            strcpy(openFileName, argv[1]);
        }
    }

    // Find solution PID
    int solutionPID = -1;
    FILE *pidFile;
    pidFile = fopen("/var/run/wasp.pid", "r");
    if(pidFile != NULL)
    {
        fscanf(pidFile, "%d", &solutionPID);
    }

    // Header
    printf("Content-Type: text/html\n\n");

    // Indhold start
    printf("<html><head><link rel='stylesheet' type='text/css' href='/style.css'><title>WASP - Administration</title></head><body>");

    // Header
    printf("<div id='head'>");
    printf("<h2>WASP Administration</h2>");
    printf("<p>Wireless Arduino Sensor Protocol</p>");
    printf("</div>");

    // Nav
    printf("<div id='nav'>");
    if(sendRequest == 0)
    {
        printf("<input type='button' value='Request data' onclick=\"window.location='?request';\">");
    }

    printf("<div style='float:right;'>");
    // PID for solution
    if(solutionPID != -1)
    {
        printf("<div style='margin-top:5px;margin-right:10px;'>Fundet solution med PID: %d</div>", solutionPID);
    }
    printf("</div>");

    printf("</div>");

    // Content
    printf("<div id='content'><br />");


    // Status
    if(sendRequest == 1 && solutionPID != -1)
    {
        printf("<p>Sender requests!</p>");
        kill(solutionPID, SIGUSR1);
        printf("<script>window.location='?success';</script>");
    }
    else if(sendRequest == 2)
    {
        printf("<p>Request sendt!</p>");
    }

    // Resultater
    if(sendRequest != 3)
    {
        printf("<br /><br /><b>Tidligere resultater:</b>");
        DIR *d;
        struct dirent *dir;
        d = opendir("/home/pi/wasp/results");
        if(d)
        {
            printf("<ul>");
            while ((dir = readdir(d)) != NULL)
            {
                if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, ".."))
                {
                    printf("<li><p><a href='?%s'>%s</a></p></li>", dir->d_name, dir->d_name);
                }
            }
            printf("</ul>");

            closedir(d);
        }
    }
    else // Åbner fil!
    {
        char path[200] = "/home/pi/wasp/results/";
        strcat(path, openFileName);

        FILE *resFile;
        resFile = fopen(path, "r");

        printf("<a href='/cgi-bin/WASP'>&larr; Tilbage</a><br />");
        printf("<br /><b>Resultater fra '%s'</b>", openFileName);

        if(resFile != NULL)
        {
            int node = -1, value = -1;
            while(fscanf(resFile, "%d:%d", &node, &value) != EOF)
            {
                printf("<p>Node %d havde værdi: %d</p>", node, value);
            }

            fclose(resFile);
        }
        else
        {
            printf("<p>Ingen resultater her!</p>");
        }
    }

    printf("</div>");

    // Footer
    printf("<div id='footer'>");
    printf("<p>WASP - SW513E15</p>");
    printf("</div>");

    // Indhold slut (Lav footer fil)
    printf("</body></html>");

    return 0;
}
