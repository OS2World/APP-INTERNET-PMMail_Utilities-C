/* PMMail 1.5-2.0 attachement striper 1.2 (C) 1996-1998 Samuel Audet <guardia@cam.org> */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BAGNAME "FOLDER.BAG"
#define BAGBAKNAME "FOLDER.BAK"

int main(int argc, char *argv[])
{
   char *bagpath;
   char newbagname[512], oldbagname[512], line[512];
   FILE *newbag, *oldbag;
   int i;

   printf("Sent Status Changer 1.2 for PMMail 1.5-2.0 - Copyright 1996-1998 Samuel Audet\n");
   if(argc > 1)
   {
      char *temp;
      bagpath = argv[1];
      temp = strrchr(bagpath,'\\')+1;
      if(*(temp+1) == 0) *temp = 0;
   }
   else
      bagpath = ".";

   strcpy(newbagname,bagpath);
   strcat(newbagname,"\\");
   strcat(newbagname,BAGNAME);

   strcpy(oldbagname,bagpath);
   strcat(oldbagname,"\\");
   strcat(oldbagname,BAGBAKNAME);

   if(rename(newbagname,oldbagname))
   {
      fprintf(stderr,"Error, could not rename %s to %s.\n", newbagname,oldbagname);
      return 1;
   }

   newbag = fopen(newbagname,"w");
   if(!newbag)
   {
      fprintf(stderr,"Error opening %s for writing.\n",newbagname);
      return 2;
   }
   oldbag = fopen(oldbagname,"r");
   if(!oldbag)
   {
      fprintf(stderr,"Error opening %s for reading.\n",oldbagname);
      return 3;
   }

   i = 0;
   while(fgets(line, sizeof(line), oldbag))
   {
      /* enum {unread=0,read,replied,sent,unsent} status; */

      i++; /* line counter */

      line[0] = '3'; /* the first column is the status byte */
      fputs(line, newbag);
   }

   fclose(newbag);
   fclose(oldbag);

   remove(oldbagname);

   printf("%d entries processed.\n",i);

   return 0;
}
