/*This program processes a 200 x 200 BITMAP file containing up to 20 circles with no */
/*circle that touch nor overlap each other, would generate an Graphics Description Language*/
/*file containing commands that creates all the circles preserving each of their */
/*radius and color but placing all of the circles' center at the center of the */
/*BITMAP.  The circles are created in such a way that the largest circle is drawn*/
/*first and the smallest circle is drawn last.  Thus, a circle is partially */
/*covered by the circles smaller than it thus all the circles can be seen in one */
/*place in the BITMAP. The sorting algorithm used to arrange the circles */
/*according to radius is the Shellsort. A SAVE command is also appended at the */
/*end of the command file after the commands for creating the circles are written*/
/*so that a BITMAP named sorted.bmp would show the circles drawn in the desired */
/*if the generated file is placed as an input to a GDL interpreter*/

/*In running this program, this follows the format <executable> <inputbitmapfile>*/
/*<outputcommandfile>.  This program assumes, along with the ones stated earlier, */
/*that all of the points of the circles are in the BITMAP file, that the BITMAP */
/*only contains circles, that the input bitmap file exists and that the BITMAP */
/*would only contain up to 20 circles. In short, all inputs are assumed to be */
/*correct. Even though there are some error checking messages found in the code, */
/*it will not cover all of the possible errors that can possibly occur with wrong*/
/*inputs.*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define MAXPTS 40000 /*total number of pixels in the BITMAP*/
#define MAXCIRC 20 /*maximum number of circles in the BITMAP*/
#define COLSZ 200 /*Horizontal Width of BITMAP*/
#define ROWSZ 200 /*Vertical Height of BITMAP*/
#define BG_COLOR 0 /*Background Color (WHITE)*/

typedef struct IMD {
  char arr[COLSZ+2][ROWSZ+2];
} ImgData; /*contains all the pixel values of all the points in the BITMAP*/

typedef struct RC {
  int color;
  int y_max;
  int y_min;
  int x_max;
  int x_min;
} RawCircle; /*Raw information of a circle*/

typedef struct CD {
  int color;
  int radius;
} CircleData; /*Specific Data of a circle needed for GDL interpreter*/

typedef struct CL {
  int taken;
  CircleData arr[MAXCIRC+1];
} CircleList; /*List of all circles's specfic data in BITMAP*/

/*function prototypes*/
void loadImgData (ImgData*, char []);
void initImgData (ImgData*);
void initCircleList (CircleList*);
void initRawCircle (RawCircle*);
void getCircles (ImgData*, CircleList*, char []);
void extractCircle (ImgData*, RawCircle*, int, int);
void makeRawCirc (RawCircle*, int, int);
CircleData convertRawCircToCircleData (RawCircle*);
void appendCircle (CircleData , CircleList* );
void generateCommands (CircleList*, char []);
void shellSortByRadius(CircleData a[], int); 

/*main function*/
int main(int argc, char **argv)
{
  ImgData color_map;
  CircleList circ_list;

  if (argc != 3)
    {
      printf ("Invalid number of arguments!");
      exit(1);
    }

  initCircleList (&circ_list);
  loadImgData (&color_map, argv[1]);
  getCircles (&color_map, &circ_list, argv[2]);

  return 0;
}

/*initializes the values of structure CircleList*/
void initCircleList (CircleList* dat){
  /*set to 1 as place of first element to suit shellsort implementation below*/
  dat->taken=1; 
}

/*loads the pixel values of all the points in BITMAP into structure ImgData*/
void loadImgData (ImgData* dat, char in[]){

  int i, num_read, header_size;
  FILE* infile;
  char color;
  struct stat file_stat;

  stat(in, &file_stat);
  header_size = file_stat.st_size - MAXPTS;
    
  infile = fopen(in, "rb");
  if (infile == NULL) {
    printf("Error unable to open %s, exiting.", in);
    exit(1);
  }

  /*places file points at the start of data part of BITMAP file*/
  fseek(infile, header_size*sizeof(char), SEEK_SET);

  /*pixel values being loaded into structure*/
  for(i=0 ; i < 40000; i++)
    {
      num_read = fread(&color, sizeof(char), 1, infile);
      if (num_read < 1) {
	printf("Warning: able to read only %d integers", i);
      }
      dat->arr[(i / ROWSZ)+1][(i % COLSZ)+1] = color;
    }
  
  fclose(infile);
  
}

/*initializes values of structure ImgData*/
void initImgData (ImgData *dat)
{
  int count, count2;
  for (count = 0; count < ROWSZ+2; count++)
    {
      for (count2 = 0; count2 < COLSZ+2; count2++)
	{
	  dat->arr[count][count2] = 0;
	}
    }
}

/*gets every circle in BITMAP, places them to structure CircleList and finally*/
/*generates the command file*/
void getCircles (ImgData* color_map, CircleList* circ_list, char out[]) 
{
  int y,x;
  RawCircle raw_circ;
  CircleData circ_data;
  
  /*start of search from left to right, from bottom to top*/
  for(y=1; y <= ROWSZ; y++)
    {
      for(x=1; x <= COLSZ; x++)
	{
	  if (color_map->arr[y][x] != BG_COLOR)
	    {
	      initRawCircle(&raw_circ);
	      raw_circ.color = color_map->arr[y][x];
	      extractCircle(color_map, &raw_circ , x, y);
	      circ_data = convertRawCircToCircleData(&raw_circ);
	      appendCircle(circ_data, circ_list);
	    }
	  
	}
    }
  generateCommands(circ_list, out); 
}

/*Extracts all points of a circle and places all the raw information into */
/*structure RawCircle*/

void extractCircle (ImgData* color_map, RawCircle* raw_circ, int x, int y) 
{
  int i,j;

  if (color_map->arr[y][x] == BG_COLOR)
    {
      return; /* stopping case */
    }
  else 
    {
      makeRawCirc (raw_circ, x, y);
      color_map->arr[y][x] = BG_COLOR;
      
      /*recursive call starting the depth first search*/
      for (j=-1; j<=1; j++)
	{
	  for (i=-1; i<=1; i++)
	    {
	      extractCircle(color_map, raw_circ, x+i, y+j);
	    }
	}
    }

}

/*makes the raw circle by gathering raw information from all of the points of */
/*the circle*/
void makeRawCirc(RawCircle* circ, int x, int y) 
{
  
  if (y > circ->y_max) circ->y_max = y;
  if (y < circ->y_min) circ->y_min = y;
  if (x > circ->x_max) circ->x_max = x;
  if (x < circ->x_min) circ->x_min = x;
}

/*initializes all the values in structure RawCircle*/
void initRawCircle (RawCircle* raw_circ) 
{
  /*raw circle color set to background color*/
  raw_circ->color = BG_COLOR;
  /*max value holders set to min value*/
  raw_circ->y_max = 1; 
  raw_circ->x_max = 1;
  /*min value holders set to max value*/
  raw_circ->y_min = 200;  
  raw_circ->x_min = 200;
}

/*converts raw information of circle to information for GDL interpreter*/
CircleData convertRawCircToCircleData (RawCircle* raw_circ) {


  CircleData circ_data;
  int radx, rady;

  circ_data.color = raw_circ->color;
  
  /*Raduis is determined*/
  radx = (raw_circ->x_max - raw_circ->x_min)/2;
  rady = (raw_circ->y_max - raw_circ->y_min)/2;

  if (radx != rady)
    {
      printf("Radius Error");
      exit(1);
    }
  else circ_data.radius = radx;

  return circ_data;
}

/*adds a circle's specific data to the structure CircleList*/
void appendCircle(CircleData circ_data, CircleList* circ_list) 
{
  circ_list->arr[circ_list->taken] = circ_data;
  circ_list->taken++;
}

/*sorts structure CircleList then generates IGuhit command file*/
void generateCommands(CircleList* circ_list, char out[]) 
{
  FILE *outfile;
  int count, count2;

  /*CircleList is sorted*/
  shellSortByRadius(circ_list->arr, circ_list->taken - 1);
  
  outfile = fopen(out, "w");
  if (outfile == NULL)
    {
      printf ("ERROR in creating file %s", out);
      exit(1);
    }

  /*command file generated with create circle commands by reading CircleList */
  /*backwards since CircleList was sorted in ascending radius order (convention)*/

  count2 = 1;
  for (count = circ_list->taken - 1; count >= 1; count--)
    {
      fprintf (outfile, "C%d %d,%d,%d,%d\n", count2, 100, 100, circ_list->arr[count].radius, circ_list->arr[count].color);
      count2++;
    }
  /*SAVE command appended in command file*/
  fprintf(outfile, "SAVE sorted.bmp\n");
  fclose (outfile);
	      
}

/*Sorts circles in increasing radius order by Shellsort*/
void shellSortByRadius(CircleData data[], int N) 
{
  int x, y, z; 
  CircleData value;
  for(z=1; z<=N/9; z=3*z+1) ;
  for( ; z>0; z/=3)
    for(x=z+1 ; x<=N; x+=1)
      {
	value = data[x]; 
	y=x;
	while(y>z && data[y-z].radius > value.radius)
	  {
	    data[y] = data[y-z];
	    y-=z;
	  }
	data[y]=value;
      }
}
