#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    int sendRequest = 0;
    // Argumenter!
    for(int n = 0; n < argc; n++)
    {
        if(strcmp(argv[n], "request") == 0) // Send request!
        {
            sendRequest = 1;
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
    printf("</div>");

    // Content
    printf("<div id='content'><br />");

    if(sendRequest == 1 && solutionPID != -1)
    {
        printf("<p>Sender requests!</p>");
        kill(solutionPID, SIGALRM);
    }

    if(solutionPID != -1)
    {
        printf("<p>Fundet solution med PID: %d</p>", solutionPID);
    }
    else
    {
        printf("<p>Solution kører ikke!</p>");
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
