/* PMMail 1.5-2.0 attachement striper 1.2 (C) 1996-1998 Samuel Audet <guardia@cam.org> */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BAGNAME "FOLDER.BAG"
#define BAGBAKNAME "FOLDER.BAK"

int strip_attachements(char *directory, char *msgfilename)
{
   FILE *newfile, *oldfile;
   char newfilename[512], oldfilename[512], line[512], boundary[128] = "";
   enum {storing,uucode,mimebase64,binhex} mode;

   strcpy(newfilename,directory);
   strcat(newfilename,"\\");
   strcat(newfilename,msgfilename);


   strcpy(oldfilename,directory);
   strcat(oldfilename,"\\");
   strncat(oldfilename,msgfilename,strchr(msgfilename,'.')-msgfilename);
   strcat(oldfilename,".BAK");

   if(rename(newfilename,oldfilename))
   {
      fprintf(stderr,"Error, could not rename %s to %s.\n", newfilename,oldfilename);
      return 0;
   }

   newfile = fopen(newfilename,"w");
   if(!newfile)
   {
      fprintf(stderr,"Error opening %s for writing.\n",newfilename);
      return 0;
   }

   oldfile = fopen(oldfilename,"r");
   if(!oldfile)
   {
      fprintf(stderr,"Error opening %s for writing.\n",oldfilename);
      return 0;
   }

   mode = storing;

   while(fgets(line, sizeof(line), oldfile))
   {
      switch(mode)
      {
         case storing:
         {
            char *temp = NULL;
            char upline[512];
            strcpy(upline,line);
            strupr(upline);

            if(!*boundary)
            {
               temp = strstr(upline,"BOUNDARY=\"");
               if(temp)
               {
                  temp += 10;
                  boundary[0] = boundary[1] = '-';
                  sscanf(&line[temp-upline],"%45[^\"]\"",&boundary[2]);
                  fputs(line,newfile);
                  break;
               }
            }

            if(strstr(upline,"BEGIN 6") == upline)
            {
               mode = uucode;
               fputs(line,newfile);
               fputs("\n",newfile);
               break;
            }

            if(strstr(upline,"CONTENT-TRANSFER-ENCODING: BASE64") == upline)
            {
               mode = mimebase64;
               fputs(line,newfile);
               fputs("\n",newfile);
               break;
            }

            if(strstr(upline,"CONTENT-DISPOSITION: ATTACHMENT; FILENAME=\"") == upline)
            {
               mode = binhex;
               fputs(line,newfile);
               fputs("\n",newfile);
               break;
            }

            /* if nothing */
            fputs(line,newfile);
            break;
         }

         case uucode:
            if(strstr(line,"end") == line)
            {
               mode = storing;
               fputs(line,newfile);
            }
            break;

         case mimebase64:
         case binhex:
            if(strstr(line,boundary) == line)
            {
               mode = storing;
               fputs(line,newfile);
            }
            break;
      }
   }


   fclose(newfile);
   fclose(oldfile);
   remove(oldfilename);

   return 1;
}

int main(int argc, char *argv[])
{
   char *bagpath;
   char newbagname[512], oldbagname[512], line[512];
   FILE *newbag, *oldbag;
   int i,e;

   printf("Attachment Stripper 1.2 for PMMail 1.5-2.0 - Copyright 1996-1998 Samuel Audet\n");
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
   e = 0;
   while(fgets(line, sizeof(line), oldbag))
   {
      enum {unread=0,read,replied,sent,unsent} status;
      int attachements = 0;
      char date[16], time[16], size[16];
      char subject[256];
      char fromemail[256],fromname[256];
      char toemail[256],toname[256];
      char msgfilename[16] = "";
      char uninteresting[256] = "";

      i++; /* line counter */

      /* don't use codepage 850 to view this */
      sscanf(line, "%dﬁ%dﬁ%15[^ﬁ]ﬁ%15[^ﬁ]ﬁ%255[^ﬁ]ﬁ%255[^ﬁ]ﬁ%255[^ﬁ]ﬁ%255[^ﬁ]ﬁ%255[^ﬁ]ﬁ%15[^ﬁ]ﬁ%15[^ﬁ]ﬁ%255[^\n]",
         &status,&attachements,date,time,subject,toemail,toname,fromemail,fromname,size,msgfilename,uninteresting);

      if(!*msgfilename)
         fprintf(stderr,"Error: bad entry in %s at line %d, continuing with other entries.\n", oldbagname, i);
      /* KB is our tag, so let's skip these entries */
      else if(attachements && !strstr(size,"KB"))
      {
         struct stat fi;
         char msgpathfilename[272];

         printf("%s\n",msgfilename); e++; /* file processed counter */
         strip_attachements(bagpath, msgfilename);

         strcpy(msgpathfilename, bagpath);
         strcpy(msgpathfilename, msgfilename);
         stat(msgpathfilename, &fi);
         sprintf(size,"%dKB",(int)(fi.st_size/1024.0+1));

         sprintf(line, "%dﬁ%dﬁ%sﬁ%sﬁ%sﬁ%sﬁ%sﬁ%sﬁ%sﬁ%sﬁ%sﬁ%s\n",
            status,attachements,date,time,subject,toemail,toname,fromemail,fromname,size,msgfilename,uninteresting);
      }

      fputs(line, newbag);
   }

   fclose(newbag);
   fclose(oldbag);

   remove(oldbagname);

   printf("%d message file processed.\n",e);

   return 0;
}
