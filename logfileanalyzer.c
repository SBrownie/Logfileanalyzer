#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <mysql/my_global.h>
#include <stdbool.h>

char* smtpdarr = "";
char smtparr[5000];
char amavisarr[5000];
char qmgrarr[5000];
bool analsondenstatus = false;
char* gquery; 
//Datenbankvariablen
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
char *server = "localhost";
char *user = "root";
char *password = "root";
char *database = "logdb";

static void servercheck(){

}

//Die tiefe gibt die absolute Position des gewünschten Darminhalts an
static char* darmspiegelung(char *analarr, int tiefe, char* darmzotte){
		char *analtok;
		analtok = strtok(analarr,darmzotte);
		int i = 0;
		for(i=0;i<tiefe-1;i++){
			analtok = strtok(NULL, darmzotte); 
//		printf("analtok: %s\n", analtok);
		}
		return analtok;
}

static void smtpd(char* analcontent){
	char* analarr;
	char* query;
	asprintf(&smtpdarr, "%s%s ", smtpdarr , analcontent);	
	strcat(smtpdarr," ");
	//abbruch wenn disco
	if(strstr(analcontent,"disconnect")){
		//jetz alled in db
		//TODO server checken & einlesen 
		analsondenstatus = true;
		char* analtok;
		//maid finden
		asprintf(&analarr, "%s", smtpdarr);
		analtok = darmspiegelung(analarr, 14, " ");
		analtok = strtok(analtok,":");
		char* maid;
		asprintf(&maid, "%s", analtok);
		printf("maid: %s\n", maid);
		//maid check&create
		asprintf(&query, "SELECT ma_id FROM mail_id WHERE ma_id = '%s';", maid);
puts(query);
		if (mysql_query(conn, query)) {fprintf(stderr, "%s\n", mysql_error(conn));}
		res = mysql_use_result(conn);
		//row = mysql_fetch_row(res);
        row = mysql_fetch_row(res);

		if(row == NULL){puts("row is null");}
		
		asprintf(&gquery,"%s", query);

		//fqdn + IP finden
		asprintf(&analarr, "%s", smtpdarr);
		analtok = darmspiegelung(analarr, 8, " ");
		char* fqdn;
		char* IP;
		analtok = strtok(analtok,"[");
		asprintf(&fqdn, "%s", analtok);
		analtok = strtok(NULL,"]");
		asprintf(&IP, "%s", analtok);
		printf("fqdn: %s\n", fqdn);
		printf("IP: %s\n", IP);
		//Datum
		asprintf(&analarr, "%s", smtpdarr);
		char* datum;
		analtok = strtok(analarr," ");
		asprintf(&datum, "%s", analtok);
		printf("datum: %s\n",datum);
		analtok = strtok(NULL," ");
		asprintf(&datum, "%s %s", datum, analtok);
		printf("datum: %s\n", datum);
        //Querystring zusammenkleistern
		asprintf(&query, "INSERT INTO smtpd (ma_id, fqdn, IP, datum) Values('%s','%s','%s','%s');", maid, fqdn, IP, datum);
		asprintf(&gquery,"%s", query);
	}
}

static void smtp(char* analcontent){
	strcat(smtparr,analcontent);
	if(strstr(analcontent,"disconnect")){
		//jetz alled in db
		
	}
}

static void amavis(char* analcontent){
	strcat(amavisarr,analcontent);
	if(strstr(analcontent,"disconnect")){
		//jetz alled in db
		
	}
}

static void qmgr(char* analcontent){
	strcat(qmgrarr,analcontent);
	if(strstr(analcontent,"disconnect")){
		//jetz alled in db
		
	}
}

static void cleanup(char* analcontent){
}

static void parse(){
    //Datenbankverbindung
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server,
          user, password, database, 0, NULL, 0)) {
       fprintf(stderr, "%s\n", mysql_error(conn));
    }

    char dirpath[99];
    puts("type in complete directory path:");
    scanf("%s", dirpath);
    DIR *dir = opendir(dirpath);
    struct dirent *d;
    FILE *logfile;
    char filepath[99];
    char content[1000];
    char *ANALyzer;

    printf("Reading files from: %s\n\n",dirpath);
    while((d = readdir(dir)) != NULL)
    {
        //. .. aussortieren
	if(strcmp(d->d_name,".") != 0 && strcmp(d->d_name,"..") != 0)
        {
	    	strcpy(filepath,dirpath);
            strcat(filepath,d->d_name);
            printf("analyzing logfile: %s\n",filepath);
            logfile = fopen(filepath,"r");
            if(logfile == NULL){puts("Please add a '/' at the end of your directory path. [ example: /home/user/exampledir/ ]");}

            while(fgets(content,999,logfile))
            {
				if(strstr(content, "smtpd")){
					smtpd(content);
					//printf("query: %s\n",gquery);
					if(analsondenstatus){
						//mysql_query(conn, gquery);
						if (mysql_query(conn, gquery)) {
             			fprintf(stderr, "%s\n", mysql_error(conn));
           				}
						analsondenstatus = false;
					}
            	}
			}

            fclose(logfile);
        }
    }

    closedir(dir);
    mysql_close(conn);
}

static void statistic(){

}

static void menu(){
    //Hauptmenü
    puts("Logfileanalyzer by StefpeeEkh's (Gruppe5)");
    puts("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    while(1){
        puts("[1] - parse logfiles");
        puts("[2] - create statistics");
        puts("[3] - exit");

        int x;
        scanf("%i",&x);
        switch(x){
            case 1:
                parse();
                break;
            case 2:
                statistic();
                break;
            case 3:
                exit(0);
                break;
            default:
    			printf ("\e[30m\e[47m");
                puts("Du hast soeben die Pforte zur Matrix gefunden | Zeit ist relativ | Jetzt, Jetzt, Jetzt, Jetzt, Jetzt");
                break;
        }
    }
}

int main(){
    menu();
    return 0;
}
