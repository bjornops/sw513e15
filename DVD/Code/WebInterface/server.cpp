#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <map>

void printNewlines(int);

int main(int argc, char *argv[])
{
	std::map<int, char *> nodeMapping;
    int sendRequest = 0;
    char openFileName[50];

    // Argumenter!
    if(argc == 2)
    {
        if(strcmp(argv[1], "request") == 0) // Send request!
        {
            remove("/var/www/html/ping.txt");
            sendRequest = 1;
        }
        else if(strcmp(argv[1], "success") == 0) // Request er sendt!
        {
            sendRequest = 2;
        }
        else // Filename, hopefully
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
    printf("<html><head><meta http-equiv='Content-Type' content='text/html; charset=UTF-8'/><link rel='stylesheet' type='text/css' href='/style.css'><title>WASP - Administration</title></head><body>");

	// Læs navne fra wasp.conf!
	FILE *optionsFile;
    optionsFile = fopen("/home/pi/wasp/wasp.conf", "a+"); 
    if(optionsFile != NULL)
    {
        int nodeID = 0;
        char nodeName[100];
    	while(fscanf(optionsFile, "%d*%s", &nodeID, &nodeName) != EOF)
    	{
	    	// Haxx.
	    	char *tmp;
	    	tmp = (char *)malloc(100*sizeof(char));
	    	strcpy(tmp, nodeName);
	    	
            nodeMapping[nodeID] = tmp;
        }
        fclose(optionsFile);
    }
    
    // Header
    printf("<div id='head'><h2><a href='/cgi-bin/WASP'>WASP Administration</a></h2><p>Wireless Arduino Sensor Protocol</p></div>");

    // Nav
    printf("<div id='nav'>");
    if(sendRequest == 0)
    {
        printf("<input type='button' value='Hent data' onclick=\"window.location='?request';\">");
    }

    // PID for solution
    if(solutionPID != -1)
    {
        printf("<div style='float:right;'><div style='margin-top:5px;margin-right:10px;'>Fundet solution med PID: %d</div></div>", solutionPID);
    }

    printf("</div>");

    // Content
    printf("<div id='content'><br />");


    // Status
    if(sendRequest == 1 && solutionPID != -1)
    {
        kill(solutionPID, SIGUSR1); // Starts request!
        printf("<script>window.location='?success';</script>");
    }
    else if(sendRequest == 2)
    {
        printf("<br /><center><img src='/loading.gif'/></center><br />");
    }

    // Resultats
    if(sendRequest != 3)
    {
        if(sendRequest != 0 && sendRequest != 2) // pwetty formatting
        {
            printNewlines(2);
        }

        printf("<b>Tidligere resultater:</b><br />");
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
            printf("</ul><br />");

            closedir(d);
        }
        printf("<br /><br /><b>Kendte noder:</b><br />");
        printf("<ul>");
        std::map<int, char *>::iterator it;
        for (it = nodeMapping.begin(); it != nodeMapping.end(); it++)
        {
	        printf("<li><p>Node %d, med navn '%s'</p></li>", it->first, it->second);
	    }
	    printf("</ul><br /><br />");
    }
    else // Åbner fil!
    {
	    // Læs og print resultater
        char path[200] = "/home/pi/wasp/results/";
        strcat(path, openFileName);

        FILE *resFile;
        resFile = fopen(path, "r");

        printf("<a href='/cgi-bin/WASP'>&larr; Tilbage</a><br />");
        printf("<br /><b>Resultater fra '%s'</b><br /><br />", openFileName);
			
        if(resFile != NULL)
        {
            printf("<table cellspacing='0'><tr><td width='200px' style='border-bottom: 1px solid black;'><b>Node</b></td><td style='border-bottom: 1px solid black;'><b>Værdi</b></td></tr>");
            int node = -1, value = -1;
            int even = 0;
            while(fscanf(resFile, "%d:%d", &node, &value) != EOF)
            {
	            // Farve på row i tabel
                char *style;
                if(even == 0)
                {
                    style = "";
                    even = 1;
                }
                else if(even == 1)
                {
                    style = " style='background-color: rgb(228, 228, 228);'";
                    even = 0;
                }
                
                // Value!
                char *val;
                val = (char *)malloc(15*sizeof(char));
                
                if(value == -1)
                {
	                val = "Ingen modtaget";
                }
                else
                {
	                sprintf(val, "%d", value);
                }
                
                printf("<tr><td%s>%s</td><td%s>%s</td></tr>", style, nodeMapping[node], style, val);
            }
            printf("</table>");

            fclose(resFile);
        }
        else
        {
            printf("<p>Ingen resultater her!</p>");
        }
    }

    printf("</div>");

    // Footer
    printf("<div id='footer'><p>WASP - SW513E15</p></div>");

    if(sendRequest == 2)
    {
        printf("<script>var interval = setInterval(testFile, 1000); function testFile() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (xhttp.readyState == 4 && xhttp.status == 200) { window.location='?'+xhttp.responseText; } }; xhttp.open('GET', '/ping.txt', true); xhttp.send(); } </script>");
    }

    // Indhold slut (Lav footer fil)
    printf("</body></html>");

    return 0;
}

// Important stuff.
void printNewlines(int num)
{
    for(int n = 0; n < num; n++)
    {
        printf("<br />");
    }
}
