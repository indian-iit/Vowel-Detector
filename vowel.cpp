//Below programe will take an audio files which are decoded as text file as an input.
//The input file has one letter which is a vowel{'a','e','i','o','u'}
//The  programe will predict the vowel present in the file with help of tohkura distance.
//Adding the required header files and libraries
#include "stdafx.h"
#include "stdafx.h"
#include<stdio.h>
#include<conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<math.h>
#include<string.h>
#include <limits>
#define pi 3.141592653
#define frames 5
//We are defining frames as 5 beacause in every file we will consider only 5 contiguos frames those are stable.
//As part of this programe we consider 320 samples for frame.
//limit variable is used in normalisation.
//5000 signifies the maximum amplitute a wave can posess.
//correct_count and total_correct_count are helpful to compute accuracy of this programe.
double limit=5000;
int correct_count=0,total_correct_count=0;
//Declaring required file pointers which will be used in the below programe.
FILE *input_normalisation,*input_file2,*input_file1,*refer_file;
//array ri is used to store ri values similary ai and ci are used to ai,ci values respectively.
//We need only current file ri,ai values only where as we need to store all files ci values used in training.
//array steady is used to store the sample values present in 5 steady frames.
double normalisation[322],ri[5][13],ai[5][13],ci[11][5][13],steady[5][320],sample_data[120][320] = {{0}},avg_ci[5][13];
//Initialising tokhura distance.
double tokhura_weights[12]={1.0, 3.0, 7.0, 13.0, 19.0, 22.0, 25.0, 33.0, 42.0, 50.0, 56.0, 61.0};
char vowels[]={'a','e','i','o','u'}, open_file[100],generate_file[100];
double dc_Shift,samples_Count;

//Below function will take the dc.txt file as input and updates the dcshfit value.
//DCshif depends on hardware so no need to calculate on every file.
//Dcshift value will also helps us to find whether the hardware is in good condition or not.
void calculate_dcshift(FILE *input_file)
{
	//"Rewind" function will make the file pointer to point the begining of file.
	rewind(input_file);
	//Intialisaing the required variables.
	char singlevalue[100];
	samples_Count=0;
	dc_Shift=0;
	int x=0;
	//feof indicates end of the file.
	//Until we reach the end of the file we keep on adding the values present in the file to dc_Shift.
	while(!feof(input_file))
	{
	    fgets(singlevalue,100, input_file);
	    //Whenever we extract a charecter from the file we will check whether it is a digit or not with the help of isalpha function.
	    //This is done to aoid the compuation on header information present in the file.
	    if(!isalpha(singlevalue[0]))
	    {
	     	//atoi function will covert the charecter arry to integer.
			x = atoi(singlevalue);
			dc_Shift+=x;
			samples_Count++;
		 }
    }
	//Finally we compute the average dcshift by adding all values in file divided by no of samples present in the file.
	dc_Shift =dc_Shift/samples_Count;
}

//The below function reads data from the file pointed by input_file and stores the output("Normalised data") in file pointed by output_normalisation.
void apply_normalisation(FILE *input_file,FILE *output_normalisation)
{
	int count1=0;
	//Making the pointers of file pointing to the begining of the file.
	rewind(input_file);
	rewind(output_normalisation);
	//Initialising the required variables.
	double max=0,x=0;
	int count=0;
	char value[100];
	double normalisation_Factor = 0;
	while(!feof(input_file))
	{
       fgets(value, 100, input_file);
	   //Checking whether the value read from input file is alphabet or a digit, this is done to avoid computation on header information present in file.
	   if(!isalpha(value[0]))
	   {
		  x = atof(value);
		  count++;
		  //if current max less that value read from the file we update the current max.
		  if(abs(max) < abs(x))
		  {
		     max = abs(x);
		  }
	   }
	}
	//normalised_factor is calculated based on below formula where limit denotes 5000.
	//With the help of this normalised_factor we will increase the amplitute of the wave form if max value is less than 5000.
	//Similary we will reduce amplitude of wave form if max value is more than 5000.
	normalisation_Factor = limit/max;
	//Making the pointer of file pointing to the begining of the file.
	rewind(input_file);
	while(!feof(input_file))
	{
	   fgets(value, 100, input_file);
	   if(!isalpha(value[0]))
	   {
		    x = atof(value);
		    normalisation[count1]=x;
		    //Before performing the multiplication operation, we will subtract the dcshift from the values present in the file to get more accurate results.
		    double normalised_output = floor((x-dc_Shift) * normalisation_Factor);
		    fprintf(output_normalisation, "%lf\n", normalised_output);
		}
	}
}

//The below programe will compute ri values of stable frames store it an 2d array.
//The array used is of size 5*13 where 5 inicates no of stable frame.
//For each stable frame we need to store 13 values.
void compute_ri()
{
   for(int k=0;k<5;k++)
   {
	 for(int i=0;i<=12;i++)
	 {
		//Before using the array we will be initialising the array,so that we can reuse the same array for every file.
		//This will help us to reduce the space consumed by the programe.
		ri[k][i]=0;
		for(int j=0;j<320-i;j++)
		{
		  //We will be cummulatively adding the ri values for all 320 samples present in a frame.
		  ri[k][i]+=steady[k][j]*steady[k][j+i];
		}
	 }
   }
}

//The below programe will compute ri values of stable frames store it an 2d array.
//The array used is of size 5*13 where 5 inicates no of stable frame.
//For each stable frame we need to store 13 values.
void compute_ai()
{
	//Initialising the required variables and arrays.
	//Here we are implementing Durbin's algo.
	//In order to compute ai values we are using ri values.
	double e[13]={0},y[13]={0};
	double alpha[13][13]={{0}};
	double temparary_sum=0;
	//Here we need to iterate over all 5 frames, so using a for loop which itertes for 5 times.
	for(int count=0;count<5;count++)
	{
	  //initialising e[0]
	  e[0]=ri[count][0];
      for(int k=1;k<=12;k++)
	  {
		 //Initialising temparary_sum variable so that previously stored value will be erased.
		 temparary_sum=0;
		 //Following algorithm steps.
		 for(int l=1; l<=k-1; l++)
		 {
            temparary_sum += (alpha[k-1][l] * ri[count][k-l]);
         }
		 y[k] = (ri[count][k] - temparary_sum)/e[k-1];
		 alpha[k][k] = y[k];
		 for(int j=1; j<=k-1; j++)
		 {
            alpha[k][j] = alpha[k-1][j] - (y[k] * alpha[k-1][k-j]);
         }
		 e[k]=(1-y[k]*y[k])*e[k-1];
	  }
	  //Copying computed ai values into destination array from alpha array.
	  for(int i=1;i<=12;i++)
	  {
		ai[count][i]=alpha[12][i];
	  }
   }
}

//The below programe will compute ri values of stable frames store it an 3d array.
//The array used is of size  11*5*13 where 5 inicates no of stable frame.
//Where 11 indiactes the no of files used in testing.
//For each stable frame we need to store 13 values.
//We are using 3d array instead of 2d array further we need to compute avg of ci values.
//Here utterence indicaes the file number.
void compute_ci(int utterence)
{
	//Decalaring and initialising required variables.
	double temp=0;
	for(int count=0;count<5;count++)
	{
	   //Computing sigma square
	   ci[utterence][count][0]=log(ri[count][0]*ri[count][0]);
       for(int x=1;x<=12;x++)
	   {
		  //Initailising the array whereever we are using so that we can reuse the same array in training and testing.
		  //This will help us to reduce the space consumed by the programe.
		  ci[utterence][count][x]=0;
		  //We are re-initialising the variable temp beacuse if any data that is produced in previous iteration is present it can be erased.
		  temp=0;
		  for(int k=1;k<=x-1;k++)
		  {
			temp+=(k*ci[utterence][count][k]*ai[count][x-k])/x;
		  }
		  ci[utterence][count][x]=ai[count][x]+temp;
	   }
	}
}

//The below function is useful to apply raised sine window on ci values
//The argument parameter here indicates the file no which we are dealing with.
//File_no helps us to acess the ci values present in the 3d array.
void apply_raised_sine(int utterence)
{
  //Declaring and initialising required variables.
  double temp=0;
  double thetha=0;
  for(int count=0;count<5;count++)
  {
	for(int m=1;m<=12;m++)
	{
		//re-initialising temp and thetha so that values obtained in previous iterations are erased.
		temp=0;
		thetha=0;
		//Applying raised sine formula
		thetha=pi*m;
		thetha=thetha/12;
	    temp=1+(6*sin(thetha));
	    ci[utterence][count][m]=ci[utterence][count][m]*temp;
	}
  }
}

//The below function helps us to identify the steady frames in the file.
//We will identify 5 contiguos frames as steady frames and store it in 2d array steady.
//Input points to the text file for which we need to identify the steady frames.
void steady_frames(FILE *input)
{
	//Making file pointer to point to the begining of the file.
	rewind(input);
	//Declaring and initilising the required variables.
	int position = 0,sample_no = 0, frame_no  = 0,max_position;
	double  energy[100] = {0};
	//Array value is used to read data from the file.
	char value[100];
	double cummulative_sum=0,max_energy=0;
	//We will iterate till we reach the end of the file.
	while(!feof(input))
	{
		fgets(value, 50, input);
		//We are considering 320 samples in a single frame.
		if(sample_no >= 320)
		{
			    //Once scanning of 320 samples is completed, we will re_initialie our sample_no(which tells sample_count) and increment frame_no.
				frame_no++;
				sample_no=0;
		}
		//We are initilising sample array beacause data computes=d in previous iterations will be erased.
		sample_data[frame_no][sample_no]=0;
		sample_data[frame_no][sample_no] = atof(value);
		sample_no++;
	}
	//Once we have stored data of all the frames we will proceed with computing energies of these frames.
	for(int i=0; i<frame_no; i++)
	{
		//We will squaring the normalised data to compute the energy.
		for(int j=0; j<320; j++)
		{
			cummulative_sum = cummulative_sum+(sample_data[i][j] * sample_data[i][j]);
		}
		energy[i] = cummulative_sum/(320);
		//Re-initialising cummulative_sum as we are entering into the next frame.
		cummulative_sum=0;
	}
	//No we are finding the frame which is having maximum energy.
	for(int j=0; j<frame_no; j++)
	{
		//If current-maximum energy is less than current frame energy we will update our max_energy and also position where maximum-energy is occuring.
		if(energy[j] > max_energy)
		{
			max_position = j;
			max_energy = energy[j];
		}
	}
	//Once we compute the maximum energy we assume 2 frames on the left and 2 frames on right of maximum-energy frame, along with maximum-energy frame as steady grames.
	int count = 0;
	//Finding starting and end frames of steady frames.
	int start=max_position-2;
	int end=max_position+2;
	for(int b=start; b<=end; b++)
	{
		for(int a=0; a<320; a++)
		{
			//Storing data of steady frames in steady array.
			steady[count][a]=0;
			steady[count][a] = sample_data[b][a];
		}
		count++;
	}
}

//The below function will compute average ci values over all testing files.
//We are storing average values in array avg_ci.
//Finally we will end up having 5 rows and 12 columns of ci values.
void compute_ci_average()
{
	for(int a=0;a<5;a++)
	{
		for(int b=0;b<=12;b++)
		{
			//Variable Utterence indicates file number on which testing is done.
			for(int utterence=1;utterence<=10;utterence++)
			{
				//Cummulatively adding all values present in frame a and bth ci value. 
				avg_ci[a][b]+=ci[utterence][a][b];
			}
			//We are dividing by 10 as we are using 10 files for testing.
			avg_ci[a][b]=avg_ci[a][b]/10;
		}
	}
}

//We will generating a refernece file which will be used in training part.
//We will writing in the file pointed by pointer output.
//For each vowel one refernece file will be generated.
void generate_reference_file(FILE *output)
{
	for(int a=0;a<5;a++)
	{
		for(int b=0;b<12;b++)
		{
			//Copying average values of ci into the file pointed by output pointer.
			fprintf(output, "%lf,", avg_ci[a][b]);
		}
		//Writing into the next line as we are goining with next frame.
		fprintf(output, "%lf\n", avg_ci[a][12]);
	}
}

//The below function is used in testing part.
//The below function will calculate the tokhura distance between he file pointed by reference file and the reference file of testing vowel.
//Variable utterence indicates file number on which testing is being done.
//The below function will return the tokuhra distance.
double tokhura_Distance(FILE *reference_file,int utterence)
{
	//Making pointer of the file to point ot th begining of the file.
	rewind(reference_file);
	//Declaring and initialising the required variables.
	int count=0;
	double ci_reference[5][13];
	char value[2048];
	//We will be reading the values present in the file either until we reach the end of the file or we have completed reading all the 5 rows/frames.
	while(!feof(reference_file)&&count<5)
	{
		fgets(value,2048,reference_file);
		//Copying all the 12 values values present in the row to my reference_array and then moving on to the next line/row in the file.
		sscanf(value, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", &ci_reference[count][0], &ci_reference[count][1], &ci_reference[count][2], &ci_reference[count][3], &ci_reference[count][4], &ci_reference[count][5], &ci_reference[count][6], &ci_reference[count][7], &ci_reference[count][8], &ci_reference[count][9], &ci_reference[count][10], &ci_reference[count][11], &ci_reference[count][12]);
		//Incrementing my frame count has we have completed reading a row in the file.
		count=count+1;
	}
	//Declaring and initialising the required variables.
	double total_distance,temp_distance,temp;
	temp=total_distance=temp_distance=0;
	for(int a=0;a<5;a++)
	{
		temp_distance=0;
		for(int b=1;b<=12;b++)
		{ 
			//re-initilising variable temp so that values present in temp due to previous iteration will be erased.
			temp=0;
			temp=ci[utterence][a][b]-ci_reference[a][b];
			temp_distance+=tokhura_weights[b-1]*temp*temp;
		}
		//We are diving temp_distance by 12 as we there are 12 colums in a row.
		total_distance+=(temp_distance/12*1.0);
	}
	//Dividing total distance by 5 has there are 5 rows in refernece file.
	return (total_distance/5);
}

//Entry point of programe.
int main()
{
	//Declaring required variables.
	double distance;
	char predicted_vowel;
	//Making input_file pointing to dc.txt which is used to calculate dc_shift.
	input_file1=fopen("dc.txt","r");
	calculate_dcshift(input_file1);
	//For each vowel we consider 20 recordings first 10 recordings are used for training and remaining 10 recordings are used for testing.
	printf("Performing Training");
	//We will be performing training on all 5 vowels.
	for(int vowel_count=0;vowel_count<5;vowel_count++)
	{
	   printf("\nGenerating reference file for vowel %c!!",vowels[vowel_count]);
	   for(int utterence=1;utterence<=10;utterence++)
	   {
		  //Opening all 10 files of each vowel.
	      sprintf(open_file,"214101059_%c_%d.txt",vowels[vowel_count],utterence);
		  //The first step in training is to correct dc_shfit and perform normalisation.
	      input_normalisation=fopen( "normalised_input1.txt", "w+");
	      fopen_s(&input_file2,open_file,"r");
	      apply_normalisation(input_file2,input_normalisation);
		  //After performimg normalisation we will identify 5 steady_frames.
	      steady_frames(input_normalisation);
		  //After identifying steady frames we compute r1,ci,ai values for those 5 frames.
	      compute_ri();
		  //For computing ai values we require ri values so first we need to compute ri values.
	      compute_ai();
		  //We will be using ai values for computing ci.
		  //Utterence here indicates file number of that particular vowel.
          compute_ci(utterence);
		  //We apply raised sine window on computed ci values.
	      apply_raised_sine(utterence);
		  //Closing the files those are opened by above functions.
	      fclose(input_normalisation);
	      fclose(input_file2);
	      fclose(input_file1);
	    }
	    //Once we apply raised window the next step is to compute average among them.
	    //Fpr each vowel we will 5 rows and 12 columns of ci values.
	    compute_ci_average();
	    sprintf(generate_file, "reference_%c.txt", vowels[vowel_count]);
	    fopen_s(&refer_file, generate_file, "w+");
		//For every vowel we generate induvidual reference file.
		//Copying average ci values to the reference file.
	    generate_reference_file(refer_file);
		//Closing reference file.
	    fclose(refer_file);
	}
	//Now we perform testing on next 10 files(file 11 to file 20).
	//Declaring and initialising required variables for training.
	double temp_distance=0;
	printf("\nTraining completed!! Testing Started !!\n");
	//We will be performing testing on all 5 vowels.
	for(int vowel_count=0;vowel_count<5;vowel_count++)
	{
	    printf("Performing Testing on vowel %c!!\n",vowels[vowel_count]);
	    correct_count=0;
		//Opening all 10 files of each vowel.
	    for(int utterence=1;utterence<=10;utterence++)
	    {
		   //The first step in testing is to correct dc_shfit and perform normalisation.
	       sprintf(open_file,"214101059_%c_%d.txt",vowels[vowel_count],utterence+10);
	       input_normalisation=fopen( "normalised_input1.txt", "w+");
	       fopen_s(&input_file2,open_file,"r");
	       apply_normalisation(input_file2,input_normalisation);
		   //After performimg normalisation we will identify 5 steady_frames.
	       steady_frames(input_normalisation);
		   //After identifying steady frames we compute r1,ci,ai values for those 5 frames.
	       compute_ri();
		   //For computing ai values we require ri values so first we need to compute ri values.
	       compute_ai();
		   //We will be using ai values for computing ci.
		   //Utterence here indicates file number of that particular vowel.
	       compute_ci(utterence);
		   //We apply raised sine window on computed ci values.
	       apply_raised_sine(utterence);
		   //Closing the files those are opened by above functions.
	       fclose(input_normalisation);
	       fclose(input_file2);
	       fclose(input_file1);
		   //initialising variable distance to maximum value that a double variable can posses.
	       distance = DBL_MAX;
		   //No we will consider all 5 vowels reference files to predict which vowel is present in the file.
		   for(int i=0; i<5; ++i)
		   {
			   sprintf(generate_file, "reference_%c.txt", vowels[i]);
			   fopen_s(&refer_file, generate_file, "r");
			   //Calculating tokhura distance from testing file to all other vowesls.
         	   temp_distance = tokhura_Distance(refer_file,utterence);
			    //We will  identify the reference file with minimum distance from testing vowel. 
				if(distance > temp_distance)
				{
					distance = temp_distance;
					predicted_vowel = vowels[i];
				}
			}
		     //The below if loop helps in computing accuracy of induvidual vowel.
			if(predicted_vowel==vowels[vowel_count]) 
			{
	              correct_count++;
			}
	}
	//Printing accuracy of induvidual vowels.
	printf("Accurcay for vowel %c is %dpercent :-)\n",vowels[vowel_count],((correct_count)*10));
	printf("*****************************************************************************************************************************************************************\t\n");
	//Variable total_correct_count maintains the no of correct prediction made so far and will help in computing overall accuracy.
	total_correct_count+=correct_count;
  }
  printf("*****************************************************************************************************************************************************************\t\n");
  //Printing overall accuracy.
  printf("Overall ACCURACY over all 5 vowels = %dpercent :-)",(total_correct_count)*2);
  getch();
}
